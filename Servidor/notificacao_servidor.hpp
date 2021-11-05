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
#include "../utilitarios/pacote.hpp"
#include "../utilitarios/pkt_.hpp"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;
#define PERFIS 100
#define SEGUIDORES 100
#define TWEETS 100
#define NOTPENDENTES 100
#define DEFAULTPATH "./data"

class GerenciaNotificacao{

    // Estrutura de um tweet: é o que recebe do Cliente

    // Estrutura que fica na fila de notificações pendentes.
    // Precisa identificar apenas o id do tweet e o perfil a ser enviado.
    struct nao_pendente{
        int seqn; //Sequence number
        string perfil;
    } nao_pendente;

    // Estrutura de uma notificação: identificada pelo perfil
    // Cada instância é uma linha do quadro do módulo de notificações
    struct notificacao{
        string perfil;  // perfil-id
        string seguidores[SEGUIDORES];  // vetor de seguidores
        struct pkt tweet[TWEETS];  // Tweets feitos por este perfil, a serem enviados
        struct nao_pendente fila_notificacoes[NOTPENDENTES]; // Fila que indica quais tweet faltam ser enviados
    } notificacao[PERFIS];

    struct perfils{  // Criei mas não estou usando. Toda essa info fica em arquivo
        string nome;
        string seguidores[SEGUIDORES];
        string seguindo[SEGUIDORES];
        string sessao1;
        string sessao2;
    } perfils;

    int server_ID;

    public:
    GerenciaNotificacao();
    int EnviaTweet(packet* tweet, string perfil);
    int AdicionaTweet(struct pkt* tweet);
    void DefineTamanhoPacote(struct pkt* tweet);
    void RemoveFilaPendentes(int indice_not, int indice_notfila);
    void RemoveTweet(int indice_not, int indice_tweet);
    int ProcuraSeguidor(string caminho, string perfil_procurado);
    int NovoSeguidor(string perfil, string seguidor);
    int NovoSeguidorNotificacao(string perfil, string seguidor);
    int adicionaLinhaArquivo(string filepath, string line);
    int AdicionaFilaPendentes(int indice_not, int id_tweet);
    void DefineEnviosPendentes(struct pkt* tweet, struct notificacao* notif);
    void NovoPerfil(string perfil);
    int NovoPerfilNotificacao(string perfil);
    void InicializaEstruturaNotificacao();
    void defineID(int id);
};

#endif
