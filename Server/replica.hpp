#ifndef REPLICA_H
#define REPLICA_H

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
#include "users.hpp"
#include "../Utils/packet.hpp"
using namespace std;

class Replica{
    int id;
    string address;
    int socket;

    public:
    Replica(string ip);
    Replica();
    string getAddress();
    int getID();
    void setId(int i);
    void setSocket(int sock);
    int getSocket();
};

#endif
