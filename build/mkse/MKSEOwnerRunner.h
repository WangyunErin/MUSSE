
#ifndef MUSECLIENTRUNNER_H
#define MUSECLIENTRUNNER_H

// #include "MKSEUserRunner.h"
#include "MKSEOwner.h"
#include "mkse.grpc.pb.h"
#include <memory>
#include <thread>
#include <atomic>
#include <map>
#include <grpc++/channel.h>
#include <mutex>
#include <condition_variable>
#include <utility>

using namespace std;

class MKSEOwnerRunner : public MKSE::Service {
private:
    MKSEOwner* client_;
    std::unique_ptr<MKSE::Stub> stub_;
    std::map<prf_type, std::pair<int, int>> setuppairs;

public:
    MKSEOwnerRunner(string serverAddress, unsigned char* masterKey);
    virtual ~MKSEOwnerRunner();
    int getRofFile(int ind);
    int share(std::string keyword, int ind, int userID);
    int sharedata(std::vector<std::string> Keywords, int ind, int userID);
    void registerUser(int userID, unsigned char* key);
    void beginSetup();
    void endSetup();
};

#endif /* MUSECLIENTRUNNER_H */

