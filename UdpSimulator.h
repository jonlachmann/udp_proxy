//
// Created by Jon Lachmann on 2023-10-25.
//

#ifndef UDP_PROXY_UDPSIMULATOR_H
#define UDP_PROXY_UDPSIMULATOR_H

#include <iostream>
#include <boost/asio.hpp>
#include <random>

using namespace boost::asio;
using ip::udp;
using std::chrono::milliseconds;

struct client_info {
    client_info(io_context& ioContext, const std::string &address, int port) : proxy_to_server(ioContext) {
        client_to_proxy = udp::endpoint(ip::address::from_string(address), port);
    }
    udp::socket proxy_to_server;
    udp::endpoint client_to_proxy;
};

struct packet_info {
    std::array<char, 8129> data;
    size_t bytes;
};

class UDPSimulator {
public:
    UDPSimulator(int latency, int jitter, double dropRate, io_context& io_ctx, const udp::endpoint& localEndpoint, const udp::endpoint& remoteEndpoint, bool verbose = false);

    void startProxy();

    // Logging parameters
    long drop_count = 0;
    long package_count = 0;

private:
    void receiveLocal();
    void receiveRemote(std::shared_ptr<client_info> &client);

    void connectClient(std::string &endpoint, const std::string &address, int port);

    void sendToServer(long packetId, const std::string &address, int port);
    void sendToClient(long packetId, std::shared_ptr<client_info> &client);

    int getJitter();
    bool shouldDrop();

    io_context &io_ctx;
    std::unordered_map<std::string, std::shared_ptr<client_info>> clients;
    udp::socket localSocket_;
    udp::endpoint remoteEndpoint_;
    udp::endpoint localEndpoint_;

    std::unordered_map<long, packet_info> packets;
    long lastPacket = 0;

    // Simulation parameters
    int latencyMs_ = 100;   // Latency in milliseconds
    int jitterMs_ = 0;    // Jitter in milliseconds
    double dropRate_ = 0; // Drop rate (0.0 to 1.0)
    std::default_random_engine randomGenerator_;

    bool verbose = false;
};

#endif //UDP_PROXY_UDPSIMULATOR_H
