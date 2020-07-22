#ifndef QUSER_H
#define QUSER_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include "Server.h"
#include <iostream>
#include <sstream>
#include "musse/Server.h"
#include "utils/Utilities.h"
#include "OMAP.h"
using namespace std;

class MusesUserRunner;

class QueueBasedUser {
    friend class Owner;
    friend class MusesOwnerRunner;
    friend class MusesUserRunner;

private:
    Server* server = NULL;
    MusesUserRunner* runner = NULL;
    unsigned char key[AES_KEY_SIZE];
    map<prf_type, int> FileCnt;
    unsigned char iv[AES_KEY_SIZE];

    inline prf_type bitwiseXOR(int input1, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    void getAESRandomValue(unsigned char* keyword, int op, int counter, unsigned char* result, unsigned char* curkey);
    //void updateLocalFileCnt();
   // void downloadMerkleRoot();
    

public:
    int userID;
    int ownerID;
    int searchCommunicationSize = 0;
    unsigned char valuesMerkleRoot[SHA256_DIGEST_LENGTH];
    unsigned char queueMerkleRoot[SHA256_DIGEST_LENGTH];
    QueueBasedUser(Server* server, unsigned char* masterKey, int ownerID, MusesUserRunner* runner);
    vector<int> search(string keyword);
    vector<int> search1(string keyword);
    prf_type searchRequest(string keyword, vector<prf_type>& tokens);
    prf_type searchRequest1(string keyword, vector<prf_type>& tokens);
    void searchProcess(vector<prf_type> values, prf_type k_w, vector<int>& finalRes);
    virtual ~QueueBasedUser();
    void updateLocalFileCnt(prf_type item);
};

#endif /* QUSER_H */
