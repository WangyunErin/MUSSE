#include "MKSEServerRunner.h"

MKSEServerRunner::MKSEServerRunner() {
}

MKSEServerRunner::~MKSEServerRunner() {
}

grpc::Status MKSEServerRunner::setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) {
    server_ = make_unique<MKSEServer>(request->maxqueuesize());
    return grpc::Status::OK;
}

grpc::Status MKSEServerRunner::update(grpc::ServerContext* context, const UpdateMessage* mes, google::protobuf::Empty* e) {
    if (!server_) {
        //  the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
    prf_type dcipher;
    int r = mes->r();
    int ind = mes->ind();
    std::copy(mes->dcipher().begin(), mes->dcipher().end(), dcipher.begin());

    server_->update(r,dcipher,ind);

    return grpc::Status::OK;
}

grpc::Status MKSEServerRunner::batchUpdate(grpc::ServerContext* context, const BatchUpdateMessage* mes, google::protobuf::Empty* e) {
    if (!server_) {
        // the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }

    for (int i = 0; i < mes->r_size(); i++) {
        prf_type dcipher;
        int r = mes->r(i);
        int ind = mes->ind(i);
        std::copy(mes->dcipher(i).begin(), mes->dcipher(i).end(), dcipher.begin());
        server_->update(r,dcipher,ind);
    }

    return grpc::Status::OK;
}

grpc::Status MKSEServerRunner::search(grpc::ServerContext* context, const SearchMessage* message, SearchResponse* response) {
    if (!server_) {
        // the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
    prf_type q;
    copy(message->q().begin(), message->q().end(), q.begin());

    vector<int> result = server_->search1(q);

    for (unsigned int i = 0; i < result.size(); i++) {
        response->add_id(result[i]);
    }
    return grpc::Status::OK;
}

grpc::Status MKSEServerRunner::getRofFile(grpc::ServerContext* context, const getRofFileRequest* mes, getRofFileResponse* response) {
    int fileid = mes->ind();
    int res = server_->getRofFile(fileid);
    response->set_r(res);
    return grpc::Status::OK;
}
