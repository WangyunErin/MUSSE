#ifndef OMAP_H
#define OMAP_H
#include <iostream>
#include "ORAM.hpp"
#include <functional>
#include <fstream>
#include <cstdio>
#include <cstring>
#include <iostream>
#include "AVLTree.h"

using namespace std;

class OMAP {
private:
    Bid rootKey;
    int rootPos;
    Server* server = NULL;
    int userID;
    MusesUserRunner* runner = NULL;
    MusesOwnerRunner* ownerrunner = NULL;
    void downloadRoot();
    void uploadRoot();

public:
    int CommunicationSize;
    AVLTree* treeHandler;
    OMAP(int maxSize, bytes<Key> key, Server* server, int userID);
    OMAP(int maxSize, bytes<Key> key, MusesUserRunner* runner, int userID);
    OMAP(int maxSize, bytes<Key> key, MusesOwnerRunner* runner, int userID);
    virtual ~OMAP();
    void insert(Bid key, string value);
    string incrementFileCnt(Bid key);
    vector<string> incrementFileCnts(vector<Bid> Keys);
    string find(Bid key);
    void printTree();
    void batchInsert(map<Bid, string> pairs);
    void setupInsert(map<Bid, string> pairs);
    vector<string> batchSearch(vector<Bid> keys);
};

#endif /* OMAP_H */

