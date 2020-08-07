#ifndef SERVER_H
#define SERVER_H
#include <string>
#include <map>
#include <vector>
#include <array>
#include <iostream>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <vector>
#include "utils/Utilities.h"
#include "utils/MerkleTree.h"
#include "RAMStore.hpp"
#include "Bid.h"


using namespace std;

class ResultWithProof {
public:
    // int treeSize;
    vector<prf_type> values;
    vector<int> valuesPoses;
    // map<int, unsigned char*> proofs;
};

class BlocksWithProof {
public:
    // int treeSize;
    vector<block> values;
    vector<int> valuesPoses;
    // map<int, unsigned char*> proofs;
};

class Server {
private:
    // bool deleteFiles;
    int counter = 0;
    int maxCounter;
    // MerkleTree valuesMerkleTree;
    // MerkleTree queuesMerkleTree;
    // map<int, MerkleTree*> omapsMerkleTree;
    map<prf_type,  prf_type > DictW;
    // map<int, vector<prf_type> > FileCntQueues;
    map<int, RAMStore*> ramStores;
    map<int, vector< pair<int, unsigned char*> > > stashes;
    map<int, pair<Bid, int> > omaproots;
    // map<int, int> usersQueuePointer;
    // map<int, int> usersOrder;

public:
    Server(int maxCounterSize);
    void update(prf_type addr, prf_type val);
    void update(vector<pair<prf_type,  prf_type> > KeyValues);
    vector<prf_type> search(vector<prf_type> KList);
    prf_type search(prf_type addr);
    void createRamStore(size_t num, int userID);
    BlocksWithProof writeInStore(vector<int> poses, vector<block> blocks, int userID);
    BlocksWithProof readStore(vector<int> poses, int userID);
    // ResultWithProof addToFileCntQueue(int userID, prf_type newItem);
    // ResultWithProof getQueueValues(int userID);
    // ResultWithProof getQueueValues(int userID, int remain);
    virtual ~Server();
    vector< pair<int, unsigned char*> > downloadStash(int userID);
    void uploadStash(vector< pair<int, unsigned char*> > stash, int userID);
    void setOMAPRoots(Bid bid, int pos, int userID);
    pair<Bid, int> getOMAPRoot(int userID);
    // unsigned char valuesMerkleRoot[SHA256_DIGEST_LENGTH];
    // unsigned char queuesMerkleRoot[SHA256_DIGEST_LENGTH];
    // map<int, unsigned char*> omapsMerkleRoot;
    bool keepsearching;
};

#endif /* SERVER_H */

