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
#include "../utilitarios/notificacao.hpp"
#include "../utilitarios/pacote.hpp"
using namespace std;


class GerenciaConexaoCliente{
    char connection_address[11];    //address to connect
    int PORT;
    Notificacoes &notification;
    int sock;

    public:
    GerenciaConexaoCliente(Notificacoes &obj);
    int EstabeleceConexao();
    void defineEnderecoConexao(char adress[11]);
    private:
    int ColetaEnviaMensagem(packet *message);
    static void* LeSocket(void *ptr);
};

#endif
