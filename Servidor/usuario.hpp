#ifndef USERS_H
#define USERS_H

#include <stdio.h>
#include <string.h>
#include <algorithm>
#include <semaphore.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "../utilitarios/perfil.hpp"
#include "../utilitarios/pacote.hpp"
#include "../utilitarios/pkt_.hpp"
#include "notificacao_servidor.hpp"
using namespace std;

class Usuario{
    vector<profile> users_informations;
    string path_database = "./data";
    string format = ".txt";
    map<int,string> usernamesMap = map<int,string>();
    GerenciaNotificacao notificationManager;
    sem_t buffer_full, mutex_p, buffer_empty, mutex_c, buffer_notification_full,buffer_notification_empty;
    int server_ID;

    public:
    Usuario();
    void convertePacote(packet message,string user_message, pkt* converted, string sender);
    void novaMensagemCliente(packet message, int socket);
    void UsuarioSaiu(string user_name, int session);
    int ColetaMensagem(int socket, packet* pkt_tweet);
    void DeletaUsuarioSocket(int socket);
    void defineID(int id);

};

#endif
