#include "src/mkse/MKSEServerRunner.h"
#include <grpc++/server.h>
#include <grpc++/server_builder.h>

#include <stdio.h>
#include <csignal>
#include <unistd.h>

int main(int argc, char** argv) {
    MKSEServerRunner service;
    grpc::ServerBuilder builder;
    builder.SetMaxReceiveMessageSize(1024 * 1024 * 1024);
    builder.SetMaxSendMessageSize(1024 * 1024 * 1024);
    std::cout<<"set succeed"<<std::endl;
    builder.AddListeningPort("0.0.0.0:50051", grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    std::cout<<"builder register succeed"<<std::endl;
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout<<"server is waiting"<<std::endl;
    server->Wait();
    return 0;
}