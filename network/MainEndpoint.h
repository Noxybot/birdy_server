#pragma once
#include "../interfaces/Database.h"

#include "../grpc/birdy.grpc.pb.h"

class MainEndpoint : public birdy_grpc::MainEndpoint::Service
{
    std::shared_ptr<Database> m_db;
public:
    MainEndpoint(std::shared_ptr<Database> db);

    ::grpc::Status RegisterUser(::grpc::ServerContext* context, const ::birdy_grpc::RegistrationRequest* request, ::birdy_grpc::RegistrationResponse* response) override;
    ::grpc::Status LoginUser(::grpc::ServerContext* context, const ::birdy_grpc::LoginRequest* request, ::birdy_grpc::LoginResponse* response) override;
    ::grpc::Status FindBird(::grpc::ServerContext* context, const ::birdy_grpc::FindBirdRequest* request, ::birdy_grpc::FindBirdResponse* response) override;
};
