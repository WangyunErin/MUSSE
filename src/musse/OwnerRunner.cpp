#include "OwnerRunner.h"
#include "Owner.h"
#include "OMAPBasedUser.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <thread>
#include <fstream>
#include <grpc/grpc.h>
#include <grpc++/client_context.h>
#include <grpc++/create_channel.h>
#include <grpc++/security/credentials.h>
#include <bits/stl_vector.h>

MusesOwnerRunner::MusesOwnerRunner(string serverAddress, int maxQueueSize, unsigned char* masterKey) {
    grpc::ChannelArguments options;
    options.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    options.SetMaxSendMessageSize(1024 * 1024 * 1024);
    std::shared_ptr<grpc::Channel> channel(grpc::CreateCustomChannel(serverAddress, grpc::InsecureChannelCredentials(), options));
    stub_ = Musse::NewStub(channel);
    client_ = new Owner(NULL, masterKey, this);
    grpc::ClientContext context;
    SetupMessage message;
    google::protobuf::Empty e;
    message.set_maxqueuesize(maxQueueSize);

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        cout << "Setup failed: " << std::endl;
    }
}

MusesOwnerRunner::~MusesOwnerRunner() {
}

int MusesOwnerRunner::sharedata(std::vector<std::string> Keywords, int index, QueueBasedUser* user) {//index is document id
    grpc::ClientContext context;
    BatchUpdateMessage message;
    google::protobuf::Empty e;
    prf_type addr, val;
    int curUserID = user->userID;

    client_->ForFU = this->ForFU;

    if (accessList.count(curUserID) == 0) {
        accessList[curUserID] = set<int>();
    }
    accessList[curUserID].insert(index);

    for(auto keyword : Keywords){
        if(forFNU){
            client_->setupMode=true;
            client_->updateRequest(keyword, index, addr, val, curUserID);  
        }else
        {
            client_->updateRequest(keyword, index, addr, val, user);
        }

        message.add_address(addr.data(), addr.size());
        message.add_value(val.data(), val.size());
    }

    grpc::Status status = stub_->batchUpdate(&context, message, &e);
    
    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    if(forFNU){
        client_->setupMode=false;
    	for (auto item : client_->omaps) {
        	item.second->setupInsert(client_->setupPairs[item.first]);
    	}
    }
}

    return 0;
}

int MusesOwnerRunner::share(std::string keyword, int index, QueueBasedUser* user) {//index is document id
    grpc::ClientContext context;
    UpdateMessage message;
    google::protobuf::Empty e;
    prf_type addr, val;
    int curUserID = user->userID;

    client_->ForFU = this->ForFU;

    if (accessList.count(curUserID) == 0) {
        accessList[curUserID] = set<int>();
    }
    accessList[curUserID].insert(index);

    if (setupMode) {
        client_->updateRequest(keyword, index, addr, val, user);
        setupPairs.push_back(pair<prf_type, prf_type>(addr, val));
    } else {
        client_->updateRequest(keyword, index, addr, val, user);

        message.set_address(addr.data(), addr.size());
        message.set_value(val.data(), val.size());

        grpc::Status status = stub_->update(&context, message, &e);

        if (!status.ok()) {
            cout << "Update failed:" << std::endl;
            cout << status.error_message() << std::endl;
        }
    }
    return 0;
}

int MusesOwnerRunner::share(std::string keyword, int index, int curUserID) {//index is document id, for OMAP
    grpc::ClientContext context;
    UpdateMessage message;
    google::protobuf::Empty e;
    prf_type addr, val;

    if (accessList.count(curUserID) == 0) {
        accessList[curUserID] = set<int>();
    }
    accessList[curUserID].insert(index);
//    cout<<"access list added"<<endl;
    if (setupMode) {
        client_->updateRequest(keyword, index, addr, val, curUserID);
        setupPairs.push_back(pair<prf_type, prf_type>(addr, val));
    } else {
        client_->updateRequest(keyword, index, addr, val, curUserID);
//        cout<<"updaterequest finished"<<endl;
        message.set_address(addr.data(), addr.size());
        message.set_value(val.data(), val.size());
//        cout<<"server update:"<<endl;
        grpc::Status status = stub_->update(&context, message, &e);

        if (!status.ok()) {
            cout << "Update failed:" << std::endl;
            cout << status.error_message() << std::endl;
        }
    }
    return 0;
}

void MusesOwnerRunner::unshare(int ind, QueueBasedUser* user, const TC<int>* testCase){
    int curUserID = user->userID;
    accessList[curUserID].clear();
    client_->allFileCnt[curUserID].clear();
    client_->ForFU = this->ForFU;
    beginSetup();
    unsigned char newuserKey[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        newuserKey[i] = rand() % 256;
    }
    registerUser(curUserID, newuserKey);
    memcpy(user->key, newuserKey, AES_KEY_SIZE);

    map<string, vector<int> > newPairs;
    newPairs = testCase->filePairs;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {
        vector<int>::iterator it=cur->second.begin();
        for(;it != cur->second.end();) {
            if(*it==ind){
                cur->second.erase(it);
                break;
            }
        }
    }

    vector<pair<prf_type,prf_type>> KeyValues;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {   //for each keyword 
        for (unsigned int j = 0; j < cur->second.size(); j++) {  //for each file id
            share(cur->first, cur->second[j], user);
        }
    }

    if(ForFU){
        endSetup(user);
    }else{
        endSetup();
    }
    cout << "Unshare finished." << endl;
}

void MusesOwnerRunner::unshare(int ind, OMAPBasedUser* user, const TC<int>* testCase){
    accessList[user->userID].clear();

    beginSetup();
    unsigned char newuserKey[AES_KEY_SIZE];
    for (int i = 0; i < AES_KEY_SIZE; i++) {
        newuserKey[i] = rand() % 256;
    }
    registerUser(user->userID, newuserKey, testCase->K * 2, user->omapkey);
    memcpy(user->key, newuserKey, AES_KEY_SIZE);

    map<string, vector<int> > newPairs;
    newPairs = testCase->filePairs;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {
        vector<int>::iterator it=cur->second.begin();
        for(;it != cur->second.end();) {
            if(*it==ind){
                cur->second.erase(it);
                break;
            }
        }
    }

    vector<pair<prf_type,prf_type>> KeyValues;
    for (auto cur = newPairs.begin(); cur != newPairs.end(); cur++) {   //for each keyword 
        for (unsigned int j = 0; j < cur->second.size(); j++) {  //for each file id
            share(cur->first, cur->second[j], user->userID);
        }
    }

    endSetup();
  
    cout << "Unshare finished." << endl;
}

void MusesOwnerRunner::registerUser(int userID, unsigned char* key, int maxKeywordSize, bytes<Key> omapkey) {
    client_->registerUser(userID, key, maxKeywordSize, omapkey, this);
}

void MusesOwnerRunner::registerUser(int userID, unsigned char* key) {
    client_->registerUser(userID, key);
}

void MusesOwnerRunner::beginSetup() {
    this->setupMode = true;
    client_->beginSetup();
}

void MusesOwnerRunner::endSetup() {
    this->setupMode = false;
    client_->setupMode = false;
    grpc::ClientContext context;
    BatchUpdateMessage message;
    google::protobuf::Empty e;
    for (auto keyValue : setupPairs) {
        prf_type addr = keyValue.first;
        prf_type val = keyValue.second;
        message.add_address(addr.data(), addr.size());
        message.add_value(val.data(), val.size());
    }

    grpc::Status status = stub_->batchUpdate(&context, message, &e);

    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    if(forFNU){
    	for (auto item : client_->omaps) {
        	item.second->setupInsert(client_->setupPairs[item.first]);
    	}
    }
}

void MusesOwnerRunner::endSetup(QueueBasedUser* user) {
    this->setupMode = false;
    client_->endSetup(user);
    grpc::ClientContext context;
    BatchUpdateMessage message;
    google::protobuf::Empty e;
    for (auto keyValue : setupPairs) {
        prf_type addr = keyValue.first;
        prf_type val = keyValue.second;
        message.add_address(addr.data(), addr.size());
        message.add_value(val.data(), val.size());
    }

    grpc::Status status = stub_->batchUpdate(&context, message, &e);

    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }
}

int MusesOwnerRunner::getOwnerID() {
    return client_->userID;
}

vector<pair<int, unsigned char*> > MusesOwnerRunner::downloadStash(int userID) {
    vector<pair<int, unsigned char*> > result;
    grpc::ClientContext context;
    DownloadStashMessage message;
    DownloadStashResponse response;
    message.set_userid(userID);
    grpc::Status status = stub_->downloadStash(&context, message, &response);

    if (!status.ok()) {
        cout << "Stash download failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    for (int i = 0; i < response.pos_size(); i++) {
        unsigned char* cipher = new unsigned char[response.pos(i)];
        memcpy(cipher, (unsigned char*) response.value(i).data(), response.pos(i));
        result.push_back(pair<int, unsigned char*> (response.pos(i), cipher));
    }
    return result;
}

void MusesOwnerRunner::uploadStash(vector<pair<int, unsigned char*> > stash, int userID) {
    grpc::ClientContext context;
    UploadStashMessage message;
    google::protobuf::Empty e;
    message.set_userid(userID);
    for (auto node : stash) {
        message.add_pos(node.first);
        message.add_value(node.second, node.first);
    }
    grpc::Status status = stub_->uploadStash(&context, message, &e);

    if (!status.ok()) {
        cout << "Stash upload failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }
}

pair<Bid, int> MusesOwnerRunner::getOMAPRoot(int userID) {
    pair<Bid, int> res;
    grpc::ClientContext context;
    DownloadRootMessage message;
    DownloadRootResponse response;
    message.set_userid(userID);
    grpc::Status status = stub_->downloadOMAPRoot(&context, message, &response);

    if (!status.ok()) {
        cout << "OMAP root download failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    memcpy(res.first.id.data(), (unsigned char*) response.value().data(), ID_SIZE);
    res.second = response.pos();
    return res;
}

void MusesOwnerRunner::setOMAPRoots(Bid bid, int pos, int userID) {
    grpc::ClientContext context;
    UploadRootMessage message;
    google::protobuf::Empty e;
    message.set_pos(pos);
    message.set_userid(userID);
    message.set_value((unsigned char*) bid.id.data(), ID_SIZE);
    grpc::Status status = stub_->uploadOMAPRoot(&context, message, &e);
    if (!status.ok()) {
        cout << "OMAP root upload failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }
}

void MusesOwnerRunner::createRamStore(size_t num, size_t size, int userID) {
    grpc::ClientContext context;
    CreateOMAPMessage message;
    google::protobuf::Empty e;

    message.set_num(num);
    message.set_userid(userID);
    grpc::Status status = stub_->createOMAP(&context, message, &e);

    if (!status.ok()) {
        cout << "Create OMAP failed: " << std::endl;
    }
}

BlocksWithProof MusesOwnerRunner::readStore(vector<int> poses, int userID) {
    grpc::ClientContext context;
    ReadStoreRequest message;
    SearchResponse response;
    for (auto pos : poses) {
        message.add_poses(pos);
    }
    message.set_userid(userID);
    //cout<<"grpc readstore start"<<endl;
    grpc::Status status = stub_->readStore(&context, message, &response);

    if (!status.ok()) {
        cout << "readStore failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }
    //cout<<"readstore finished"<<endl;
    BlocksWithProof result;
    for (int i = 0; i < response.ciphertext_size(); i++) {
        block item(response.ciphersize(i));
        memcpy(item.data(), (unsigned char*) response.ciphertext(i).data(), response.ciphersize(i));
        result.values.push_back(item);
        result.valuesPoses.push_back(response.cipherpositions(i));
    }
    // result.treeSize = response.treesize();
    // for (int j = 0; j < response.hash_size(); j++) {
    //     unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
    //     string proofStr = response.hash(j);
    //     memcpy(hash, (unsigned char*) proofStr.data(), SHA256_DIGEST_LENGTH);
    //     int id = response.proofposistions(j);
    //     result.proofs[id] = hash;
    // }

    return result;
    cout<<"readstore finished"<<endl;
}

BlocksWithProof MusesOwnerRunner::writeInStore(vector<int> pos, vector<block> b, int userID) {
    grpc::ClientContext context;
    WriteStoreRequest message;
    SearchResponse response;
    message.set_userid(userID);
    for (unsigned int i = 0; i < pos.size(); i++) {
        message.add_pos(pos[i]);
        message.add_value((unsigned char*) b[i].data(), b[i].size());
        message.add_size(b[i].size());
    }

    grpc::Status status = stub_->writeInStore(&context, message, &response);

    if (!status.ok()) {
        cout << "writeStore failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    BlocksWithProof result;
    result.values.insert(result.values.begin(), b.begin(), b.end());
    for (int i = 0; i < response.ciphertext_size(); i++) {
        result.valuesPoses.push_back(response.cipherpositions(i));
    }
    // result.treeSize = response.treesize();
    // for (int j = 0; j < response.hash_size(); j++) {
    //     unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
    //     string proofStr = response.hash(j);
    //     memcpy(hash, (unsigned char*) proofStr.data(), SHA256_DIGEST_LENGTH);
    //     int id = response.proofposistions(j);
    //     result.proofs[id] = hash;
    // }
    return result;
}
