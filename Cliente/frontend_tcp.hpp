#ifndef FRONTEND_H
#define FRONTEND_H

#include <unistd.h>
#include <cstdio>
#include <sys/socket.h>
#include <cstdlib>
#include <netinet/in.h>
#include <cstring>
#include <ctime>
#include <semaphore.h>
#include <iostream>
#include <cerrno>
#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <ctime> //FD_SET, FD_ISSET, FD_ZERO macros
#include "../utilitarios/notificacao.hpp"
#include "../utilitarios/pacote.hpp"
using namespace std;

class FrontendConnectionManagement{
    char addr[11];    //frontend's address
    int PORT;
    int client_socket;
    int server_socket;
    string username;
    pthread_t th_read_client;
    pthread_t th_read_server;
    sem_t mutex_socket;
    Notifications &notification;


    public:
    FrontendConnectionManagement(Notifications &obj);
    static void* ReadFromClient(void *arg);
    static void* ReadFromServer(void *arg);
    int SendMessage(packet pkt, int socket);
    static void printPacket (packet pkt);
    int establishConnection();
    int ConnectToServer();
    int addNewSocket(int socket);
    void deleteSocket(int socket);
    bool isSocketAlive(int socket);
    char* getAdress();
    int getClientSocket();
    sem_t* getMutexSocket();
    private:
    int GetAndSendNotification(packet *message);
};

#endif