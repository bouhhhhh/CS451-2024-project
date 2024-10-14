#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <unordered_map>

// Assuming definitions of Message and Packet
struct Message {
    uint32_t sequence_number;
    // sender id 
};

struct Packet {
    Message messages[1];  // Simple example with a single message per packet for clarity
    uint8_t message_count;
};

std::unordered_map<uint32_t, bool> ack_received;

// Setup the socket
void setupSocket(int& sockfd, uint16_t port, const char* ip_address = "127.0.0.1") {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error("Failed to create socket.");
    }

    struct sockaddr_in myaddr;
    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = inet_addr(ip_address);
    myaddr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        close(sockfd);
        throw std::runtime_error("Bind failed.");
    }
}

// Send a packet
void sendPacket(int sockfd, const struct sockaddr_in& addr, const Packet& packet) {
    if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("sendto failed");
    }
}

// Send a message with retransmission
void sendMessage(int sockfd, struct sockaddr_in& addr, const Message& msg) {
    Packet packet;
    packet.messages[0] = msg;  // Simplified: Sending one message per packet
    packet.message_count = 1;

    while (!ack_received[msg.sequence_number]) {
        sendPacket(sockfd, addr, packet);
        std::this_thread::sleep_for(std::chrono::seconds(1)); // Retransmission timeout
    }
}



// Receive messages and handle acknowledgments
void receiveMessages(int sockfd) {
    struct sockaddr_in remoteAddr;
    socklen_t addrLen = sizeof(remoteAddr);
    Packet packet;

    while (true) {
        if (recvfrom(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&remoteAddr, &addrLen) > 0) {
            // Process received packet
            // write it in the output 
            std::cout << "Received packet with sequence number: " << packet.messages[0].sequence_number << std::endl;
            sendAck(sockfd, remoteAddr, packet.messages[0].sequence_number);
        }
    }
}

// Send an acknowledgment
void sendAck(int sockfd, const struct sockaddr_in& addr, uint32_t sequence_number) {
    Message ack;
    ack.sequence_number = sequence_number;
    Packet ack_packet;
    ack_packet.messages[0] = ack;
    ack_packet.message_count = 1;

    sendPacket(sockfd, addr, ack_packet);
}