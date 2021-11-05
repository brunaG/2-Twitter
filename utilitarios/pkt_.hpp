#ifndef PKT_H
#define PKT_H
#include <string>
using namespace std;

typedef struct EDPacote{
        int type; // Type of the packet (0 - DATA | 1 - CMD)
        int seqn; //Sequence number
        int length; //Length of message
        int timestamp;
        int envios_faltantes;
        string owner;
        string payload;
} EDPacote;

#endif