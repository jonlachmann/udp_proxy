//
// Created by Jon Lachmann on 2023-10-25.
//

#ifndef UDP_PROXY_UDPSIMULATOR_H
#define UDP_PROXY_UDPSIMULATOR_H

#define BOOST_ASIO_ENABLE_HANDLER_TRACKING
#include <iostream>
#include <boost/asio.hpp>
#include <random>
#include <chrono>

using namespace boost::asio;
using ip::udp;
using std::chrono::milliseconds;

struct client_info {
    client_info(io_context& ioContext, udp::endpoint endpoint) :
        proxy_to_server(ioContext), client_to_proxy(endpoint) {
    }
    udp::socket proxy_to_server;
    udp::endpoint client_to_proxy;
};

class UDPSimulator {
public:
    UDPSimulator(io_context& ioContext, const udp::endpoint& localEndpoint, const udp::endpoint& remoteEndpoint);

    void start();

private:
    void receive();

    void connectClient(udp::endpoint &endpoint);

    void receive_back(std::shared_ptr<client_info> client);

    void send_to_server(std::size_t bytesReceived, udp::endpoint &client_endpoint, std::array<char, 8192> receive_buffer);

    void send_to_client(std::size_t bytesReceived, std::shared_ptr<client_info> &client, std::array<char, 8192> receive_buffer);

    void simulateLatency();

    void simulateJitter();

    bool simulatePacketDrop();

    io_context ioContext;
    std::unordered_map<udp::endpoint, std::shared_ptr<client_info>> clients;
    udp::socket localSocket_;
    udp::endpoint remoteEndpoint_;
    udp::endpoint localEndpoint_;

    // Simulation parameters
    int latencyMs_ = 0;   // Latency in milliseconds
    int jitterMs_ = 0;    // Jitter in milliseconds
    double dropRate_ = 0; // Drop rate (0.0 to 1.0)
    std::default_random_engine randomGenerator_;
};

#endif //UDP_PROXY_UDPSIMULATOR_H
