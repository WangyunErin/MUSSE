
#ifndef OMAPUSERRUNNER_H
#define OMAPUSERRUNNER_H

#include "Owner.h"

#include "musse.grpc.pb.h"

#include <memory>
#include <thread>
#include <atomic>
#include <grpc++/channel.h>

#include <mutex>
#include <condition_variable>
class OMAPBasedUser;
class QueueBasedUser;

class MusesUserRunner : public Musse::Service {
    friend class MusesOwnerRunner;

private:
    std::unique_ptr<Musse::Stub> stub_;
    std::unique_ptr<Musse::Stub> fabric_;

public:
    MusesUserRunner(string serverAddress);
    virtual ~MusesUserRunner();
    vector<int> search(std::string keyword, OMAPBasedUser* user_);
    vector<int> search1(std::string keyword, QueueBasedUser* user_);
    vector<int> search(std::string keyword, QueueBasedUser* user_);
    void createRamStore(size_t num, size_t size, int userID);
    BlocksWithProof writeInStore(vector<int> pos, vector<block> b, int userID);
    BlocksWithProof readStore(vector<int> poses, int userID);
    vector< pair<int, unsigned char*> > downloadStash(int userID);
    void uploadStash(vector< pair<int, unsigned char*> > stash, int userID);
    // void uploadMerkleRoot(OMAPBasedUser* user_);
    // void downloadMerkleRoot(OMAPBasedUser* user_);
    // void downloadMerkleRoot(QueueBasedUser* user_);
    void setOMAPRoots(Bid bid, int pos, int userID);
    pair<Bid, int> getOMAPRoot(int userID);
    // string executeCommandOnFabric(string cmd);
    // void updateCounter(int size, QueueBasedUser* user_);
};

#endif /* OMAPUSERRUNNER_H */

