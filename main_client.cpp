#include "Client/client_tcp.hpp"
#include "Client/client_interface.hpp"
#include "Client/frontend_tcp.hpp"
#include "Utils/notification.hpp"
using namespace std;

void *CallInterface(void *arg){
    ClientInterface obj = * (ClientInterface *) arg;
    obj.CreateInterface();
    pthread_exit(NULL);
}

void *OpenConnection(void *arg){
    ClientConnectionManegment obj = * (ClientConnectionManegment *) arg;
    obj.StabilishConection();
    pthread_exit(NULL);
}

void *BootFrontend(void *arg){
    FrontendConnectionManagement *obj = (FrontendConnectionManagement *) arg;
    obj->establishConnection();
    pthread_exit(NULL);
}

void *ConnectServer(void *arg){
    FrontendConnectionManagement *obj = (FrontendConnectionManagement *) arg;
    obj->ConnectToServer();
    pthread_exit(NULL);
}

int main()
{
    Notifications* notification = new Notifications();
    Notifications* notification_fe = new Notifications();
    ClientInterface client (*notification);
    ClientConnectionManegment connection (*notification);
    FrontendConnectionManagement frontend (*notification_fe);
    pthread_t th1, th2, client_fe, server_fe;

    //init unitary semaphore to wait while the frontend is booted
    sem_init(frontend.getMutexSocket(), 0, 0);
    //booting frontend on a new thread
    pthread_create(&client_fe, NULL, BootFrontend, &frontend);
    //wait for the frontend to bind its address
    sem_wait(frontend.getMutexSocket());
    //informing frontend's address to the client
    connection.setConnectionAddress(frontend.getAdress());

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
