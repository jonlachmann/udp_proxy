#include "UdpSimulator.h"
#include <iostream>
#include <boost/asio.hpp>

int main(int argc, char* argv[]) {
    if (argc != 7)
    {
        std::cerr << "usage: udp_proxy <local port> <forward host ip> <forward port> <latency> <jitter> <drop_rate>" << std::endl;
        return 1;
    }

    unsigned short local_port = static_cast<unsigned short>(std::stoul(argv[1]));
    std::string forward_host = argv[2];
    unsigned short forward_port = static_cast<unsigned short>(std::stoul(argv[3]));
    int latency = std::stoi(argv[4]);
    int jitter = std::stoi(argv[5]);
    double drop_rate = std::stod(argv[6]);


    boost::asio::io_context io_context;

    // Define local and remote endpoints
    udp::endpoint localEndpoint(ip::address::from_string("127.0.0.1"), local_port);
    udp::endpoint remoteEndpoint(ip::address::from_string(forward_host), forward_port);

    // Create and start the UDP simulator
    UDPSimulator udp_simulator(latency, jitter, drop_rate, io_context, localEndpoint, remoteEndpoint);
    udp_simulator.startProxy();

    io_context.run();

    return 0;
}
