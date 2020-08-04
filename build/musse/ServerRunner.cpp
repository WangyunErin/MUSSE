#include "ServerRunner.h"

MusesServerRunner::MusesServerRunner() {
}

MusesServerRunner::~MusesServerRunner() {
}

grpc::Status MusesServerRunner::setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e) {
    server_ = make_unique<Server>(request->maxqueuesize());
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::createOMAP(grpc::ServerContext* context, const CreateOMAPMessage* request, google::protobuf::Empty* e) {
    server_->createRamStore(request->num(), request->userid());
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::update(grpc::ServerContext* context, const UpdateMessage* mes, UpdateResponse* response) {
    if (!server_) {
        //  the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
    prf_type addr, val;
    std::copy(mes->address().begin(), mes->address().end(), addr.begin());
    std::copy(mes->value().begin(), mes->value().end(), val.begin());

    server_->update(addr, val);

    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::batchUpdate(grpc::ServerContext* context, const BatchUpdateMessage* mes, UpdateResponse* response) {
    if (!server_) {
        // the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }

    for (int i = 0; i < mes->address_size(); i++) {
        prf_type addr, value;
        copy(mes->address(i).begin(), mes->address(i).end(), addr.begin());
        copy(mes->value(i).begin(), mes->value(i).end(), value.begin());
        server_->update(addr, value);
    }

    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::search(grpc::ServerContext* context, const SearchMessage* message, SearchResponse* response) {
    if (!server_) {
        // the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
    vector<prf_type> addresses;
    for (unsigned int i = 0; i < message->address_size(); i++) {
        prf_type item;
        copy(message->address(i).begin(), message->address(i).end(), item.begin());
        addresses.push_back(item);
    }
    vector<prf_type> result = server_->search(addresses);

    for (unsigned int i = 0; i < result.size(); i++) {
        response->add_ciphertext(result[i].data(), result[i].size());
    }
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::search1(grpc::ServerContext* context, const SearchMessage1* message, SearchResponse1* response) {
    if (!server_) {
        // the server is already set up
        return grpc::Status(grpc::FAILED_PRECONDITION, "The server is not set up");
    }
   vector<prf_type> addresses;
    for (unsigned int i = 0; i < message->address_size(); i++) {
        prf_type item;
        copy(message->address(i).begin(), message->address(i).end(), item.begin());
        addresses.push_back(item);
    }

    for(unsigned int i = 0; i < addresses.size(); i++){
        prf_type val =server_->search(addresses[i]);
        prf_type notfound;
        memset(notfound.data(), 0, AES_KEY_SIZE);
        if(val==notfound){
            break;
        }
        response->add_ciphertext(val.data(), val.size());
    }

    return grpc::Status::OK;
}

// grpc::Status MusesServerRunner::getMerkleRoot(grpc::ServerContext* context, const google::protobuf::Empty* mes, GetMerkleRootMessage* res) {
//     res->set_valuesroot((unsigned char*) server_->valuesMerkleRoot, SHA256_DIGEST_LENGTH);
//     res->set_queuesroot((unsigned char*) server_->queuesMerkleRoot, SHA256_DIGEST_LENGTH);
//     return grpc::Status::OK;
// }

// grpc::Status MusesServerRunner::updateMerkleRoot(grpc::ServerContext* context, const UpdateMerkleRootMessage* mes, google::protobuf::Empty* e) {
//     memcpy(server_->valuesMerkleRoot, (unsigned char*) mes->valuesroot().data(), SHA256_DIGEST_LENGTH);
//     memcpy(server_->queuesMerkleRoot, (unsigned char*) mes->queuesroot().data(), SHA256_DIGEST_LENGTH);
//     return grpc::Status::OK;
// }

// grpc::Status MusesServerRunner::addToFileCntQueue(grpc::ServerContext* context, const AddFileCntQueueMessage* mes, google::protobuf::Empty* e) {
//     int userID = mes->userid();
//     prf_type val;
//     memcpy((unsigned char*) val.data(), (unsigned char*) mes->value().data(), AES_KEY_SIZE);
//     server_->addToFileCntQueue(userID, val);
//     return grpc::Status::OK;
// }

// grpc::Status MusesServerRunner::batchAddToFileCntQueue(grpc::ServerContext* context, const BatchAddFileCntQueueMessage* mes, google::protobuf::Empty* e) {
//     int userID = mes->userid();
//     for (int i = 0; i < mes->values_size(); i++) {
//         prf_type val;
//         memcpy((unsigned char*) val.data(), (unsigned char*) mes->values(i).data(), AES_KEY_SIZE);
//         server_->addToFileCntQueue(userID, val);
//     }
//     return grpc::Status::OK;
// }

// grpc::Status MusesServerRunner::getFileCntDiff(grpc::ServerContext* context, const FileCntQueueRequestMessage* mes, FileCntDiffMessage* response) {
//     ResultWithProof diffs = server_->getQueueValues(mes->userid());
//     for (unsigned int i = 0; i < diffs.values.size(); i++) {
//         response->add_value((unsigned char*) diffs.values[i].data(), AES_KEY_SIZE);
//     }
//     return grpc::Status::OK;
// }

grpc::Status MusesServerRunner::readStore(grpc::ServerContext* context, const ReadStoreRequest* request, SearchResponse* response) {
    vector<int> poses;
    for (int i = 0; i < request->poses_size(); i++) {
        poses.push_back(request->poses(i));
    }
    BlocksWithProof result = server_->readStore(poses, request->userid());
    for (unsigned int i = 0; i < result.values.size(); i++) {
        response->add_ciphertext((unsigned char*) result.values[i].data(), result.values[i].size());
        response->add_cipherpositions(result.valuesPoses[i]);
        // response->add_ciphersize(result.values[i].size());
    }
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::writeInStore(grpc::ServerContext* context, const WriteStoreRequest* request, SearchResponse* response) {
    vector<block> blocks;
    vector<int> indexes;
    for (int i = 0; i < request->pos_size(); i++) {
        block b(request->size(i));
        memcpy(b.data(), (unsigned char*) request->value(i).data(), request->size(i));
        blocks.push_back(b);
        indexes.push_back(request->pos(i));
    }
    BlocksWithProof result = server_->writeInStore(indexes, blocks, request->userid());
    for (unsigned int i = 0; i < result.values.size(); i++) {
        response->add_ciphertext(result.values[i].data(), result.values[i].size());
        response->add_cipherpositions(result.valuesPoses[i]);
    }
    // for (auto item : result.proofs) {
    //     response->add_hash(item.second, SHA256_DIGEST_LENGTH);
    //     response->add_proofposistions(item.first);
    // }
    // response->set_treesize(result.treeSize);
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::downloadStash(grpc::ServerContext* context, const DownloadStashMessage* mes, DownloadStashResponse* response) {
    int userID = mes->userid();
    vector< pair<int, unsigned char*> > stash = server_->downloadStash(userID);
    for (unsigned int i = 0; i < stash.size(); i++) {
        response->add_value((unsigned char*) stash[i].second, stash[i].first);
        response->add_pos(stash[i].first);
    }
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::uploadStash(grpc::ServerContext* context, const UploadStashMessage* mes, google::protobuf::Empty* e) {
    vector< pair<int, unsigned char*> > stash;
    for (int i = 0; i < mes->pos_size(); i++) {
        unsigned char* cipher = new unsigned char[mes->pos(i)];
        memcpy(cipher, (unsigned char*) mes->value(i).data(), mes->pos(i));
        stash.push_back(pair<int, unsigned char*> (mes->pos(i), cipher));
    }
    server_->uploadStash(stash, mes->userid());
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::downloadOMAPRoot(grpc::ServerContext* context, const DownloadRootMessage* mes, DownloadRootResponse* response) {
    auto rt = server_->getOMAPRoot(mes->userid());
    response->set_pos(rt.second);
    response->set_value((unsigned char*) rt.first.id.data(), rt.first.id.size());
    return grpc::Status::OK;
}

grpc::Status MusesServerRunner::uploadOMAPRoot(grpc::ServerContext* context, const UploadRootMessage* mes, google::protobuf::Empty* e) {
    Bid bid;
    memcpy(bid.id.data(), (unsigned char*) mes->value().data(), ID_SIZE);
    server_->setOMAPRoots(bid, mes->pos(), mes->userid());
    return grpc::Status::OK;
}

// grpc::Status MusesServerRunner::downloadOMAPMerkleRoot(grpc::ServerContext* context, const DownloadRootMessage* mes, DownloadRootResponse* response) {
//     response->set_value((unsigned char*) server_->omapsMerkleRoot[mes->userid()], SHA256_DIGEST_LENGTH);
//     return grpc::Status::OK;
// }

// grpc::Status MusesServerRunner::uploadOMAPMerkleRoot(grpc::ServerContext* context, const UploadRootMessage* mes, google::protobuf::Empty* e) {
//     memcpy(server_->omapsMerkleRoot[mes->userid()], (unsigned char*) mes->value().data(), SHA256_DIGEST_LENGTH);
//     return grpc::Status::OK;
// }

// grpc::Status MusesServerRunner::getFileCntDiffWithLimit(grpc::ServerContext* context, const FileCntQueueRequestMessage* mes, FileCntDiffMessage* response) {
    //ResultWithProof diffs = server_->getQueueValues(mes->userid(), mes->limit());
    // for (unsigned int i = 0; i < diffs.values.size(); i++) {
    //     response->add_value((unsigned char*) diffs.values[i].data(), AES_KEY_SIZE);
    // }
    // return grpc::Status::OK;
// }
