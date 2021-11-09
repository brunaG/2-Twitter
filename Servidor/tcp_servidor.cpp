#include <ctime>
#include "tcp_servidor.hpp"

typedef struct thread_args{
    int socket_;
    ServerConnectionManagement* obj;
    Usuario* users_obj;

} Thread_args;

typedef struct election_args{
    int socket_;
    bool* resp;
    int* id;

} Election_args;

typedef struct confirm_args{
    vector<Replica> *replicas;
    packet pkt;
    int socket;
    Usuario* users_obj;
} Confirm_args;

string addressToStr(sockaddr_in address){
    struct sockaddr_in* pV4Addr = (struct sockaddr_in*)&address;
    struct in_addr ipAddr = pV4Addr->sin_addr;

    char str[INET_ADDRSTRLEN];
    inet_ntop( AF_INET, &ipAddr, str, INET_ADDRSTRLEN );

    string ipstr(str);

    return ipstr;
}

ServerConnectionManagement::ServerConnectionManagement(int id, string ip, string type){
    PORT = 8080;
    max_clients = 30;
    IP_addr = ip;
    ID_server = id;
    type_server = type;
    users = Usuario();
    users.defineID(id);

    //initialise all client_socket[] to 0 so not checked
    for (int i = 0; i < max_clients; i++)
    {
        client_socket[i] = 0;
    }

    sem_init(&mutex_socket, 0, 1);
};

int ServerConnectionManagement::establishConnection(){
    int opt = 1;
    int master_socket, addrlen, new_socket, activity, i, sd;
    int max_sd;
    struct sockaddr_in address;

    SetConfigurations();

    //set of socket descriptors
    fd_set readfds;

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    char ip[IP_addr.size()+1];
    strcpy(ip, IP_addr.c_str());

    cout << "Endereco IP : " << ip << endl;
    cout << "ID: " << ID_server << endl;
    cout << "Tipo de Servidor: " << type_server << endl;

    for (int i=0; i< (int) replicas.size(); i++){
        cout << "Replica: " << replicas[i].pegaEndereco() << endl;
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    //printf("Listener on port %d \n", PORT);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //create thread to listen to new replicas trying to join the replicas' group
    pthread_t listen_group;
    pthread_create(&listen_group, NULL, &ServerConnectionManagement::listenToGroup, this);

    //accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");

    while(true)
    {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < max_clients ; i++)
        {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL ,
        //so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);

        if ((activity < 0) && (errno!=EINTR))
        {
            printf("select error");
        }

        //If something happened on the master socket ,
        //then its an incoming connection
        if (FD_ISSET(master_socket, &readfds))
        {
            if ((new_socket = accept(master_socket,
                    (struct sockaddr *)&address, (socklen_t*)&addrlen))<0)
            {
                perror("ACEITO");
                exit(EXIT_FAILURE);
            }

            //inform user of socket number - used in send and receive commands
            printf("Nova conexao, socket fd  %d , ip  : %s , porta : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));

            int index = addNewSocket(new_socket);
            addNewIP( addressToStr(address) );

            thread_args th_struct;

            th_struct.socket_ = new_socket;
            th_struct.obj = this;
            th_struct.users_obj = &users;

            pthread_create(&th_send[index], NULL, &ServerConnectionManagement::SendMessage, &th_struct);
            pthread_create(&th_read[index], NULL, &ServerConnectionManagement::ReadMessage, &th_struct);
        }
    }

    return 0;

}

bool ServerConnectionManagement::isSocketAlive(int socket){
    bool isAlive = false;
    int i;

    sem_wait(&mutex_socket);
    for (i = 0; i < max_clients; i++){
        if (socket == client_socket[i]){
            isAlive = true;
            break;
        }
    }
    sem_post(&mutex_socket);

    return isAlive;
}

int ServerConnectionManagement::addNewSocket(int socket){
    int i;

    sem_wait(&mutex_socket);
    for (i = 0; i < max_clients; i++)
    {
        //if position is empty
        if( client_socket[i] == 0 )
        {
            client_socket[i] = socket;
            //printf("Adding to list of sockets as %d\n" , i);

            break;
        }
    }
    sem_post(&mutex_socket);
    return i;
}

void ServerConnectionManagement::SetConfigurations(){
    string line, addrs;
    ifstream file_config (FILE_CONFIG_PATH);
    string delimeter = ",";
    string replica_ip;
    int pos;

    if (file_config.is_open()){
        while(getline (file_config, line)){
            addrs = line;
            while ((pos=addrs.find(delimeter)) != (int) std::string::npos){
                replica_ip = addrs.substr(0, pos);
                addrs.erase(0, pos + delimeter.length());
                Replica rep(replica_ip);
                if (IP_addr != replica_ip){
                    replicas.push_back(rep);
                }
            }
        }
        file_config.close();
    }
    else{
        cout << "File config not found!" << endl;
    }

    if (IP_addr != addrs){
        Replica rep(addrs);
        replicas.push_back(rep);
    }
}

void ServerConnectionManagement::deleteSocket(int socket){
    int i;
    sem_wait(&mutex_socket);
    for (i = 0; i < max_clients; i++){
        if (socket == client_socket[i]){
            client_socket[i] = 0;
            break;
        }
    }
    users.DeletaSocketUsuario(socket);
    sem_post(&mutex_socket);
}

void* ServerConnectionManagement::SendMessage(void *arg){
    packet notification;
    thread_args th_struct = *(thread_args *) arg;
    int socket = th_struct.socket_;

    while (true){
        //TO DO: Replace using the buffer of the notification
        if (!((ServerConnectionManagement*) th_struct.obj) -> isSocketAlive(socket)){
            pthread_exit(0);
            break;
        }
        else{
            int erro = ((Usuario*) th_struct.users_obj) -> ColetaMensagem(socket, &notification);
            if (erro == -1){
                cout << "No message!"<< endl;
            }
            else{
                if(send(socket, &notification, sizeof(notification), MSG_NOSIGNAL) != sizeof(notification) ){
                        perror("send");
                }
            }
        }
    }
}

void* ServerConnectionManagement::ReadMessage(void *arg){
    packet pkt;
    thread_args th_struct = *(thread_args *) arg;
    int socket = th_struct.socket_;
    vector<Replica>* replicas_list = ((ServerConnectionManagement *) th_struct.obj)->getReplicas();

    while(true){
        if (read( socket , &pkt, sizeof(pkt)) == 0){
            cout << "Host desconectado" << endl;
            close( socket );
            ((ServerConnectionManagement *) th_struct.obj)->deleteSocket(socket);
            // TO DO: Call user LogOut method
            pthread_exit(0);
        }

printPacket(pkt);

        confirm_args args;
        args.pkt = pkt;
        args.replicas = replicas_list;
        args.socket = socket;
        args.users_obj = ((Usuario*) th_struct.users_obj);
        pthread_t th_confirm;
        pthread_create(&th_confirm, NULL, &ServerConnectionManagement::ConfirmTransaction, &args);
        

    }
}

void* ServerConnectionManagement::ConfirmTransaction(void *arg){
    confirm_args arg_struct = *(confirm_args *) arg;
    vector<pthread_t> threads;
    vector<Replica> *replicas_list = arg_struct.replicas;
    packet pkt = arg_struct.pkt;
    int socket = arg_struct.socket;

    int i = 0;
    while ((*replicas_list)[i].coletaSocket() > -1){
   // for (int i = 0; i < (int) replicas_list->size(); i++){
        //cout << "Replica socket: " << (*replicas_list)[i].getSocket() << endl;
    //    if ( (*replicas_list)[i].coletaSocket() < 0)
      //      continue;
        
        //building packet to replica
        EDPacoteReplica pkt_rep;
        pkt_rep.seqn = pkt.seqn;
        pkt_rep.type = pkt.type;
        pkt_rep.timestamp = pkt.timestamp;
        pkt_rep.socket = socket;
        pkt_rep.length = pkt.length;
        strncpy(pkt_rep.payload, pkt.payload, sizeof(pkt_rep.payload));

        //sending the packet
        if( send((*replicas_list)[i].coletaSocket(), &pkt_rep, sizeof(pkt_rep), MSG_NOSIGNAL) != sizeof(pkt_rep) ) {
            //perror("send");
            cout << "Replica " << (*replicas_list)[i].coletaID() << " disconnected! Ignoring it..." << endl;
            close ( (*replicas_list)[i].coletaSocket() );
            (*replicas_list)[i].defineSocket(-1);
        }
        //if packet was send with success, wait for acknowledgement
        else {
           // bzero(&pkt, sizeof(pkt));
           packet response;
            //if (read( replicas_list , &pkt, sizeof(pkt)) <= 0) {
            if (read( (*replicas_list)[i].getSocket() , &response, sizeof(response)) <= 0) {
                perror("read");
                cout << "Replica " << (*replicas_list)[i].coletaID() << " could not answer! Disconnecting it..." << endl;
                close ( (*replicas_list)[i].coletaSocket() );
                (*replicas_list)[i].defineSocket(-1);
                continue;
            }
            printf("\nReplica %d : recebeu com sucesso a informacao!\n", (*replicas_list)[i].coletaID());
        }
    }

    ((Usuario*) arg_struct.users_obj) -> novaMensagemCliente(pkt, socket);
    
    return 0;
}

void ServerConnectionManagement::imprimePacote (packet pkt){
   // printf("type: %d\n", pkt.type);
    //printf("seqn: %d\n", pkt.seqn);
    //printf("length: %d\n", pkt.length);
  //  printf("timestamp: %d\n", pkt.timestamp);
   // printf("payload: ");
    cout << pkt.payload << endl;
}

int ServerConnectionManagement::connectToGroup(){
    int primaryindex = -1;

    for (int i = 0; i < (int) replicas.size(); i++){
        //creating the socket
        int sockfd;
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            cout << "\n Socket creation error \n";
            continue;
        }
        //setting timeout for socket
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 0;
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        struct sockaddr_in localaddr;
        localaddr.sin_family = AF_INET;
        localaddr.sin_addr.s_addr = inet_addr(this->IP_addr.c_str());
        localaddr.sin_port = 0;

        //bindind the socket to its own ip address
        if (bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr)) < 0)
        {
            perror("bind failed");
            continue;
        }

        //getting replica's address
        struct sockaddr_in struct_address;
        string address = replicas[i].pegaEndereco();

        struct_address.sin_family = AF_INET;
        struct_address.sin_port = htons(30000);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if(inet_pton(AF_INET, address.c_str(), &struct_address.sin_addr)<=0)
        {
            cout << "\nInvalid address/ Address not supported \n";
            continue;
        }

        //trying to connect to the replica
        if (connect(sockfd, (struct sockaddr *)&struct_address, sizeof(struct_address)) < 0)
        {
            cout << "Failed to connect to **" << address.c_str() << endl;
            continue;
        }

        //sending its own id and requesting replicas' id
        packet idpacket;
        idpacket.type = 3;
        idpacket.seqn = 0;
        idpacket.length = sizeof(to_string(this->ID_server).c_str());
        strcpy(idpacket.payload, to_string(this->ID_server).c_str());
        if( send(sockfd, &idpacket, sizeof(idpacket), MSG_NOSIGNAL) != sizeof(idpacket) ){
            perror("send");
            cout << "Failed to send identification packet to **" << address.c_str() << endl;
            continue;
        }

        //waiting for response
        packet response;
        if (read( sockfd , &response, sizeof(response)) <= 0){
            cout << "Failed to read response id message from " << address.c_str() << endl;
            continue;
        } else
            replicas[i].defineID(atoi(response.payload));

        if (response.type == 4)
            primaryindex = i;

        //finally adds the socket to the replica's entry on *replicas vector*
        replicas[i].defineSocket(sockfd);
    }

    return primaryindex;
}

void* ServerConnectionManagement::listenToGroup(void *arg){
    int opt = 1;
    int master_socket;
    struct sockaddr_in address;
    ServerConnectionManagement *localserver = (ServerConnectionManagement *) arg;

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0)
    {
        perror("group listener socket failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections ,
    //this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
          sizeof(opt)) < 0 )
    {
        perror("group setsockopt");
        exit(EXIT_FAILURE);
    }

    char ip[localserver->getIP().size()+1];
    strcpy(ip, localserver->getIP().c_str());

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr(ip);
    address.sin_port = htons( 30000 );

    //bind the socket to localhost port 30000
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
   //  printf("Group listener on port %d \n", 30000);

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in replica_address;
    socklen_t addrlen = sizeof(struct sockaddr_in);
    addrlen = sizeof(replica_address);
    while(true)
    {
        int replica_socket = -1;
        if ((replica_socket = accept(master_socket, (struct sockaddr *) &replica_address,
                &addrlen)) < 0)
        {
            perror(" nova replica aceita");
            continue;
        }
        struct timeval tv;
        tv.tv_sec = 2;
        tv.tv_usec = 2000000;
        setsockopt(replica_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);

        //update replica's socket
        string ip_address = addressToStr(replica_address);
        vector<Replica>* replicas = localserver->getReplicas();
        for(int i = 0; i < (int) replicas->size(); i++)
        {
            if ( ip_address == (*replicas)[i].pegaEndereco() )
            {
                int greeting_result;
                greeting_result = localserver->execGreetingProtocol(replica_socket, &(*replicas)[i]);

                if(greeting_result >= 0)
                {
                    (*replicas)[i].defineSocket(replica_socket);
                    printf("Replica  %s conectada \n", ip_address.c_str());
                }
                break;
            }
        }
    }
}

int ServerConnectionManagement::execGreetingProtocol(int sockfd, Replica *replica){
    ServerConnectionManagement *localserver = this;
    packet idpacket;
    if (read( sockfd , &idpacket, sizeof(idpacket)) <= 0) {
        cout << "Failed to read id message" << endl;
        return -1;
    }else
        replica->defineID(atoi(idpacket.payload));

    packet response;
    if(localserver->getType() == "primario")
        response.type = 4;
    else
        response.type = 3;
    response.seqn = 0;
    response.length = sizeof(to_string(localserver->ID_server).c_str());
    strcpy(response.payload, to_string(localserver->ID_server).c_str());
    if( send(sockfd, &response, sizeof(response), MSG_NOSIGNAL) != sizeof(response) ){
        perror("send");
        cout << "Failed to send identification packet **" << endl;
        return -1;
    }

    return 0;
}

string ServerConnectionManagement::getType(){
    return this->type_server;
}

string ServerConnectionManagement::getIP(){
    return this->IP_addr;
}

int ServerConnectionManagement::getID(){
    return this->ID_server;
}

vector<Replica>* ServerConnectionManagement::getReplicas(){
    return &this->replicas;
}

int ServerConnectionManagement::runAsReplica(){
    int p_index = 0;
    Replica primary = Replica();

    SetConfigurations();
    p_index = connectToGroup();
    if (p_index >= 0){
        primary = replicas[p_index];
        printf("Servidor id %d é o lider\n", primary.coletaID());
        lider = primary.coletaID();
    }
    else{
        printf("Servidor Primario Nao Encontrado\n");
        // START ELECTION
    }

    //create thread to listen to new replicas trying to join the replicas' group
    pthread_t listen;
    pthread_create(&listen, NULL, &ServerConnectionManagement::listenToGroup, this);

    //TODO quase tudo
    while (true){
        //does something
        //read the primary server
        packet pkt;
        EDPacoteReplica pkt_rep;
        int primary_socket = -1;
        int status;

        for (int i=0; i<(int)replicas.size();i++){
            if (replicas[i].coletaID() == lider){
                primary_socket = replicas[i].coletaSocket();
            }
        }

        status = read( primary_socket , &pkt_rep, sizeof(pkt_rep));
        if (status == 0) {
            cout << "Servidor Primario Desconectado!" << endl;
            startElection();
           // cout << "*****LIDER: " << lider << endl;
         
           for (int i = 0; i < max_clients; i++)
            {
                if( client_ip[i].size() != 0 )
                {
                    string ip = client_ip[i];
                    cout << "IP cliente " << ip << endl;
                    connectFrontEnd(ip);
                }
            }

        }
        else if (status > 0) {
            pkt.length = pkt_rep.length;
            //pkt.payload = pkt_rep.payload;
            strncpy(pkt.payload, pkt_rep.payload, sizeof(pkt.payload));
            pkt.seqn = pkt_rep.seqn;
            pkt.type = pkt_rep.type;
            pkt.timestamp = pkt_rep.timestamp;
            int sck = pkt_rep.socket;

            imprimePacote(pkt);

            //proccess message
            users.novaMensagemCliente(pkt, sck);
            if (pkt_rep.type == 6){
                int i;

                for (i = 0; i < max_clients; i++)
                {
                    //if position is empty
                    if( client_ip[i].size() <= 0 )
                    {
                        client_ip[i] = pkt_rep.payload;
                        printf("Adicionando na lista de ips:  %d\n" , i);
                        break;
                    }
                }
            }

            //send acknowledgement
            packet response;
            response.type = 5;
            response.seqn = 0;
            response.length = sizeof("ACK");
            response.timestamp = time(0);
            strcpy(response.payload, "ACK");
            if( send(primary_socket, &response, sizeof(response), 0) != sizeof(response) ){
                cout << "falha ao enviar para o servidor" << endl;
               close(primary_socket);
                exit(EXIT_FAILURE);
              } else {
                cout << "recebeu com sucesso a informacao no socket \n" << primary_socket << endl;                
            }
            }
        }

    }
    return 0;
}
void ServerConnectionManagement::connectFrontEnd(string ip_addrs){
    struct sockaddr_in serv_address;
    packet message;
    int sock = 0;
    string lider_ip;

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cout << "\n Socket creation error \n";
    }

    serv_address.sin_family = AF_INET;
    serv_address.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, ip_addrs.c_str(), &serv_address.sin_addr)<=0)
    {
        cout << "\nInvalid address/ Address not supported \n";
    }

    if (connect(sock, (struct sockaddr *)&serv_address, sizeof(serv_address)) < 0)
    {
        cout << "\nConnection Failed**\n";
    }

    for (int i; i < (int) replicas.size(); i++){
        if (lider == replicas[i].getID()){
            lider_ip = replicas[i].getAddress();
            break;
        }
    }
    message.type = 4;
    message.seqn = 0;
    message.length = sizeof(lider_ip.c_str());
    message.timestamp = time(0);
    strcpy(message.payload, lider_ip.c_str());
    if (send(sock , &message, sizeof(message), 0) < 1){
        cout << "Error sending message to frontend!" << endl;
    }

    close(sock);

}

void ServerConnectionManagement::startElection(){
    int contador;
    int lider_atual = lider;
    bool resposta;
    vector<int> maiores_sock;
    pthread_t listen[4];
    pthread_t listen_lider[4];

     //Enquanto ainda nao tem novo lider
    while(lider == lider_atual)
    {
        resposta = false; // *** Quando receber uma resposta, setar esse resposta

        contador = 0;
        // Envia mensagem com seu id para as replicas com id maior
        for(int i=0; i< (int) replicas.size();i++) //vetor das replicas
        {
            // Se a replica do la�o tiver id maior que a replica atual
            if(replicas[i].coletaID() > ID_server)  // *** Percorrer o vetor de replicas
            {
                //Envia pkt
                //TO DO: CRIAR O PKT PARA ENVIAR
                maiores_sock.push_back(replicas[i].coletaSocket());
                packet idpacket;
                idpacket.type = 3;
                idpacket.seqn = 0;
                idpacket.length = sizeof(to_string(ID_server).c_str());
                strcpy(idpacket.payload, to_string(ID_server).c_str());
            //    cout << "Sending election to: " << replicas[i].coletaSocket() << endl;
              //  cout << "Sending election to ID: " << replicas[i].coletaID() << endl;
                int status = send(replicas[i].coletaSocket(), &idpacket, sizeof(idpacket), MSG_NOSIGNAL);
                if( status < 0 ){
                    perror("send");
                    cout << "Failed to send packet to election**" << endl;
                } // *** send(indicador de mensagem de elei��o) para as outras replicas
                contador++;
            }
        }
        //sleep(20000);
        // Se o contador n�o incrementou, n�o enviou msg pra ningu�m e ele � o l�der
        if(contador == 0)
        {
            //sleep(1);
            // for enviando mensagens pras replicas se auto proclamando lider
            for(int i=0; i< (int) replicas.size();i++){
                //TO DO: CRIAR O PKT PARA ENVIAR
                packet idpacket;
                idpacket.type = 4;
                idpacket.seqn = 0;
                idpacket.length = sizeof(to_string(ID_server).c_str());
                strcpy(idpacket.payload, to_string(ID_server).c_str());
                if( send(replicas[i].coletaSocket(), &idpacket, sizeof(idpacket), MSG_NOSIGNAL) != sizeof(idpacket) ){
                    perror("send");
                    cout << "Failed to send packet to election**" << endl;
                }

            }
            lider_atual = ID_server;

            //Atualiza informa��es do l�der (porta? socket?)
        }
        else // Sen�o, aguarda uma resposta
        {
            cout << "Mensagens enviadas. Aguardando retorno...\n";

            for (int i=0; i< (int) maiores_sock.size();i++){
                election_args elect_th;
                elect_th.resp = &resposta;
                elect_th.socket_ = maiores_sock[i];
                elect_th.id = &lider_atual;
                pthread_create(&listen[i], NULL, &ServerConnectionManagement::readElection, &elect_th);
            }
            sleep(2);
            // Se dentro do TIMEOUT ninguem mandou nenhuma mensagem, sou o lider
            if(!resposta)
            {
                for(int i=0; i< (int) replicas.size();i++){
                    packet idpacket;
                    idpacket.type = 4;
                    idpacket.seqn = 0;
                    idpacket.length = sizeof(to_string(ID_server).c_str());
                    strcpy(idpacket.payload, to_string(ID_server).c_str());
                    if( send(replicas[i].coletaSocket(), &idpacket, sizeof(idpacket), MSG_NOSIGNAL) != sizeof(idpacket) ){
                        perror("send");
                        cout << "Failed to send packet to election**" << endl;
                    } // *** send(indicador de novo lider) para as outras replicas
                }
                lider_atual = ID_server;
            }
            else
            {
             //   sleep(2);
                for(int i=0; i< (int) replicas.size();i++){
                    election_args elect_th;
                    elect_th.resp = &resposta;
                    elect_th.socket_ = replicas[i].coletaSocket();
                    elect_th.id = &lider_atual;
                    pthread_create(&listen_lider[i], NULL, &ServerConnectionManagement::waitLider, &elect_th);
                }

            }

        }

    }

    lider = lider_atual;

    cout << "Eleicao finalizada. Novo lider: " << lider << "___\n";

}

void* ServerConnectionManagement::readElection(void* arg){
    election_args elect_th = *(election_args *) arg;
    bool* resposta = (bool*) elect_th.resp;
    packet idpacket;
    int* id = (int*) elect_th.id;

    signal(SIGALRM, alarm_handler);
    alarm(2);
    if (read( elect_th.socket_ , &idpacket, sizeof(idpacket)) <= 0) {
       cout << "Not possible to read from socket " << elect_th.socket_ << endl;
    }
    else{
        *resposta = true;
        *id = atoi(idpacket.payload);
    }
    pthread_exit(NULL);
}

void ServerConnectionManagement::alarm_handler(int value){
    cout << "TIMEOUT" << endl;
}

void* ServerConnectionManagement::waitLider(void* arg){
    election_args elect_th = *(election_args *) arg;
    bool* resposta = (bool*) elect_th.resp;
    packet idpacket;
    int* id = (int*) elect_th.id;

    signal(SIGALRM, alarm_handler);
    alarm(2);

    if (read( elect_th.socket_ , &idpacket, sizeof(idpacket)) <= 0) {
         cout << "Not possible to read from socket " << elect_th.socket_ << endl;
    }
    else{
        *resposta = true;
        *id = atoi(idpacket.payload);
    }
    pthread_exit(NULL);
}

int ServerConnectionManagement::addNewIP(string ip){
    int i;

    for (i = 0; i < max_clients; i++)
    {
        //if position is empty
        if( client_ip[i].size() <= 0 )
        {
            client_ip[i] = ip;
            printf("Adding to list of ips as %d\n" , i);
            for(int i=0; i< (int) replicas.size();i++){
                //TO DO: CRIAR O PKT PARA ENVIAR
                packet idpacket;
                idpacket.type = 6;
                idpacket.seqn = 0;
                idpacket.length = sizeof(ip.c_str());
                strcpy(idpacket.payload, ip.c_str());
                if( send(replicas[i].getSocket(), &idpacket, sizeof(idpacket), MSG_NOSIGNAL) != sizeof(idpacket) ){
                    perror("send");
                    cout << "Failed to send packet to election**" << endl;
                }

            }
            break;
        }
    }

    return i;
}
