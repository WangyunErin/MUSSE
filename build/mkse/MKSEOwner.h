#ifndef MKSEOWNER_H
#define MKSEOWNER_H
#include <cstring>
#include <string>
#include <map>
#include <vector>
#include <set>
#include <array>
#include <iostream>
#include <sstream>
#include "utils/Utilities.h"
#include "MKSEServer.h"
using namespace std;

class MKSEOwnerRunner;

class MKSEOwner {
private:
    unsigned char iv[AES_KEY_SIZE];
    unsigned char key[AES_KEY_SIZE];
    map<int, unsigned char* > keys;
    vector<string> keywords;
    MKSEServer* server;
    int userID;
    bool setupMode;

public:
    int updateCommunicationSize;
    virtual ~MKSEOwner();

    MKSEOwner(MKSEServer* server, unsigned char* masterKey);

    void shareData(string keyword, int ind, int userID);
    void registerUser(int userID, unsigned char* key);
    void beginSetup();
    void endSetup();

};

#endif /* MKSEOWNER_H */

