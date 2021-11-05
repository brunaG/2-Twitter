#include <ctime>
#include "frontend_tcp.hpp"

typedef struct thread_args{
    int socket_;
    GerenciaConexaoFrontend* obj;
} Thread_args;


GerenciaConexaoFrontend::GerenciaConexaoFrontend(Notificacoes &obj):notification{obj}{
    PORT = 8080;
    strcpy(addr, "127.0.0.");

    //initialise sockets to -1 so not checked
    client_socket = -1;
    server_socket = -1;

    sem_init(&mutex_socket, 0, 1);
};

int GerenciaConexaoFrontend::estabeleceConexao(){
    int opt = 1;
    int master_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == -1 )
    {
        perror("socket failed");
        sem_post(&mutex_socket);
        exit(EXIT_FAILURE);
    }
    //needed to reutilize the socket after its closed
    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    //setting frontend address
    address.sin_family = AF_INET;
    address.sin_port = htons( PORT );
    //address range from 127.0.0.10 to 127.0.0.50
    address.sin_addr.s_addr = inet_addr("127.0.0.10");
    bzero(&(address.sin_zero), 8);

    //bind the socket to localhost port 8080
    int endaddr = 11;
    string newadress = "127.0.0.10";
    while ( (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) && (endaddr <= 50) )
    {
        //trying another address
        newadress = "127.0.0." + to_string(endaddr);
        address.sin_addr.s_addr = inet_addr(newadress.c_str());
        endaddr++;
    }

    //assign obtained address
    strcpy(this->addr, newadress.c_str());
    printf("Listener on port %d, address %s\n", PORT, addr);

    sem_post(&mutex_socket);

    if (endaddr > 50) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //try to specify maximum of 1 pending connection for the master socket
    if (listen(master_socket, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    //accepting new connection
    struct sockaddr_in client_address;
    while (client_socket < 0)
    {
        if ((client_socket = accept(master_socket, (struct sockaddr *) &client_address,
                &addrlen)) == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        //creating threads to read from client and send to client
        pthread_create(&th_read_client, NULL, &GerenciaConexaoFrontend::LeCliente, this);  //TODO
        pthread_create(&th_read_server, NULL, &GerenciaConexaoFrontend::LeServidor, this);  //TODO

        pthread_join(th_read_client, NULL);
        pthread_join(th_read_server, NULL);
    }

    if (close(master_socket) < 0)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }

    return 0;

}

int GerenciaConexaoFrontend::ConectaServidor(){
    struct sockaddr_in serv_addr;
    packet message;
    pthread_t th;

    printf("Connecting to Servidor\n");

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "\n Socket creation error \n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        cout << "\nInvalid address/ Address not supported \n";
        return -1;
    }

    if (connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "\nConnection Failed**\n";
        return -1;
    }

    pthread_create(&th, NULL, &GerenciaConexaoFrontend::LeServidor, this);

    while (true){
        if(ColetaEnviaMensagem(&message) < 0){
            cout << "Error when sending the message" << endl;
        };
    }

    pthread_join(th, NULL);

    close(server_socket);

    return 0;
}

int GerenciaConexaoFrontend::ColetaEnviaMensagem(packet *message){
    notification.ColetaNotificacoesEnviar(message);
    return send(server_socket , message, sizeof(*message), 0);
}

void* GerenciaConexaoFrontend::LeCliente(void *arg){
    packet pkt;
    GerenciaConexaoFrontend connection = *(GerenciaConexaoFrontend *) arg;
    int socket = connection.client_socket;

    while(true){
        if (read( socket , &pkt, sizeof(pkt)) == 0){
            cout << "Host disconnected" << endl;
            close( socket );
            pthread_exit(0);
        }

        if (connection.LeMensagem(pkt, connection.server_socket) < 0)
            printf("Failed to send message to server\n");
    }
}

void GerenciaConexaoFrontend::imprimePacote (packet pkt){
    printf("type: %d\n", pkt.type);
    printf("seqn: %d\n", pkt.seqn);
    printf("length: %d\n", pkt.length);
    printf("timestamp: %d\n", pkt.timestamp);
    printf("payload: ");
    cout << pkt.payload << endl;

}

char* GerenciaConexaoFrontend::coletaEndereco(){
    return addr;
}

sem_t* GerenciaConexaoFrontend::coletaSocketMutex(){
    return &this->mutex_socket;
}

void* GerenciaConexaoFrontend::LeServidor(void *ptr){
    packet received_message;
    GerenciaConexaoFrontend connection = *(GerenciaConexaoFrontend *) ptr;

    while (true){
        if( read(connection.server_socket, &received_message, 1024) < 0){
            printf("Failed to read from server\n");
        } else {
            if (connection.LeMensagem(received_message, connection.client_socket) < 0)
            printf("FRONTEND: Failed to foward message\n");
        }
    }
}

int GerenciaConexaoFrontend::LeMensagem(packet pkt, int socket){
    return send(socket , &pkt, sizeof(pkt), 0);
}