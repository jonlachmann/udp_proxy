#include "UdpSimulator.h"
#include <iostream>
#include <boost/asio.hpp>

void run_io_context(boost::asio::io_context& io_context) {
    io_context.run();
}

int main() {
    boost::asio::io_context ioContext;

    // Define local and remote endpoints
    udp::endpoint localEndpoint(ip::address::from_string("127.0.0.1"), 12345);
    udp::endpoint remoteEndpoint(ip::address::from_string("127.0.0.1"), 54321);

    // Create and start the UDP simulator
    UDPSimulator udpSimulator(ioContext, localEndpoint, remoteEndpoint);
    udpSimulator.start();

    // Start io_context.run() in a separate thread
    std::thread io_thread(run_io_context, std::ref(ioContext));

    io_thread.join();
    return 0;
}
