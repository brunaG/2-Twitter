#ifndef PENDING_USERS_H
#define PENDING_USERS_H

#include <vector>
using namespace std;

typedef struct{
    string name;
    int seqn;
    int timestamp;
    int length_message;
    string message;
} pending_users;

#endif
