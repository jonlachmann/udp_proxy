#include <iostream>

int main() {
    io_context ioContext;

    // Define local and remote endpoints
    udp::endpoint localEndpoint(ip::address::from_string("127.0.0.1"), 12345);
    udp::endpoint remoteEndpoint(ip::address::from_string("127.0.0.1"), 54321);

    // Create and start the UDP simulator
    UDPSimulator udpSimulator(ioContext, localEndpoint, remoteEndpoint);
    udpSimulator.start();

    ioContext.run();

    return 0;
}
