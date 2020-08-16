#include "MKSEUserRunner.h"
#include "MKSEOwner.h"
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

MKSEUserRunner::MKSEUserRunner(string serverAddress) {
    grpc::ChannelArguments options;
    options.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    options.SetMaxSendMessageSize(1024 * 1024 * 1024);
    std::shared_ptr<grpc::Channel> channel(grpc::CreateCustomChannel(serverAddress, grpc::InsecureChannelCredentials(), options));
    stub_ = MKSE::NewStub(channel);
    grpc::ClientContext context;
    SetupMessage message;
    google::protobuf::Empty e;

    grpc::Status status = stub_->setup(&context, message, &e);

    if (!status.ok()) {
        cout << "Setup failed: " << std::endl;
    }
}

MKSEUserRunner::~MKSEUserRunner() {
}

vector<int> MKSEUserRunner::search(std::string keyword, MKSEUser* user_) {

    vector<int> result;
    SearchMessage message;
    SearchResponse response;
    grpc::ClientContext context;

    prf_type k_w;
    memset(k_w.data(), 0, AES_KEY_SIZE);
    copy(keyword.begin(), keyword.end(), k_w.data());
    prf_type kprime;
    Utilities::encrypt((unsigned char*) k_w.data(), AES_KEY_SIZE - 1, user_->key, user_-> iv, (unsigned char*) kprime.data());

    message.set_q(kprime.data(), kprime.size());

    grpc::Status status = stub_->search(&context, message, &response);

    if (!status.ok()) {
        cout << "search failed:" << std::endl;
        cout << status.error_message() << std::endl;
    }

    for (int i = 0; i < response.id_size(); i++) {
        int item = response.id(i);
        result.push_back(item);
    }

    return result;
}
