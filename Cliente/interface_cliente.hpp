#ifndef CLIENT_INTERFACE_H
#define CLIENT_INTERFACE_H
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string.h>
#include "../utilitarios/notificacao.hpp"
#define TYPE_USER 0
#define TYPE_COMMAND 1
using namespace std;

class InterfaceCliente{
    private:
    string profile;
    Notifications &notification;
    int seqn;

    void DefinePerfil(string user_profile);
    void SalvaNomePerfil();
    void ProcessaComandos(string command,int type);
    string LeComandos();

    public:
    InterfaceCliente(Notifications &obj);
    bool perfilInvalido(string user_profile);
    void CriaInterface();
    string ColetaPerfil();
    static void* ColetaNovaNotificacao(void *ptr);
};

#endif
