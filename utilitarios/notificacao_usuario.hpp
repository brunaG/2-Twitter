#ifndef NOTIFICATION_USER_H
#define NOTIFICATION_USER_H
#include "usuario_pendente.hpp"
using namespace std;

typedef struct{
    string profile;
    vector<string> tweet;
    vector<EDUsuarioPendente> pending_notifications;
} EDnotificacaoUsuario;

#endif
