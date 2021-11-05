#include <pthread.h>
#include "interface_cliente.hpp"

InterfaceCliente::InterfaceCliente(Notifications &obj):notification{obj}{
    seqn = 0;
}

void InterfaceCliente::CriaInterface(){
    string user_profile;
    //string command;
    pthread_t th;

    cout << "*********************************" << endl;
    cout << "****** Welcome to Twitter !******" << endl;
    cout << "*********************************" << "\n\n\n\n" << endl;
    cout << "Please, inform your username!" << endl;
    cout << "Username: ";
    getline(cin, user_profile);
    cout << "\n\n\n\n" << endl;

    while (perfilInvalido(user_profile)){
        cout << "Your username must to have at least 4 characters and at most 20!" << endl;
        cout << "\n" << endl;
        cout << "Please, inform your username!" << endl;
        cout << "Username: ";
        getline(cin, user_profile);
        cout << "\n" << endl;
    }

    DefinePerfil(user_profile);
    ProcessaComandos(ColetaPerfil(), TYPE_USER);

    cout << "****** Welcome " << ColetaPerfil() << " ******" << endl;
    cout << "To create a post, you can use the command 'SEND' followed by your message!" << endl;
    cout << "To follow a user, you can use the command 'FOLLOW' followed by the username!" << endl;

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

    notification.UpdateBufferToSend(0, seqn, ColetaPerfil().length(), time(0), user_converted);
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
    notification.UpdateBufferToSend(type, seqn, command.length(), time(0), convertedCommand);
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
        ((InterfaceCliente*) ptr) -> notification.GetNotificationToReceive(&new_notification);
        cout << "**********************************" << endl;
        cout << "***You have a new notification!***" << endl;
        cout << "**********************************" << endl;
        printf("timestamp: %d\n", new_notification.timestamp);
        cout << "Message: " << new_notification.payload << endl;
        cout << "\n" << endl;
    }
}
