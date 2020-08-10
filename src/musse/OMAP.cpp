#include "OMAP.h"
using namespace std;
#include "MUSSEUserRunner.h"
#include "OwnerRunner.h"

OMAP::OMAP(int maxSize, bytes<Key> key, Server* server, int userID) {
    treeHandler = new AVLTree(maxSize, key, server, userID);
    this->server = server;
    this->userID = userID;
    rootKey = 0;
}

OMAP::OMAP(int maxSize, bytes<Key> key, MusesUserRunner* runner, int userID) {
    treeHandler = new AVLTree(maxSize, key, runner, userID);
    rootKey = 0;
    this->userID = userID;
    this->runner = runner;
}

OMAP::OMAP(int maxSize, bytes<Key> key, MusesOwnerRunner* runner, int userID) {
    treeHandler = new AVLTree(maxSize, key, runner, userID);
    rootKey = 0;
    this->userID = userID;
    this->ownerrunner = runner;
}

OMAP::~OMAP() {

}

string OMAP::find(Bid key) {
    treeHandler->CommunicationSize = 0;
    downloadRoot();
    if (rootKey == 0) {
        return "";
    }
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    auto resNode = treeHandler->search(node, key);
    string res = "";
    if (resNode != NULL) {
        // res.assign(resNode->value.begin(), resNode->value.end());
        // res = res.c_str();
        res = to_string(resNode->value);
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    uploadRoot();
    CommunicationSize = treeHandler->CommunicationSize;
    return res;
}

void OMAP::insert(Bid key, string value) {
    downloadRoot();
    treeHandler->startOperation();
    if (rootKey == 0) {
        rootKey = treeHandler->insert(0, rootPos, key, value);
    } else {
        rootKey = treeHandler->insert(rootKey, rootPos, key, value);
    }
    treeHandler->finishOperation(false, rootKey, rootPos);
    uploadRoot();
}

void OMAP::printTree() {
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    treeHandler->printTree(node, 0);
    delete node;
    treeHandler->finishOperation(true, rootKey, rootPos);
}

/**
 * This function is used for batch insert which is used at the end of setup phase.
 */
void OMAP::batchInsert(map<Bid, string> pairs) {
    downloadRoot();
    treeHandler->startOperation(true);
    int cnt = 0;
    cout << "before insert" << endl;
    for (auto pair : pairs) {
        cnt++;
        if (rootKey == 0) {
            rootKey = treeHandler->insert(0, rootPos, pair.first, pair.second);
        } else {
            rootKey = treeHandler->insert(rootKey, rootPos, pair.first, pair.second);
        }
    }
    cout << "after insert" << endl;
    treeHandler->finishOperation(false, rootKey, rootPos);
    uploadRoot();
}

/**
 * This function is used for batch search which is used in the real search procedure
 */
vector<string> OMAP::batchSearch(vector<Bid> keys) {
    downloadRoot();
    vector<string> result;
    treeHandler->startOperation(false);
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;

    vector<Node*> resNodes;
    treeHandler->batchSearch(node, keys, &resNodes);
    for (Node* n : resNodes) {
        // string res;
        if (n != NULL) {
            // res.assign(n->value.begin(), n->value.end());
            result.push_back(to_string(n->value));
        } else {
            result.push_back("");
        }
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    uploadRoot();
    return result;
}

void OMAP::setupInsert(map<Bid, string> pairs) {
    downloadRoot();
    treeHandler->setupInsert(rootKey, rootPos, pairs);
    uploadRoot();
}

string OMAP::incrementFileCnt(Bid key) {
    treeHandler->CommunicationSize = 0;
    downloadRoot();
    string res = "";
    treeHandler->startOperation();
    Node* node = new Node();
    node->key = rootKey;
    node->pos = rootPos;
    cout<<"tree increment:"<<endl;
    res = treeHandler->incrementFileCnt(node, key);
    cout<<"tree increment finished"<<endl;
    if (res == "") {
        rootKey = treeHandler->insert(rootKey, rootPos, key, "1");
        res = "1";
    }
    treeHandler->finishOperation(true, rootKey, rootPos);
    uploadRoot();
    CommunicationSize += treeHandler->CommunicationSize;
    return res;
}

void OMAP::downloadRoot() {
    if (server != NULL) {
        auto rt = server->getOMAPRoot(userID);
        rootKey = rt.first;
        rootPos = rt.second;
    } else if (runner != NULL) {
        auto rt = runner->getOMAPRoot(userID);
        rootKey = rt.first;
        rootPos = rt.second;
    } else {
        auto rt = ownerrunner->getOMAPRoot(userID);
        rootKey = rt.first;
        rootPos = rt.second;
    }
    CommunicationSize += (sizeof (Bid) + 4);
}

void OMAP::uploadRoot() {
    if (server != NULL) {
        server->setOMAPRoots(rootKey, rootPos, userID);
    } else if (runner != NULL) {
        runner->setOMAPRoots(rootKey, rootPos, userID);
    } else {
        ownerrunner->setOMAPRoots(rootKey, rootPos, userID);
    }
    CommunicationSize += (sizeof (Bid) + 4);
}
