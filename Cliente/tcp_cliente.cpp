#include "tcp_cliente.hpp"

GerenciaConexaoCliente::GerenciaConexaoCliente(Notificacoes &obj):notification{obj}{
    //TO DO: We can modify the constructor to receive the port
    PORT = 8080;
    sock = 0;
}

int GerenciaConexaoCliente::EstabeleceConexao()
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

    pthread_create(&th, NULL, &GerenciaConexaoCliente::LeSocket, this);

    while (true){
        if(ColetaEnviaMensagem(&message) < 0){
            cout << "Error when sending the message" << endl;
        };
    }

    pthread_join(th, NULL);

    close(sock);
    return 0;
}

int GerenciaConexaoCliente::ColetaEnviaMensagem(packet *message){
    notification.ColetaNotificacoesEnviar(message);
    return send(sock , message, sizeof(*message), 0);
}

void* GerenciaConexaoCliente::LeSocket(void *ptr){
    packet received_message;
    int valread;

    while (true){

        valread = read(((GerenciaConexaoCliente*) ptr) -> sock, &received_message, 1024);
        if (valread > 0){
            ((GerenciaConexaoCliente*) ptr) -> notification.ColetaNotificacoesReceber(received_message.type, received_message.length, received_message.seqn, received_message.timestamp, received_message.payload);
        }
    }
}

void GerenciaConexaoCliente::defineEnderecoConexao(char adress[11]){
    strcpy(this->connection_address, adress);
}