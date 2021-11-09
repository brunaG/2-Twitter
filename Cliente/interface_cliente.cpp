#include <pthread.h>
#include "interface_cliente.hpp"

InterfaceCliente::InterfaceCliente(Notificacoes &obj):notification{obj}{
    seqn = 0;
}

void InterfaceCliente::CriaInterface(){
    string user_profile;
    pthread_t th;

    cout << "███████████████████████████████████████" << endl;
    cout << "               TWITTER "                 << endl;
    cout << "███████████████████████████████████████" << "\n" << endl;
    cout << "Digite seu nome de usuario" << endl;
    cout << "Username: ";
    getline(cin, user_profile);
    cout << "\n" << endl;

    while (perfilInvalido(user_profile)){
        cout << "Seu nome de usuario deve ter entre 4 e 20 caracteres" << endl;
        cout << "\n" << endl;
        cout << "Digite seu nome de usuario" << endl;
        cout << "Username: ";
        getline(cin, user_profile);
        cout << "\n" << endl;
    }

    DefinePerfil(user_profile);
    ProcessaComandos(ColetaPerfil(), TYPE_USER);

    cout << "██████████ SEJA BEM VINDO " << ColetaPerfil()<< " ██████████" << endl;
    cout << "Use o comando SEND + sua mensagem para postar uma nova mensagem!" << endl;
    cout << "Use o comando FOLLOW @ + nome de usuario que voce deseja seguir e nao esqueca do @" << endl;
    // cout << "nao esqueca do @" << endl;

    pthread_create(&th, NULL, &InterfaceCliente::ColetaNovaNotificacao, this);

    while (true){
        ProcessaComandos(LeComandos(),TYPE_COMMAND);
    }   

    pthread_join(th, NULL);
}

/*
    UNUSED
*/
void InterfaceCliente::SalvaNomePerfil(){
    char user_converted[ColetaPerfil().size()+1];
    strcpy(user_converted, ColetaPerfil().c_str());

    notification.AtualizaBufferEnviar(0, seqn, ColetaPerfil().length(), time(0), user_converted);
    seqn++;
}

bool InterfaceCliente::perfilInvalido(string user_profile){
    return (user_profile.length() < 4) || (user_profile.length() > 20);
}
string InterfaceCliente::LeComandos(){
    string command;
    cout << "\n" << endl;
    cin.ignore(-1);
    getline(cin, command);
    return command;
}

void InterfaceCliente::ProcessaComandos(string command, int type){
    char convertedCommand[command.length() + 1];
    strcpy(convertedCommand, command.c_str());
    notification.AtualizaBufferEnviar(type, seqn, command.length(), time(0), convertedCommand);
    seqn++;

}

void InterfaceCliente::DefinePerfil(string user_profile){
    profile = "@" + user_profile;
}

string InterfaceCliente::ColetaPerfil(){
    return profile;
}

void* InterfaceCliente::ColetaNovaNotificacao(void *ptr){
    packet new_notification;
    while (true){
        ((InterfaceCliente*) ptr) -> notification.ColetaNotificacoesReceber(&new_notification);
        cout << "█████████████████████████████████" << endl;
        cout << "    NOVA MENSAGEM                  " << endl;
        cout << "█████████████████████████████████" << endl;
        cout << "\n" << endl;
        //printf("timestamp: %d\n", new_notification.timestamp);
        cout << new_notification.payload << endl;
        cout << "\n" << endl;
    }
}
