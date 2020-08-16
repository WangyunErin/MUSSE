
#ifndef MKSESERVERRUNNER_H
#define MKSESERVERRUNNER_H

#include "MKSEServer.h"
#include "mkse.grpc.pb.h"
#include <string>
#include <memory>
#include <mutex>
#include <grpc++/server.h>
#include <grpc++/server_context.h>

class MKSEServerRunner : public MKSE::Service {
private:
    std::unique_ptr<MKSEServer> server_;

public:
    MKSEServerRunner();
    virtual ~MKSEServerRunner();
    // grpc::Status setup(grpc::ServerContext* context, const SetupMessage* request, google::protobuf::Empty* e);
    grpc::Status update(grpc::ServerContext* context, const UpdateMessage* request, google::protobuf::Empty* e);
    grpc::Status batchUpdate(grpc::ServerContext* context, const BatchUpdateMessage* request, google::protobuf::Empty* e);
    grpc::Status search(grpc::ServerContext* context, const SearchMessage* mes, SearchResponse* res);
    
    grpc::Status getRofFile(grpc::ServerContext* context, const getRofFileRequest* mes, getRofFileResponse* response);
};

#endif /* MKSESERVERRUNNER_H */

