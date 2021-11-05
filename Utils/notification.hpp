#ifndef NOTIFICATIONS_H
#define NOTIFICATIONS_H

#include <iostream>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>
#include "packet.hpp"
using namespace std;


class Notifications{
    int buffer_size;
    int in_send, in_receive;
    int out_send, out_receive;
    bool new_notification, new_packet_to_send;
    packet buffer_to_send [256];
    packet buffer_to_receive [256];
    sem_t buffer_to_send_empty, buffer_to_send_full, mutex_p_send, mutex_c_send;
    sem_t buffer_to_receive_empty, buffer_to_receive_full, mutex_p_receive, mutex_c_receive;

    public:
    Notifications();
    void UpdateBufferToSend(uint16_t type, uint16_t seqn, uint16_t length, int16_t timestamp, char* payload);
    void UpdateBufferToReceive(uint16_t type, uint16_t seqn, uint16_t length, int16_t timestamp, char* payload);
    void GetNotificationToSend(packet* message);
    void GetNotificationToReceive(packet* message);
    bool GetNewNotification();
    bool GetNewPacket();

};


#endif
