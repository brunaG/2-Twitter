#include "users.hpp"
#include "../Utils/pending_users.hpp"

NotificationManager manager;

Users::Users(){
    sem_init(&buffer_empty, 0, 30);  //TO DO: Create a file of conStants to add the 30 value
    sem_init(&mutex_p, 0, 1);
    sem_init(&buffer_full, 0, 0);
    sem_init(&mutex_c, 0, 1);
    sem_init(&buffer_notification_empty, 0, 30);
    sem_init(&buffer_notification_full, 0, 0);
    notificationManager.InicializaEDnotification();
}

void Users::setID(int id){
    server_ID = id;
    notificationManager.setID(id);
}

void Users::DeleteUserSocket(int socket){
    usernamesMap.erase(socket);
}

void Users::NewMessageFromClient(packet message, int socket){

    string user_name;
    string user_message;
    int command_length;

    if (message.type == 0){ //Data
        sem_wait(&buffer_empty);
        sem_wait(&mutex_p);
        user_name = message.payload;
        notificationManager.NovoPerfil(user_name);
        usernamesMap[socket] = user_name;
        sem_post(&mutex_p);
        sem_post(&buffer_full);
    }
    else{ //Command
        string upper_case = message.payload;
        string normal_message = message.payload;

        std::transform (upper_case.begin(), upper_case.end(), upper_case.begin(), ::toupper);

        command_length = upper_case.length();
        int send = upper_case.find("SEND");
        int follow = upper_case.find("FOLLOW");

        if (follow != (int) string::npos){
            sem_wait(&buffer_empty);
            sem_wait(&mutex_p);

            user_message = normal_message.substr(7, command_length);
            user_name = usernamesMap[socket];
            notificationManager.NovoSeguidor(user_message, user_name);

            sem_post(&mutex_p);
            sem_post(&buffer_full);
        }
        else if (send != (int) string::npos){
            sem_wait(&buffer_notification_empty);
            sem_wait(&mutex_p);

            pkt tweetToSend;
            user_message = normal_message.substr(5, command_length);
            user_name = usernamesMap[socket];
            convertPacket(message, user_message, &tweetToSend, user_name);
            notificationManager.AdicionaTweet(&tweetToSend);
            sem_post(&mutex_p);
            sem_post(&buffer_notification_full);
        }
    }


}

void Users::convertPacket(packet message, string user_message, pkt* converted, string sender){
    converted->type = message.type;
    converted->length = message.length;
    converted->timestamp = message.timestamp;
    converted->seqn = message.seqn;

    converted->payload = user_message;
    converted->owner = sender;
}

void Users::UserLogOut(string user_name, int session){
    vector<profile>::iterator ptr;

    for (ptr = users_informations.begin(); ptr != users_informations.end(); ptr++){
        if (ptr->name == user_name){
            if (ptr->session1 == session){
                ptr->session1 = 0;
            }
            else{
                ptr->session2 = 0;
            }
        }
    }
}

int Users::GetMessage(int socket, packet* pkt_tweet){
    sem_wait(&buffer_notification_full);
    //sem_wait(&mutex_c);
    int erro = notificationManager.EnviaTweet(pkt_tweet, usernamesMap[socket]);
    if(erro == -1){
        cout << "Erro em EnviaTweet em GetMessage" << endl;
        return -1;
    }
    cout << pkt_tweet->payload << endl;
    //sem_post(&mutex_c);
    sem_post(&buffer_notification_empty);
    return 0;
}
