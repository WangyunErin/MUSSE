#ifndef CLIENT_H
#define CLIENT_H
#include <string>
#include <map>
#include <vector>
#include <set>
#include <array>
#include "Server.h"
#include <iostream>
#include <sstream>
#include "musse/Server.h"
#include "utils/Utilities.h"
#include "OMAPBasedUser.h"
#include "QueueBasedUser.h"
using namespace std;

class Owner {
    friend class MusesOwnerRunner;
private:
    Server* server = NULL;
    MusesOwnerRunner* runner = NULL;
    unsigned char iv[AES_KEY_SIZE];
    unsigned char key[AES_KEY_SIZE];
    map<int, map<prf_type, int> > allFileCnt;
    map<int, unsigned char* > keys;
    map<int, OMAP*> omaps;
    vector<pair<prf_type,  prf_type> > KeyValues;
    map<int, map<Bid, string> > setupPairs;
    vector<Bid> sharekeywords;
    map<int, set<int> > accessList;
    bool setupMode = false;
    unsigned char valuesMerkleRoot[SHA256_DIGEST_LENGTH];
    unsigned char queuesMerkleRoot[SHA256_DIGEST_LENGTH];
    map<int, unsigned char*> omapsMerkleRoot;
    inline prf_type bitwiseXOR(int input1, prf_type input2);
    inline prf_type bitwiseXOR(prf_type input1, prf_type input2);
    inline void getAESRandomValue(unsigned char* keyword, int op, int counter, unsigned char* result, unsigned char* curkey);
    void uploadMerkleRoot(int userID);
    //void updateMerkleProof(prf_type val, int pos, map<int, unsigned char*> proofs, int treeSize, unsigned char* localRoot);
    void downloadMerkleRoot(int userID);


public:
    int userID;
    unsigned long int updateCommunicationSize = 0;
    bool ForFU = false;
    Owner(Server* server, unsigned char* masterKey, MusesOwnerRunner* runner);

    //int update(OP op, string keyword, int ind);
    pair<prf_type,prf_type> share(string keyword, int ind, QueueBasedUser* user);
    void unshare(int ind, QueueBasedUser* user, const TC<int>* testCase);
    pair<prf_type,prf_type> share(string keyword, int ind, int userID);
    void unshare(int ind, OMAPBasedUser* user, const TC<int>* testCase);
    int updateRequest(string keyword, int ind, prf_type& address, prf_type& value, QueueBasedUser* user);
    int updateRequest(string keyword, int ind, prf_type& addr, prf_type& val, int userID);
    virtual ~Owner();
    void registerUser(int userID, unsigned char* key, int maxKeywordSize, bytes<Key> omapkey);
    void registerUser(int userID, unsigned char* key, int maxKeywordSize, bytes<Key> omapkey, MusesOwnerRunner* ownerRunner);
    void registerUser(int userID, unsigned char* key);
    bool isUserQueueBased(int userID);
    prf_type getQueueValue(string keyword, int userID);
    void beginSetup();
    void endSetup();
    void endSetup(QueueBasedUser* user);
};

#endif /* CLIENT_H */
