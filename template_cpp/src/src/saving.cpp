#include <chrono>
#include <iostream>
#include <thread>
#include <signal.h>
#include <cstdlib>  // for std::exit
#include <netinet/in.h>  // for sockaddr_in
#include <arpa/inet.h>   // for inet_pton
#include <pthread.h>

#include "parser.hpp"
#include "hello.h"

static void stop(int) {
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    std::cout << "Immediately stopping network packet processing.\n";
    std::cout << "Writing output.\n";
    std::exit(0);
}

struct Node {
    int pid;
    bool is_active;
};

void* sender_thread(void* arg);
void* receiver_thread(void* arg);

void* sender_thread(void* arg) {
    Node* node = static_cast<Node*>(arg);
    int pid = node->pid;
    int message_count = 0;
    const int MAX_MESSAGES = 3;  // Each sender will send 3 messages

    struct sockaddr_in targetAddr;
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(12345);
    inet_pton(AF_INET, "127.0.0.1", &targetAddr.sin_addr);

    while (node->is_active && message_count < MAX_MESSAGES) {
        std::cout << "Sender " << pid << ": Sent message " << message_count + 1 << std::endl;
        message_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // delay
    }

    std::cout << "Sender " << pid << ": Finished sending messages\n";
    node->is_active = false;  // Mark the node as inactive
    delete node;  // Clean up the allocated node
    return NULL;
}

void* receiver_thread(void* arg) {
    int sockfd;
    uint16_t port = 12345; // same port
    // setupSocket(sockfd, port); // Add your socket setup code here

    // while (true) {
    //     // Message msg = receiveMessage(sockfd);
    //     std::cout << "Receiver: Received message with sequence number " << std::endl;
    // }

    // close(sockfd);
    return NULL;
}

int main(int argc, char **argv) {
    signal(SIGTERM, stop);
    signal(SIGINT, stop);

    Parser parser(argc, argv);
    parser.parse();

    const int MAX_SENDERS = 3;
    pthread_t senders[MAX_SENDERS];
    pthread_t receiver;

    pthread_create(&receiver, NULL, receiver_thread, NULL);
    for (int i = 0; i < MAX_SENDERS; i++) {
        Node* node = new Node();  // Use new instead of malloc
        node->pid = getpid();
        node->is_active = true; 
        pthread_create(&senders[i], NULL, sender_thread, node);
    }

    for (int i = 0; i < MAX_SENDERS; i++) {
        pthread_join(senders[i], NULL);
    }
    pthread_join(receiver, NULL);

    hello();
    std::cout << std::endl;

    std::cout << "My PID: " << getpid() << "\n";
    std::cout << "From a new terminal type `kill -SIGINT " << getpid() << "` or `kill -SIGTERM "
              << getpid() << "` to stop processing packets\n\n";

    std::cout << "My ID: " << parser.id() << "\n\n";

    std::cout << "List of resolved hosts is:\n";
    std::cout << "==========================\n";
    auto hosts = parser.hosts();
    for (auto &host : hosts) {
        std::cout << host.id << "\n";
        std::cout << "Human-readable IP: " << host.ipReadable() << "\n";
        std::cout << "Machine-readable IP: " << host.ip << "\n";
        std::cout << "Human-readable Port: " << host.portReadable() << "\n";
        std::cout << "Machine-readable Port: " << host.port << "\n";
        std::cout << "\n";
    }
    std::cout << "\n";

    std::cout << "Path to output:\n";
    std::cout << "===============\n";
    std::cout << parser.outputPath() << "\n\n";

    std::cout << "Path to config:\n";
    std::cout << "===============\n";
    std::cout << parser.configPath() << "\n\n";

    std::cout << "Doing some initialization...\n\n";

    std::cout << "Broadcasting and delivering messages...\n\n";

    // After a process finishes broadcasting,
    // it waits forever for the delivery of messages.
    while (true) {
        std::this_thread::sleep_for(std::chrono::hours(1));
    }

    return 0;
}
