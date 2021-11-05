#include "replica.hpp"

Replica::Replica(string ip){
    this->id = -1;      //-1 means its not defined
    this->address = ip;
    this->socket = -1;
};

Replica::Replica(){
    this->id = -1;      //-1 means its not defined
    this->address = "noaddress";
    this->socket = -1;
};

string Replica::getAddress(){
    return this->address;
}

int Replica::getID(){
    return this->id;
}

void Replica::setSocket(int sock){
    this->socket = sock;
}

void Replica::setId(int i){
    this->id = i;
}

int Replica::getSocket(){
    return this->socket;
}
