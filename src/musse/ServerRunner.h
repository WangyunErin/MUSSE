
#ifndef MUSSEERVERRUNNER_H
#define MUSSEERVERRUNNER_H

#include "Server.h"
#include "musse.grpc.pb.h"
#include <string>
#include <memory>
#include <mutex>
#include <grpc++/server.h>
#include <grpc++/server_context.h>

class MusesServerRunner : public Musse::Service {
private:
    std::unique_ptr<Server> server_;

public:
    MusesServerRunner();
    virtual ~MusesServerRunner();
    grpc::Status setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e);
    grpc::Status createOMAP(grpc::ServerContext* context, const CreateOMAPMessage* request, google::protobuf::Empty* e);
    grpc::Status readStore(grpc::ServerContext* context, const ReadStoreRequest* request, SearchResponse* response);
    grpc::Status writeInStore(grpc::ServerContext* context, const WriteStoreRequest* request, SearchResponse* e);
    grpc::Status update(grpc::ServerContext* context, const UpdateMessage* request, UpdateResponse* response);
    grpc::Status batchUpdate(grpc::ServerContext* context, const BatchUpdateMessage* request, UpdateResponse* response);
    grpc::Status search(grpc::ServerContext* context, const SearchMessage* mes, SearchResponse* res);
    grpc::Status search1(grpc::ServerContext* context, const SearchMessage1* mes, SearchResponse1* res);
    grpc::Status updateMerkleRoot(grpc::ServerContext* context, const UpdateMerkleRootMessage* mes, google::protobuf::Empty* e);
    grpc::Status getMerkleRoot(grpc::ServerContext* context, const google::protobuf::Empty* mes, GetMerkleRootMessage* res);
    grpc::Status addToFileCntQueue(grpc::ServerContext* context, const AddFileCntQueueMessage* mes, google::protobuf::Empty* e);
    grpc::Status batchAddToFileCntQueue(grpc::ServerContext* context, const BatchAddFileCntQueueMessage* mes, google::protobuf::Empty* e);
    grpc::Status getFileCntDiff(grpc::ServerContext* context, const FileCntQueueRequestMessage* mes, FileCntDiffMessage* response);
    grpc::Status getFileCntDiffWithLimit(grpc::ServerContext* context, const FileCntQueueRequestMessage* mes, FileCntDiffMessage* response);
    grpc::Status uploadStash(grpc::ServerContext* context, const UploadStashMessage* mes, google::protobuf::Empty* e);
    grpc::Status downloadStash(grpc::ServerContext* context, const DownloadStashMessage* mes, DownloadStashResponse* response);
    grpc::Status uploadOMAPRoot(grpc::ServerContext* context, const UploadRootMessage* mes, google::protobuf::Empty* e);
    grpc::Status downloadOMAPRoot(grpc::ServerContext* context, const DownloadRootMessage* mes, DownloadRootResponse* response);
    grpc::Status uploadOMAPMerkleRoot(grpc::ServerContext* context, const UploadRootMessage* mes, google::protobuf::Empty* e);
    grpc::Status downloadOMAPMerkleRoot(grpc::ServerContext* context, const DownloadRootMessage* mes, DownloadRootResponse* response);
};

#endif /* MusseERVERRUNNER_H */

