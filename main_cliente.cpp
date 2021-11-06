#include "Cliente/tcp_cliente.hpp"
#include "Cliente/interface_cliente.hpp"
#include "Cliente/frontend_tcp.hpp"
#include "utilitarios/notificacao.hpp"
using namespace std;

void *chamaInterface(void *arg){
    InterfaceCliente obj = * (InterfaceCliente *) arg;
    obj.CriaInterface();
    pthread_exit(NULL);
}

void *criaConexao(void *arg){
    GerenciaConexaoCliente obj = * (GerenciaConexaoCliente *) arg;
    obj.EstabeleceConexao();
    pthread_exit(NULL);
}

void *BootFrontend(void *arg){
    GerenciaConexaoFrontend *obj = (GerenciaConexaoFrontend *) arg;
    obj->estabeleceConexao();
    pthread_exit(NULL);
}

void *ConnectServidor(void *arg){
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

    //init semáforo unitário, esperar o frontend ser inicializado
    sem_init(frontend.coletaSocketMutex(), 0, 0);
    //inicializando frontend em uma nova thread
    pthread_create(&client_fe, NULL, BootFrontend, &frontend);
    //espera o frontend vincular um endereco
    sem_wait(frontend.coletaSocketMutex());
    //informando o endereço do frontend ao cliente
    connection.defineEnderecoConexao(frontend.coletaEndereco());

    //thread para se conectar ao servidor
    pthread_create(&server_fe, NULL, ConnectServidor, &frontend);
    
    //criando thread client
    pthread_create(&th2, NULL, criaConexao, &connection);
    pthread_create(&th1, NULL, chamaInterface, &client);

    pthread_join(th2, NULL);
    pthread_join(th1, NULL);
    pthread_join(client_fe, NULL);
    pthread_join(server_fe, NULL);

    return 0;
}
