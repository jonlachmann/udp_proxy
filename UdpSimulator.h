//
// Created by Jon Lachmann on 2023-10-25.
//

#ifndef UDP_PROXY_UDPSIMULATOR_H
#define UDP_PROXY_UDPSIMULATOR_H


#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <random>
#include <chrono>

using namespace boost::asio;
using ip::udp;
using std::chrono::milliseconds;

class UDPSimulator {
public:
    UDPSimulator(io_context& ioContext, const udp::endpoint& localEndpoint, const udp::endpoint& remoteEndpoint)
            : socket_(ioContext, localEndpoint),
              remoteEndpoint_(remoteEndpoint),
              randomGenerator_(std::random_device()());

    void start();

private:
    void receive();

    void send(std::size_t bytesReceived);

    void simulateLatency();

    void simulateJitter();

    bool simulatePacketDrop();

    udp::socket socket_;
    udp::endpoint remoteEndpoint_;
    boost::array<char, 8192> receiveBuffer_;

    // Simulation parameters
    int latencyMs_ = 50;   // Latency in milliseconds
    int jitterMs_ = 20;    // Jitter in milliseconds
    double dropRate_ = 0.1; // Drop rate (0.0 to 1.0)
    std::default_random_engine randomGenerator_;
};

#endif //UDP_PROXY_UDPSIMULATOR_H
