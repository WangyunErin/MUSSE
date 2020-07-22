#include "MUSSEUserRunner.h"
#include "Owner.h"
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
#include "OMAPBasedUser.h"
#include "QueueBasedUser.h"

MusesUserRunner::MusesUserRunner(string serverAddress) {
    grpc::ChannelArguments options;
    options.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    options.SetMaxSendMessageSize(1024 * 1024 * 1024);
    std::shared_ptr<grpc::Channel> channel(grpc::CreateCustomChannel(serverAddress, grpc::InsecureChannelCredentials(), options));
    stub_ = Musse::NewStub(channel);
    grpc::ClientContext context;
    SetupMessage message;
    google::protobuf::Empty e;

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        cout << "Setup failed: " << std::endl;
    }
}

MusesUserRunner::~MusesUserRunner() {
}

vector<int> MusesUserRunner::search(std::string keyword, OMAPBasedUser* user_) {

    vector<prf_type> addresses, tokens;
    vector<int> result;
    SearchMessage message;
    SearchResponse response;
    grpc::ClientContext context2;


    prf_type k_w = user_->searchRequest(keyword, addresses);
    for (auto it : addresses) {
        message.add_address(it.data(), it.size());
    }

    grpc::Status status = stub_->search(&context2, message, &response);

    if (!status.ok()) {
        cout << "search failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    for (int i = 0; i < response.ciphertext_size(); i++) {
        prf_type item;
        copy(response.ciphertext(i).begin(), response.ciphertext(i).end(), item.begin());
        tokens.push_back(item);
    }

    user_->searchProcess(tokens, k_w, result);

    return result;
}

vector<int> MusesUserRunner::search1(std::string keyword, QueueBasedUser* user_) {

    //grpc::ClientContext context1;
   // SearchMessage1 message;
    //SearchResponse1 response;
    vector<prf_type> addresses, tokens;
    vector<int> result;

    //searchrequest in Queuebaseduser, copied to here
    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    bool keepsearching=1;
    int address_size=1000;
    int i=1;

    while(keepsearching){
        grpc::ClientContext context1;
        SearchMessage1 message;
        SearchResponse1 response;

        for (int j=i;j<i+address_size;j++) {
            prf_type rnd;
            user_->getAESRandomValue(k_w.data(), 0, j, rnd.data(), user_->key);
            message.add_address(rnd.data(), rnd.size());
        }
        i=i+address_size;
        
        grpc::Status status = stub_->search1(&context1, message, &response);

        if (!status.ok()) {
            cout << "search failed:" << std::endl;
            cout << status.error_message() << std::endl;
        }

        if(response.ciphertext_size()<address_size){
            keepsearching=0;
        }

        for (int k = 0; k < response.ciphertext_size(); k++) {
            prf_type item;
            prf_type notfound;
            memset(notfound.data(), 0, AES_KEY_SIZE);
            copy(response.ciphertext(k).begin(), response.ciphertext(k).end(), item.begin());
            if(item != notfound){
                tokens.push_back(item);
            }else{
                keepsearching=0;
            }
        }
    }

    user_->searchProcess(tokens, k_w, result);

    return result;
}

vector<int> MusesUserRunner::search(std::string keyword, QueueBasedUser* user_) {

    grpc::ClientContext context2;
    // grpc::ClientContext context3;
    SearchMessage message;
    SearchResponse response;
    vector<prf_type> addresses, tokens;
    vector<int> result;

    prf_type k_w = user_->searchRequest(keyword, addresses);
    for (auto it : addresses) {
        message.add_address(it.data(), it.size());
    }

    grpc::Status status = stub_->search(&context2, message, &response);

    if (!status.ok()) {
        cout << "search failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    for (int i = 0; i < response.ciphertext_size(); i++) {
        prf_type item;
        copy(response.ciphertext(i).begin(), response.ciphertext(i).end(), item.begin());
        tokens.push_back(item);
    }

    user_->searchProcess(tokens, k_w, result);

    return result;
}

void MusesUserRunner::createRamStore(size_t num, size_t size, int userID) {
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

BlocksWithProof MusesUserRunner::readStore(vector<int> poses, int userID) {
    grpc::ClientContext context;
    ReadStoreRequest message;
    SearchResponse response;
    for (auto pos : poses) {
        message.add_poses(pos);
    }
    message.set_userid(userID);

    grpc::Status status = stub_->readStore(&context, message, &response);

    if (!status.ok()) {
        cout << "readStore failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    BlocksWithProof result;
    for (int i = 0; i < response.ciphertext_size(); i++) {
        block item(response.ciphersize(i));
        memcpy(item.data(), (unsigned char*) response.ciphertext(i).data(), response.ciphersize(i));
        result.values.push_back(item);
        result.valuesPoses.push_back(response.cipherpositions(i));
    }
    result.treeSize = response.treesize();
    for (int j = 0; j < response.hash_size(); j++) {
        unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
        string proofStr = response.hash(j);
        memcpy(hash, (unsigned char*) proofStr.data(), SHA256_DIGEST_LENGTH);
        int id = response.proofposistions(j);
        result.proofs[id] = hash;
    }

    return result;
}

BlocksWithProof MusesUserRunner::writeInStore(vector<int> pos, vector<block> b, int userID) {
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
    result.treeSize = response.treesize();
    for (int j = 0; j < response.hash_size(); j++) {
        unsigned char* hash = new unsigned char[SHA256_DIGEST_LENGTH];
        string proofStr = response.hash(j);
        memcpy(hash, (unsigned char*) proofStr.data(), SHA256_DIGEST_LENGTH);
        int id = response.proofposistions(j);
        result.proofs[id] = hash;
    }
    return result;
}

vector<pair<int, unsigned char*> > MusesUserRunner::downloadStash(int userID) {
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

void MusesUserRunner::uploadStash(vector<pair<int, unsigned char*> > stash, int userID) {
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

pair<Bid, int> MusesUserRunner::getOMAPRoot(int userID) {
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

void MusesUserRunner::setOMAPRoots(Bid bid, int pos, int userID) {
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


// string MusesUserRunner::executeCommandOnFabric(string cmd) {
//     grpc::ClientContext context;
//     CommandMessage message;
//     ResultMessage response;
//     message.set_cmd(cmd.data(), cmd.length());
//     message.set_size(cmd.length());
//     grpc::Status status = fabric_->executeFabricCommand(&context, message, &response);

//     if (!status.ok()) {
//         cout << "fabric commnad execution failed:" << std::endl;
//         cout << status.error_message() << std::endl;
//     }
//     char* result = new char[response.size()];
//     memcpy(result, (unsigned char*) response.result().data(), response.size());
//     string resStr(result);
//     delete result;
//     return resStr;
// }

void MusesUserRunner::updateCounter(int size, QueueBasedUser* user_) {
    //downloadMerkleRoot(user_);

    grpc::ClientContext context2;
    grpc::ClientContext context3;
    FileCntQueueRequestMessage queueRequest;
    FileCntDiffMessage queueResponse;
    SearchMessage message;
    SearchResponse response;
    vector<prf_type> addresses, tokens;
    vector<int> result;

    queueRequest.set_userid(user_->userID);
    //grpc::Status status1 = stub_->getFileCntDiffWithLimit(&context3, queueRequest, &queueResponse);
    // if (!status1.ok()) {
    //     cout << "getting Merkle root failed:" << std::endl;
    //     cout << status1.error_message() << std::endl;
    // }
    for (int i = 0; i < queueResponse.value_size(); i++) {
        prf_type item;
        copy(queueResponse.value(i).begin(), queueResponse.value(i).end(), item.begin());
       // user_->updateLocalFileCnt(item);
    }
}
