#ifndef PACKET_REPLICA_H
#define PACKET_REPLICA_H

using namespace std;

typedef struct{
	uint16_t type; // Type of the packet (0 - DATA | 1 - CMD)
	uint16_t seqn; //Sequence number
    uint16_t length; //Length of payload
    uint16_t timestamp;
    uint16_t socket;
    char payload[256];
} EDPacoteReplica;

#endif
