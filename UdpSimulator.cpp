//
// Created by Jon Lachmann on 2023-10-25.
//

#include "UdpSimulator.h"

#include <boost/asio.hpp>
#include <random>
#include <chrono>

using namespace boost::asio;
using ip::udp;
using std::chrono::milliseconds;

UDPSimulator::UDPSimulator(io_context& ioContext, const udp::endpoint& localEndpoint, const udp::endpoint& remoteEndpoint)
            : localSocket_(ioContext),
              remoteEndpoint_(remoteEndpoint),
              localEndpoint_(localEndpoint),
              randomGenerator_(std::random_device()()) { }

void UDPSimulator::start() {
    localSocket_.open(udp::v4());
    localSocket_.bind(localEndpoint_);
    receive();
}

void UDPSimulator::receive() {
    udp::endpoint receive_endpoint;
    std::array<char, 8192> receive_buffer;
    localSocket_.async_receive_from(buffer(receive_buffer), receive_endpoint,
                               [this, &receive_endpoint, &receive_buffer](const boost::system::error_code& error, std::size_t bytesReceived) {
                                   if (!error && bytesReceived > 0) {
                                       receive();
                                       simulateLatency();
                                       simulateJitter();
                                       if (!simulatePacketDrop()) {
                                               std::cout << "Data from client" << std::endl;
                                               send_to_server(bytesReceived, receive_endpoint, receive_buffer);
                                       }
                                   } else {
                                       std::cout << "Error receiving data from client" << std::endl;
                                   }
                               });
}

void UDPSimulator::connectClient(udp::endpoint &endpoint) {
    clients.emplace(endpoint, std::make_shared<client_info>(client_info(ioContext, endpoint)));
    clients[endpoint]->proxy_to_server.open(udp::v4());
}

void UDPSimulator::receive_back(std::shared_ptr<client_info> client) {
    udp::endpoint receive_endpoint;
    std::array<char, 8192> receive_buffer;
    client->proxy_to_server.async_receive_from(buffer(receive_buffer), receive_endpoint,
                                     [this, &client, &receive_buffer](const boost::system::error_code& error, std::size_t bytesReceived) {
                                         if (!error && bytesReceived > 0) {
                                             receive_back(client);
                                             simulateLatency();
                                             simulateJitter();
                                             if (!simulatePacketDrop()) {
                                                 std::cout << "Data from server for client: " << client->client_to_proxy << std::endl;
                                                 send_to_client(bytesReceived, client, receive_buffer);
                                             } else {
                                                 std::cout << "Error receiving data from server" << std::endl;
                                             }
                                         }
                                     });

}

void UDPSimulator::send_to_server(std::size_t bytesReceived, udp::endpoint &client_endpoint, std::array<char, 8192> receive_buffer) {
    // Add or create mapping from client to server and vice versa
    bool new_client = clients.find(client_endpoint) == clients.end();
    if (new_client) {
        connectClient(client_endpoint);
        std::cout << "New client saved with client to proxy endpoint: " << client_endpoint << " and proxy to server endpoint: " << clients[client_endpoint]->proxy_to_server.local_endpoint() << std::endl;
    }

    // Forward data to the server
    std::cout << "Sending to server from: " << clients[client_endpoint]->proxy_to_server.local_endpoint() << " to: " << remoteEndpoint_ << std::endl;
    clients[client_endpoint]->proxy_to_server.async_send_to(buffer(receive_buffer, bytesReceived), remoteEndpoint_,
                          [](const boost::system::error_code&, std::size_t) {});

    receive_back(clients[client_endpoint]);
}

void UDPSimulator::send_to_client(std::size_t bytesReceived, std::shared_ptr<client_info> &client, std::array<char, 8192> receive_buffer) {
    // Check if the client is already known, otherwise nothing to do
    std::cout << "Sending to known client" << std::endl;

    // Forward data to a known client
    std::cout << "Sending to client from: " << client->proxy_to_server.remote_endpoint() << " over endpoint: " << client->client_to_proxy << std::endl;
    localSocket_.async_send_to(buffer(receive_buffer, bytesReceived), client->client_to_proxy,
                                [](const boost::system::error_code&, std::size_t) {});

}

void UDPSimulator::simulateLatency() {
    std::this_thread::sleep_for(milliseconds(latencyMs_));
}

void UDPSimulator::simulateJitter() {
    std::uniform_int_distribution<int> jitterDistribution(0, jitterMs_ * 2);
    int jitter = jitterDistribution(randomGenerator_);
    if (jitter != 0) {
        std::this_thread::sleep_for(milliseconds(jitter));
    }
}

bool UDPSimulator::simulatePacketDrop() {
    std::uniform_real_distribution<double> dropDistribution(0.0, 1.0);
    double dropProbability = dropDistribution(randomGenerator_);
    return dropProbability < dropRate_;
}