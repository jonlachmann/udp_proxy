//
// Created by Jon Lachmann on 2023-10-25.
//

#include "UdpSimulator.h"

#include <iostream>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <random>
#include <chrono>

using namespace boost::asio;
using ip::udp;
using std::chrono::milliseconds;

UDPSimulator::UDPSimulator(io_context& ioContext, const udp::endpoint& localEndpoint, const udp::endpoint& remoteEndpoint)
            : socket_(ioContext, localEndpoint),
              remoteEndpoint_(remoteEndpoint),
              randomGenerator_(std::random_device()()) {}

void UDPSimulator::start() {
    receive();
}

void UDPSimulator::receive() {
    socket_.async_receive_from(buffer(receiveBuffer_), remoteEndpoint_,
                               [this](const boost::system::error_code& error, std::size_t bytesReceived) {
                                   if (!error && bytesReceived > 0) {
                                       simulateLatency();
                                       simulateJitter();
                                       if (!simulatePacketDrop()) {
                                           send(bytesReceived);
                                       }
                                       receive();
                                   }
                               });
}

void UDPSimulator::send(std::size_t bytesReceived) {
    socket_.async_send_to(buffer(receiveBuffer_, bytesReceived), remoteEndpoint_,
                          [](const boost::system::error_code&, std::size_t) {});
}

void UDPSimulator::simulateLatency() {
    std::this_thread::sleep_for(milliseconds(latencyMs_));
}

void UDPSimulator::simulateJitter() {
    std::uniform_int_distribution<int> jitterDistribution(-jitterMs_, jitterMs_);
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