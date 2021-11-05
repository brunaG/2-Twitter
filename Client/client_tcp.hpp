#ifndef CLIENT_CONNECTION_MANEGMENT_H
#define CLIENT_CONNECTION_MANEGMENT_H

#include <iostream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "../Utils/notification.hpp"
#include "../Utils/packet.hpp"
using namespace std;


class ClientConnectionManegment{
    char connection_address[11];    //address to connect
    int PORT;
    Notifications &notification;
    int sock;

    public:
    ClientConnectionManegment(Notifications &obj);
    int StabilishConection();
    void setConnectionAddress(char adress[11]);
    private:
    int GetAndSendNotification(packet *message);
    static void* ReadSocket(void *ptr);
};

#endif
