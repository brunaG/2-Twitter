#include "Server/server_tcp.hpp"

using namespace std;

void *maintainConnection(void *arg){
    ServerConnectionManagement obj = * (ServerConnectionManagement *) arg;
    
    if (obj.getType() == "primario")
        obj.establishConnection();
    else
        obj.runAsReplica();
    pthread_exit(NULL);
}

int main(int argc, char** argv){
    int status;
    pthread_t clientsThread;

    if (argc != 4){
        cout << "Missing arguments! To run use: './server <ID> <IP address> <server type: primario or replica>'" << endl;  
        exit(-1);
    }

    string arg1 = argv[2];
    string arg2 = argv[3];
    ServerConnectionManagement currentServer(atoi(argv[1]), arg1, arg2);

    status = pthread_create(&clientsThread, NULL, maintainConnection, &currentServer);
    if (status != 0)
        exit(1);

    status = pthread_join (clientsThread, NULL);
    if (status != 0)
        exit(1);

    return 0;
}

