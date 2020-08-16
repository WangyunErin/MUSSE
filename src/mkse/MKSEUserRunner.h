
#ifndef OMAPUSERRUNNER_H
#define OMAPUSERRUNNER_H

#include "MKSEOwner.h"
#include "MKSEUser.h"
#include "mkse.grpc.pb.h"

#include <memory>
#include <thread>
#include <atomic>
#include <grpc++/channel.h>

#include <mutex>
#include <condition_variable>
class OMAPBasedUser;
class QueueBasedUser;
class MKSEUser;

class MKSEUserRunner : public MKSE::Service {
    friend class MKSEOwnerRunner;

private:
    std::unique_ptr<MKSE::Stub> stub_;

public:
    MKSEUserRunner(string serverAddress);
    virtual ~MKSEUserRunner();
    vector<int> search(std::string keyword, MKSEUser* user_);
};

#endif /* OMAPUSERRUNNER_H */

