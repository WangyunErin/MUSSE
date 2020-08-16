#include "MKSEOwner.h"
#include "MKSEOwnerRunner.h"
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

MKSEOwnerRunner::MKSEOwnerRunner(string serverAddress, int maxQueueSize, unsigned char* masterKey) {
    grpc::ChannelArguments options;
    options.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    options.SetMaxSendMessageSize(1024 * 1024 * 1024);
    std::shared_ptr<grpc::Channel> channel(grpc::CreateCustomChannel(serverAddress, grpc::InsecureChannelCredentials(), options));
    stub_ = MKSE::NewStub(channel);
    client_ = new MKSEOwner(NULL, masterKey, this);
    grpc::ClientContext context;
    SetupMessage message;
    google::protobuf::Empty e;
    message.set_maxqueuesize(maxQueueSize);

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        cout << "Setup failed: " << std::endl;
    }
}

MKSEOwnerRunner::~MKSEOwnerRunner() {
}

int MKSEOwnerRunner::getRofFile(int ind){
    grpc::ClientContext context;
    getRofFileRequest message;
    getRofFileResponse response;

    message.set_ind(ind);
    grpc::Status status = stub_->getRofFile(&context, message, &response);

    if (!status.ok()) {
        cout << "getRofFile failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    int result =response.r();
    return result;
}

int MKSEOwnerRunner::share(std::string keyword, int ind, int userID) {//index is document id
    int r = getRofFile(ind);
    if (r == -1) {
        r = rand();
    }

    prf_type dcipher = client_->sharerequest(keyword, ind, userID,r);
    setuppairs[dcipher]=pair<int, int>(ind, r);
    
    return 0;
}

int MKSEOwnerRunner::sharedata(std::vector<std::string> Keywords, int ind, int userID) {//index is document id
    grpc::ClientContext context;
    BatchUpdateMessage message;
    google::protobuf::Empty e;


    int r = getRofFile(ind);
    if (r == -1) {
        r = rand();
    }

    for(auto keyword : Keywords){
        prf_type dcipher = client_->sharerequest(keyword, ind, userID,r);

        message.add_r(r);
        message.add_dcipher(dcipher.data(), dcipher.size());
        message.add_ind(ind);
    }
    
    grpc::Status status = stub_->batchUpdate(&context, message, &e);

    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }
    
    return 0;
}

void MKSEOwnerRunner::registerUser(int userID, unsigned char* key) {
    client_->registerUser(userID, key);
}

void MKSEOwnerRunner::beginSetup() {
    client_->beginSetup();
}

void MKSEOwnerRunner::endSetup() {
    client_->endSetup();
    grpc::ClientContext context;
    BatchUpdateMessage message;
    google::protobuf::Empty e;

    map<prf_type,pair<int,int>>::iterator iter;
    for(iter = setuppairs.begin(); iter != setuppairs.end(); iter++) {
        message.add_dcipher(iter->first.data(), iter->first.size());
        std::pair<int,int> p= iter->second;
        message.add_ind(p.first);
        message.add_r(p.second);
    }
    grpc::Status status = stub_->batchUpdate(&context, message, &e);

    if (!status.ok()) {
        cout << "Update failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }    
}