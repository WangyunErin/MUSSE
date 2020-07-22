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
using namespace std;

class MKSEUser {
private:
    MKSEServer* server;
    unsigned char key[AES_KEY_SIZE];
    unsigned char iv[AES_KEY_SIZE];

public:
    MKSEUser(MKSEServer* server, unsigned char* masterKey);
    virtual ~MKSEUser();
    vector<int> search(string keyword);
    int userID;
    int searchCommunicationSize;

};

#endif /* MKSEUSER_H */

