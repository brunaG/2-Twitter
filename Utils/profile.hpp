#ifndef PROFILE_H
#define PROFILE_H

#include <vector>
using namespace std;

typedef struct{
    string name;
    vector<string> followers;
    vector<string> following;
    int session1;
    int session2;
} profile;

#endif
