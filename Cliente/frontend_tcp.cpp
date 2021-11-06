#include <ctime>
#include "frontend_tcp.hpp"

typedef struct thread_args{
    int socket_;
    GerenciaConexaoFrontend* obj;
} Thread_args;


GerenciaConexaoFrontend::GerenciaConexaoFrontend(Notificacoes &obj):notification{obj}{
    PORT = 8080;
    strcpy(addr, "127.0.0.");

    // inicializa os sockets para -1, portanto não verificado
    client_socket = -1;
    server_socket = -1;

    sem_init(&mutex_socket, 0, 1);
};

int GerenciaConexaoFrontend::estabeleceConexao(){
    int opt = 1;
    int master_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(struct sockaddr_in);

    //cria o socket master
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == -1 )
    {
        perror("socket failed");
        sem_post(&mutex_socket);
        exit(EXIT_FAILURE);
    }
    //necessario para reutilizar o socket apos o fechamento
    setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // definindo endereço do frontend
    address.sin_family = AF_INET;
    address.sin_port = htons( PORT );
    //intervalo de endereços de 127.0.0.10 a 127.0.0.50
    address.sin_addr.s_addr = inet_addr("127.0.0.10");
    bzero(&(address.sin_zero), 8);

    // vincular o socket à porta localhost 8080
    int endaddr = 11;
    string newadress = "127.0.0.10";
    while ( (bind(master_socket, (struct sockaddr *)&address, sizeof(address)) < 0) && (endaddr <= 50) )
    {
        // tentando outro endereço
        newadress = "127.0.0." + to_string(endaddr);
        address.sin_addr.s_addr = inet_addr(newadress.c_str());
        endaddr++;
    }

    // atribuir endereço obtido
    strcpy(this->addr, newadress.c_str());
    printf("Listener on port %d, address %s\n", PORT, addr);

    sem_post(&mutex_socket);

    if (endaddr > 50) {
        perror("ligacao falhou");
        exit(EXIT_FAILURE);
    }

    // tente especificar no máximo 1 conexão pendente para o socket mestre  
    if (listen(master_socket, 1) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Aguardando conexões ...");

    //accepting new connection
    struct sockaddr_in client_address;
    while (client_socket < 0)
    {
        if ((client_socket = accept(master_socket, (struct sockaddr *) &client_address,
                &addrlen)) == -1)
        {
            perror("aceito");
            exit(EXIT_FAILURE);
        }

        // criando threads para ler do cliente e enviar para o cliente
        pthread_create(&th_read_client, NULL, &GerenciaConexaoFrontend::LeCliente, this); 
        pthread_create(&th_read_server, NULL, &GerenciaConexaoFrontend::LeServidor, this);  

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

    printf("Conectando ao Servidor\n");

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "\n erro ao criar o Socket  \n";
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Converte os endereços IPv4 e IPv6 do formato de texto para o formato binário
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0)
    {
        cout << "\n Endereco invalido ou não suportado \n";
        return -1;
    }

    if (connect(server_socket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        cout << "\n Conexao falhou **\n";
        return -1;
    }

    pthread_create(&th, NULL, &GerenciaConexaoFrontend::LeServidor, this);

    while (true){
        if(ColetaEnviaMensagem(&message) < 0){
            cout << "Erro ao enviar mensagem" << endl;
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
            cout << "Host desconectado" << endl;
            close( socket );
            pthread_exit(0);
        }

        if (connection.LeMensagem(pkt, connection.server_socket) < 0)
            printf("Falha ao enviar mensagem para o servidor\n");
    }
}

void GerenciaConexaoFrontend::imprimePacote (packet pkt){
  //  printf("type: %d\n", pkt.type);
 //   printf("seqn: %d\n", pkt.seqn);
  //  printf("length: %d\n", pkt.length);
 //   printf("timestamp: %d\n", pkt.timestamp);
 //   printf("payload: ");
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
            printf("Falha ao ler do servidor\n");
        } else {
            if (connection.LeMensagem(received_message, connection.client_socket) < 0)
            printf("FRONTEND: Falha ao encaminhar mensagem\n");
        }
    }
}

int GerenciaConexaoFrontend::LeMensagem(packet pkt, int socket){
    return send(socket , &pkt, sizeof(pkt), 0);
}