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

class GerenciaConexaoFrontend{
    char addr[11];    //frontend's address
    int PORT;
    int client_socket;
    int server_socket;
    string username;
    pthread_t th_read_client;
    pthread_t th_read_server;
    sem_t mutex_socket;
    Notificacoes &notification;


    public:
    GerenciaConexaoFrontend(Notificacoes &obj);
    static void* LeCliente(void *arg);
    static void* LeServidor(void *arg);
    int LeMensagem(packet pkt, int socket);
    static void imprimePacote (packet pkt);
    int estabeleceConexao();
    int ConectaServidor();
    int adicionanovosocket(int socket);
    void deletaSocket(int socket);
    bool socketAtivo(int socket);
    char* coletaEndereco();
    int coletaSocketCliente();
    sem_t* coletaSocketMutex();
    private:
    int ColetaEnviaMensagem(packet *message);
};

#endif