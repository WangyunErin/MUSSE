#ifndef MKSEUSER_H
#define MKSEUSER_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <sstream>
#include "utils/Utilities.h"
#include "MKSEServer.h"
#include "MKSEUserRunner.h"
using namespace std;

class MKSEUserRunner;

class MKSEUser {
    friend class MKSEUserRunner;
private:
    MKSEServer* server;
    MKSEUserRunner* runner=NULL;
    unsigned char key[AES_KEY_SIZE];
    unsigned char iv[AES_KEY_SIZE];

public:
    MKSEUser(MKSEServer* server, unsigned char* masterKey, MKSEUserRunner* runner);
    virtual ~MKSEUser();
    vector<int> search(string keyword);
    int userID;
    int searchCommunicationSize;

};

#endif /* MKSEUSER_H */

