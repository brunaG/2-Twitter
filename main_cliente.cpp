#include "Cliente/tcp_cliente.hpp"
#include "Cliente/interface_cliente.hpp"
#include "Cliente/frontend_tcp.hpp"
#include "utilitarios/notificacao.hpp"
using namespace std;

void *CallInterface(void *arg){
    InterfaceCliente obj = * (InterfaceCliente *) arg;
    obj.CriaInterface();
    pthread_exit(NULL);
}

void *OpenConnection(void *arg){
    GerenciaConexaoCliente obj = * (GerenciaConexaoCliente *) arg;
    obj.EstabeleceConexao();
    pthread_exit(NULL);
}

void *BootFrontend(void *arg){
    GerenciaConexaoFrontend *obj = (GerenciaConexaoFrontend *) arg;
    obj->estabeleceConexao();
    pthread_exit(NULL);
}

void *ConnectServer(void *arg){
    GerenciaConexaoFrontend *obj = (GerenciaConexaoFrontend *) arg;
    obj->ConectaServidor();
    pthread_exit(NULL);
}

int main()
{
    Notificacoes* notification = new Notificacoes();
    Notificacoes* notification_fe = new Notificacoes();
    InterfaceCliente client (*notification);
    GerenciaConexaoCliente connection (*notification);
    GerenciaConexaoFrontend frontend (*notification_fe);
    pthread_t th1, th2, client_fe, server_fe;

    //init unitary semaphore to wait while the frontend is booted
    sem_init(frontend.coletaSocketMutex(), 0, 0);
    //booting frontend on a new thread
    pthread_create(&client_fe, NULL, BootFrontend, &frontend);
    //wait for the frontend to bind its address
    sem_wait(frontend.coletaSocketMutex());
    //informing frontend's address to the client
    connection.defineEnderecoConexao(frontend.coletaEndereco());

    //thread to fe connect to server
    pthread_create(&server_fe, NULL, ConnectServer, &frontend);
    
    //creating client threads
    pthread_create(&th2, NULL, OpenConnection, &connection);
    pthread_create(&th1, NULL, CallInterface, &client);

    pthread_join(th2, NULL);
    pthread_join(th1, NULL);
    pthread_join(client_fe, NULL);
    pthread_join(server_fe, NULL);

    return 0;
}
