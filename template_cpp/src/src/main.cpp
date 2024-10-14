#include <chrono>
#include <iostream>
#include <thread>

#include "parser.hpp"
#include "hello.h"
#include <signal.h>
#include "udpLayer.cpp"



define MAX_SENDERS 10
define MAX_MESSAGES 2

static void stop(int) {
  // reset signal handlers to default
  signal(SIGTERM, SIG_DFL);
  signal(SIGINT, SIG_DFL);

  // immediately stop network packet processing
  std::cout << "Immediately stopping network packet processing.\n";

  // write/flush output file if necessary
  std::cout << "Writing output.\n";

  // exit directly from signal handler
  exit(0);
}


struct Node {
    int pid;
    bool is_active;
};

void* sender_thread(void* arg) {
    Node* node = (Node*)arg;
    int pid = node->pid;
    int message_count = 0;

    // Initialize socket and port
    int sockfd;
    uint16_t port = 4200 + pid;  
    setupSocket(sockfd, port);

    struct sockaddr_in targetAddr;
    targetAddr.sin_family = AF_INET;
    targetAddr.sin_port = htons(12345); 
    inet_pton(AF_INET, "127.0.0.1", &targetAddr.sin_addr);

    while (node->is_active && message_count < MAX_MESSAGES) {
        // Create and send a message
        Message msg;
        msg.sequence_number = message_count + 1;
        sendMessage(sockfd, targetAddr, msg);
        std::cout << "Sender " << pid << ": Sent message " << msg.sequence_number << std::endl;

        message_count++;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));  // delay
    }

    close(sockfd);
    return NULL;
}

void* receiver_thread(void* arg) {
    int sockfd;
    uint16_t port = 12345;  // same port
    setupSocket(sockfd, port);

    while (true) {
        Message msg = receiveMessage(sockfd);
        std::cout << "Receiver: Received message with sequence number " << msg.sequence_number << std::endl;
    }

    close(sockfd);
    return NULL;
}


int main(int argc, char **argv) {
  signal(SIGTERM, stop);
  signal(SIGINT, stop);

  // `true` means that a config file is required.
  // Call with `false` if no config file is necessary.
  bool requireConfig = true;


  Parser parser(argc, argv);
  parser.parse();

 //initializing the threads
  pthread_t senders[MAX_SENDERS];
  pthread_t receiver;
  pthread_create(&receiver, NULL, receiver_thread, NULL);
    for (int i = 0; i < MAX_SENDERS; i++) {
        Node* node = (Node*)malloc(sizeof(Node));
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
    std::cout << "Human-readbale Port: " << host.portReadable() << "\n";
    std::cout << "Machine-readbale Port: " << host.port << "\n";
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


  int sockfd;
  uint16_t port = 4200;  // Example
  setupSocket(sockfd, port);

  // Example usage of sendMessage
  struct sockaddr_in targetAddr;
  targetAddr.sin_family = AF_INET;
  targetAddr.sin_port = htons(12345);
  inet_pton(AF_INET, "127.0.0.1", &targetAddr.sin_addr); // Example IP address

  Message msg;
  msg.sequence_number = 1;  // Starting with sequence number 1
  sendMessage(sockfd, targetAddr, msg);

  // Example of receiving messages in a separate thread
  std::thread receiver(receiveMessages, sockfd);
  receiver.detach();  // Detach the thread or join as needed

  // Your application logic

// initializing the threads





  // After a process finishes broadcasting,
  // it waits forever for the delivery of messages.
  while (true) {
    std::this_thread::sleep_for(std::chrono::hours(1));
  }

  return 0;
}
