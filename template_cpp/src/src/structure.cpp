#include <cstdint>

#define MAX_MESSAGES_PER_PACKET 8
#define MAX_INT 1234567


struct Message {
    uint32_t sequence_number;
};

struct Packet {
    Message messages[MAX_MESSAGES_PER_PACKET];
    uint8_t message_count;
};
