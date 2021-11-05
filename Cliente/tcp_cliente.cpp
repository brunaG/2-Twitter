#include "tcp_cliente.hpp"

ClientConnectionManegment::ClientConnectionManegment(Notifications &obj):notification{obj}{
    //TO DO: We can modify the constructor to receive the port
    PORT = 8080;
    sock = 0;
}

int ClientConnectionManegment::StabilishConection()
{
    struct sockaddr_in serv_address;
    packet message;
    pthread_t th;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "\n Socket creation error \n";
        return -1;
    }

    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(PORT);
    
    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, this->connection_address, &serv_address.sin_addr)<=0)
    {
        cout << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_address, sizeof(serv_address)) < 0)
    {
        cout << "\nConnection Failed**\n";
        return -1;
    }

    pthread_create(&th, NULL, &ClientConnectionManegment::ReadSocket, this);

    while (true){
        if(GetAndSendNotification(&message) < 0){
            cout << "Error when sending the message" << endl;
        };
    }

    pthread_join(th, NULL);

    close(sock);
    return 0;
}

int ClientConnectionManegment::GetAndSendNotification(packet *message){
    notification.GetNotificationToSend(message);
    return send(sock , message, sizeof(*message), 0);
}

void* ClientConnectionManegment::ReadSocket(void *ptr){
    packet received_message;
    int valread;

    while (true){

        valread = read(((ClientConnectionManegment*) ptr) -> sock, &received_message, 1024);
        if (valread > 0){
            ((ClientConnectionManegment*) ptr) -> notification.UpdateBufferToReceive(received_message.type, received_message.length, received_message.seqn, received_message.timestamp, received_message.payload);
        }
    }
}

void ClientConnectionManegment::setConnectionAddress(char adress[11]){
    strcpy(this->connection_address, adress);
}