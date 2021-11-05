#ifndef PACKET_H
#define PACKET_H

using namespace std;

typedef struct{
	uint16_t type; // Type of the packet (0 - DATA | 1 - CMD)
	uint16_t seqn; //Sequence number
    uint16_t length; //Length of payload
    uint16_t timestamp;
    char payload[256];
} packet;

#endif
