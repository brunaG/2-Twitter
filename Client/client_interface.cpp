#include <pthread.h>
#include "client_interface.hpp"

ClientInterface::ClientInterface(Notifications &obj):notification{obj}{
    seqn = 0;
}

void ClientInterface::CreateInterface(){
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

    while (isProfileInvalid(user_profile)){
        cout << "Your username must to have at least 4 characters and at most 20!" << endl;
        cout << "\n" << endl;
        cout << "Please, inform your username!" << endl;
        cout << "Username: ";
        getline(cin, user_profile);
        cout << "\n" << endl;
    }

    SetProfile(user_profile);
    ProcessCommand(GetProfile(), TYPE_USER);

    cout << "****** Welcome " << GetProfile() << " ******" << endl;
    cout << "To create a post, you can use the command 'SEND' followed by your message!" << endl;
    cout << "To follow a user, you can use the command 'FOLLOW' followed by the username!" << endl;

    pthread_create(&th, NULL, &ClientInterface::GetNewNotification, this);

    while (true){
        ProcessCommand(ReadCommand(),TYPE_COMMAND);
    }   

    pthread_join(th, NULL);
}

/*
    UNUSED
*/
void ClientInterface::SaveProfileName(){
    char user_converted[GetProfile().size()+1];
    strcpy(user_converted, GetProfile().c_str());

    notification.UpdateBufferToSend(0, seqn, GetProfile().length(), time(0), user_converted);
    seqn++;
}

bool ClientInterface::isProfileInvalid(string user_profile){
    return (user_profile.length() < 4) || (user_profile.length() > 20);
}
string ClientInterface::ReadCommand(){
    string command;
    cout << "\n" << endl;
    cin.ignore(-1);
    getline(cin, command);
    return command;
}

void ClientInterface::ProcessCommand(string command, int type){
    char convertedCommand[command.length() + 1];
    strcpy(convertedCommand, command.c_str());
    notification.UpdateBufferToSend(type, seqn, command.length(), time(0), convertedCommand);
    seqn++;

}

void ClientInterface::SetProfile(string user_profile){
    profile = "@" + user_profile;
}

string ClientInterface::GetProfile(){
    return profile;
}

void* ClientInterface::GetNewNotification(void *ptr){
    packet new_notification;
    while (true){
        ((ClientInterface*) ptr) -> notification.GetNotificationToReceive(&new_notification);
        cout << "**********************************" << endl;
        cout << "***You have a new notification!***" << endl;
        cout << "**********************************" << endl;
        printf("timestamp: %d\n", new_notification.timestamp);
        cout << "Message: " << new_notification.payload << endl;
        cout << "\n" << endl;
    }
}
