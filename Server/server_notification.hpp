#ifndef SERVER_NOTIFICATIONS_H
#define SERVER_NOTIFICATIONS_H

#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <iomanip>
#include <string.h>
#include <iostream>
#include "../Utils/packet.hpp"
#include "../Utils/pkt_.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
#define PERFIS 100
#define SEGUIDORES 100
#define TWEETS 100
#define NOTPENDENTES 100
#define DEFAULTPATH "./data"

class NotificationManager{

    // Estrutura de um tweet: é o que recebe do Client

    // Estrutura que fica na fila de notificações pendentes.
    // Precisa identificar apenas o id do tweet e o perfil a ser enviado.
    struct not_pendente{
        int seqn; //Sequence number
        string perfil;
    } not_pendente;

    // Estrutura de uma notificação: identificada pelo perfil
    // Cada instância é uma linha do quadro do módulo de notificações
    struct notification{
        string perfil;  // perfil-id
        string seguidores[SEGUIDORES];  // vetor de seguidores
        struct pkt tweet[TWEETS];  // Tweets feitos por este perfil, a serem enviados
        struct not_pendente fila_notificacoes[NOTPENDENTES]; // Fila que indica quais tweet faltam ser enviados
    } notification[PERFIS];

    struct perfis{  // Criei mas não estou usando. Toda essa info fica em arquivo
        string nome;
        string seguidores[SEGUIDORES];
        string seguindo[SEGUIDORES];
        string sessao1;
        string sessao2;
    } perfis;

    int server_ID;

    public:
    NotificationManager();
    int EnviaTweet(packet* tweet, string perfil);
    int AdicionaTweet(struct pkt* tweet);
    void SetPktLenght(struct pkt* tweet);
    void RemoveNotFila(int indice_not, int indice_notfila);
    void RemoveTweet(int indice_not, int indice_tweet);
    int ProcuraSeguidor(string caminho, string perfil_procurado);
    int NovoSeguidor(string perfil, string seguidor);
    int NovoSeguidorNotificacao(string perfil, string seguidor);
    int appendLineToFile(string filepath, string line);
    int IncluiNaFilaNotPendentes(int indice_not, int id_tweet);
    void SetEnviosFaltantes(struct pkt* tweet, struct notification* notif);
    void NovoPerfil(string perfil);
    int NovoPerfilNotificacao(string perfil);
    void InicializaEDnotification();
    void setID(int id);
};

#endif
