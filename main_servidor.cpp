#include "Servidor/tcp_servidor.hpp"

using namespace std;

void *manterconexao(void *arg) {
    ServerConnectionManagement obj = *(ServerConnectionManagement *) arg;

    if (obj.getType() == "primario")
        obj.establishConnection();
    else
        obj.runAsReplica();
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int status;
    pthread_t clientsThread;

    if (argc != 4) {
        cout << "Argumentos Invalidos! './server <ID> <ENDERECO IP> <primario or replica>'"
             << endl;
        exit(-1);
    }

    string arg1 = argv[2];
    string arg2 = argv[3];
    ServerConnectionManagement currentServer(atoi(argv[1]), arg1, arg2);

    status = pthread_create(&clientsThread, NULL, manterconexao, &currentServer);
    if (status != 0)
        exit(1);

    status = pthread_join(clientsThread, NULL);
    if (status != 0)
        exit(1);

    return 0;
}

