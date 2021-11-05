#ifndef SERVER_H
#define SERVER_H

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
#include <fstream>
#include <vector>
#include <signal.h>
#include <unistd.h>
#include "users.hpp"
#include "../Utils/packet.hpp"
#include "../Utils/utils.hpp"
#include "../Utils/packet_replica.hpp"
#include "replica.hpp"
#define FILE_CONFIG_PATH "./Utils/file_config.txt"
#define TIMEOUT 0.5
using namespace std;

class ServerConnectionManagement{
    int PORT;
    int client_socket[30];
    string client_ip[30];
    int max_clients;
    pthread_t th_read[30];
    pthread_t th_send[30];
    sem_t mutex_socket;
    Users users;
    vector<string> replicas_addrs;
    vector<Replica> replicas;
    string IP_addr;
    int ID_server;
    string type_server;
    int lider;


    public:
    ServerConnectionManagement(int id, string ip, string type);
    static void* ReadMessage(void *arg);
    static void* SendMessage(void *arg);
    static void* ConfirmTransaction(void *arg);
    static void printPacket (packet pkt);
    int establishConnection();
    int runAsReplica();
    int addNewSocket(int socket);
    int addNewIP(string ip);
    void deleteSocket(int socket);
    bool isSocketAlive(int socket);
    string getType();
    string getIP();
    int getID();
    vector<Replica>* getReplicas();
    int execGreetingProtocol(int sockfd, Replica *replica);
    private:
    void SetConfigurations();
    int connectToGroup();   //returns the index of the primary;
    static void* listenToGroup(void *arg);
    static void* waitLider(void* arg);
    static void alarm_handler(int value);
    static void* readElection(void* arg);
    void startElection();
};

#endif
