//
// Created by Jon Lachmann on 2023-10-25.
//

#include "UdpSimulator.h"

using namespace boost::asio;
using ip::udp;
using std::chrono::milliseconds;

UDPSimulator::UDPSimulator(int latency, int jitter, double dropRate, io_context& io_ctx, const udp::endpoint& localEndpoint, const udp::endpoint& remoteEndpoint, bool verbose)
            : latencyMs_(latency),
              jitterMs_(jitter),
              dropRate_(dropRate),
              localSocket_(io_ctx),
              remoteEndpoint_(remoteEndpoint),
              localEndpoint_(localEndpoint),
              io_ctx(io_ctx),
              verbose(verbose),
              randomGenerator_(std::random_device()()) { }

// Start the proxy server, bind to the address and start receiving data
void UDPSimulator::startProxy() {
    localSocket_.open(udp::v4());
    localSocket_.bind(localEndpoint_);
    receiveLocal();
}

// Receive packages from clients and queue them for forwarding to the server
void UDPSimulator::receiveLocal() {
    std::cout << "\033[2J\033[1;1H";
    std::cout << "UDP lag/latency/drop simulation proxy running!" << std::endl;
    std::cout << "Latency: " << latencyMs_ << "ms" << std::endl;
    std::cout << "Jitter: " << jitterMs_ << "ms" << std::endl;
    std::cout << "Drop probability: " << dropRate_ << "" << std::endl;
    std::cout << "Dropped/Total: " << drop_count << "/" << package_count << std::endl;

    udp::endpoint receive_endpoint;
    long packetId = ++lastPacket;
    packets[packetId] = packet_info();
    localSocket_.async_receive_from(buffer(packets[packetId].data, packets[packetId].data.size()), receive_endpoint,
        [this, &receive_endpoint, packetId](const boost::system::error_code &error, std::size_t bytesReceived) {
            if (!error && bytesReceived > 0) {
                packets[packetId].bytes = bytesReceived;
                if (!shouldDrop()) {
                    std::string address = receive_endpoint.address().to_string();
                    int port = receive_endpoint.port();
                    int delay = latencyMs_ + getJitter();
                    auto timer = std::make_shared<boost::asio::steady_timer>(io_ctx,
                                                                                boost::asio::chrono::milliseconds(
                                                                                        delay));
                    timer->async_wait([this, packetId, address, port, timer](const boost::system::error_code &ec) {
                        sendToServer(packetId, address, port);
                    });
                }
                receiveLocal();
            } else {
                std::cout << "Error receiving data from client" << std::endl;
            }
        });
}

// Add new clients to the clients table, keeping track of "connections"
void UDPSimulator::connectClient(std::string &endpoint, const std::string &address, int port) {
    clients.emplace(endpoint, std::make_shared<client_info>(client_info(io_ctx, address, port)));
    clients[endpoint]->proxy_to_server.open(udp::v4());
}

// Receive packages from the server per client and queue them for forwarding to the respective client
void UDPSimulator::receiveRemote(std::shared_ptr<client_info> &client) {
    udp::endpoint receive_endpoint;
    long packetId = ++lastPacket;
    packets[packetId] = packet_info();
    client->proxy_to_server.async_receive_from(buffer(packets[packetId].data), receive_endpoint,
       [this, &client, packetId](const boost::system::error_code &error, std::size_t bytesReceived) {
           if (!error && bytesReceived > 0) {
               packets[packetId].bytes = bytesReceived;
               int delay = latencyMs_ + getJitter();
               auto timer = std::make_shared<boost::asio::steady_timer>(
                       io_ctx, boost::asio::chrono::milliseconds(delay));
               if (!shouldDrop()) {
                   timer->async_wait([this, &client, packetId, timer](const boost::system::error_code &ec) {
                       sendToClient(packetId, client);
                   });
               }
               receiveRemote(client);
           } else {
               std::cout << "Error receiving data from server" << std::endl;
           }
       });

}

// Forward a package from the server to the respective client
void UDPSimulator::sendToClient(long packetId, std::shared_ptr<client_info> &client) {
    // Forward data to a known client
    package_count++;
    localSocket_.async_send_to(buffer(packets[packetId].data, packets[packetId].bytes), client->client_to_proxy,
                                [this, packetId, &client](const boost::system::error_code &ec, std::size_t) {
        if (verbose) std::cout << "Sent id: " << packetId << " to client from over endpoint: " << client->client_to_proxy << std::endl;
        packets.erase(packetId);
    });
}

// Forward a package from a client to the server, adding new clients and opening a socket to receive on
void UDPSimulator::sendToServer(long packetId, const std::string &address, int port) {
    // Add or create mapping from client to server and vice versa
    std::string endpoint = address + ":" + std::to_string(port);
    bool new_client = clients.find(endpoint) == clients.end();
    if (new_client) {
        connectClient(endpoint, address, port);
        if (verbose) std::cout << "New client saved with client to proxy endpoint: " << endpoint << std::endl;
        receiveRemote(clients[endpoint]);
    }

    // Forward data to the server
    package_count++;
    clients[endpoint]->proxy_to_server.async_send_to(buffer(packets[packetId].data, packets[packetId].bytes), remoteEndpoint_,
                                                     [this, packetId, endpoint](const boost::system::error_code&, std::size_t) {
                                                         if (verbose) std::cout << "Sent id: " << packetId << " to server from: " << clients[endpoint]->proxy_to_server.local_endpoint() << " to: " << remoteEndpoint_ << std::endl;
                                                         packets.erase(packetId);
                                                     });
}

// Draw random jitter from the jitter distribution
int UDPSimulator::getJitter() {
    std::uniform_int_distribution<int> jitterDistribution(0, jitterMs_ * 2);
    int jitter = jitterDistribution(randomGenerator_);
    return jitter;
}

// Draw from the package drop distribution
bool UDPSimulator::shouldDrop() {
    std::uniform_real_distribution<double> dropDistribution(0.0, 1.0);
    double drop_probability = dropDistribution(randomGenerator_);
    bool should_drop = drop_probability < dropRate_;
    if (should_drop) drop_count++;
    return should_drop;
}
