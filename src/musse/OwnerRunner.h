
#ifndef MUSECLIENTRUNNER_H
#define MUSECLIENTRUNNER_H

#include "MUSSEUserRunner.h"
#include "Owner.h"
#include <memory>
#include <thread>
#include <atomic>
#include <grpc++/channel.h>
#include <mutex>
#include <condition_variable>

class MusesOwnerRunner : public Musse::Service {
private:
    Owner* client_;
    std::unique_ptr<Musse::Stub> stub_;
    // std::unique_ptr<Musse::Stub> fabric_;
    bool setupMode = false;
    vector<pair<prf_type, prf_type> > setupPairs;
    map<int, set<int> > accessList;

public:
    bool ForFU = false;
    MusesOwnerRunner(string serverAddress, int maxQueueSize, unsigned char* masterKey);
    virtual ~MusesOwnerRunner();
    int share(std::string keyword, int index, QueueBasedUser* user);
    int share(std::string keyword, int index, int curUserID);
    void unshare(int ind, QueueBasedUser* user, const TC<int>* testCase);
    void unshare(int ind, OMAPBasedUser* user, const TC<int>* testCase);
    void registerUser(int userID, unsigned char* key, int maxKeywordSize, bytes<Key> omapkey);
    void registerUser(int userID, unsigned char* key);
    void beginSetup();
    void endSetup();
    void endSetup(QueueBasedUser* user);
    int getOwnerID();
    vector< pair<int, unsigned char*> > downloadStash(int userID);
    void uploadStash(vector< pair<int, unsigned char*> > stash, int userID);
    void setOMAPRoots(Bid bid, int pos, int userID);
    pair<Bid, int> getOMAPRoot(int userID);
    void createRamStore(size_t num, size_t size, int userID);
    BlocksWithProof writeInStore(vector<int> pos, vector<block> b, int userID);
    BlocksWithProof readStore(vector<int> poses, int userID);
};

#endif /* MUSECLIENTRUNNER_H */

