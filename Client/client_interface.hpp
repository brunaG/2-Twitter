#ifndef CLIENT_INTERFACE_H
#define CLIENT_INTERFACE_H
#include <ctime>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <string.h>
#include "../Utils/notification.hpp"
#define TYPE_USER 0
#define TYPE_COMMAND 1
using namespace std;

class ClientInterface{
    private:
    string profile;
    Notifications &notification;
    int seqn;

    void SetProfile(string user_profile);
    void SaveProfileName();
    void ProcessCommand(string command,int type);
    string ReadCommand();

    public:
    ClientInterface(Notifications &obj);
    bool isProfileInvalid(string user_profile);
    void CreateInterface();
    string GetProfile();
    static void* GetNewNotification(void *ptr);
};

#endif
