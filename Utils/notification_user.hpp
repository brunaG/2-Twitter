#ifndef NOTIFICATION_USER_H
#define NOTIFICATION_USER_H
#include "pending_users.hpp"
using namespace std;

typedef struct{
    string profile;
    vector<string> tweet;
    vector<pending_users> pending_notifications;
} notification_user;

#endif
