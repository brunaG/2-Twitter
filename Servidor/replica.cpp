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

string Replica::pegaEndereco(){
    return this->address;
}

int Replica::coletaID(){
    return this->id;
}

void Replica::defineSocket(int sock){
    this->socket = sock;
}

void Replica::defineID(int i){
    this->id = i;
}

int Replica::coletaSocket(){
    return this->socket;
}
