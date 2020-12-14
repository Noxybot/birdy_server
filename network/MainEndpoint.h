#pragma once
#include "../interfaces/Database.h"

#pragma warning(push, 0)
#include "../grpc/birdy.grpc.pb.h"

#pragma warning(pop)
class MainEndpoint : public birdy_grpc::MainEndpoint::Service
{
    std::shared_ptr<Database> m_db;
public:
    MainEndpoint(std::shared_ptr<Database> db);

    ::grpc::Status RegisterUser(::grpc::ServerContext* context, const ::birdy_grpc::RegistrationRequest* request, ::birdy_grpc::RegistrationResponse* response) override;
    ::grpc::Status LoginUser(::grpc::ServerContext* context, const ::birdy_grpc::LoginRequest* request, ::birdy_grpc::LoginResponse* response) override;
    ::grpc::Status FindBirdByName(::grpc::ServerContext* context, const ::birdy_grpc::FindBirdByNameRequest* request, ::grpc::ServerWriter<::birdy_grpc::FindBirdByNameResponse>* writer) override;
    ::grpc::Status AddBirdWithData(::grpc::ServerContext* context, const ::birdy_grpc::AddBirdWithDataRequest* request, ::birdy_grpc::AddBirdWithDataResponse* response) override;
    ::grpc::Status FindBirdCoordinatesByName(::grpc::ServerContext* context, const ::birdy_grpc::FindBirdCoordinatesByNameRequest* request, ::grpc::ServerWriter<::birdy_grpc::FindBirdCoordinatesByNameResponse>* writer) override;
    ::grpc::Status BindBoysByCity(::grpc::ServerContext* context, const ::birdy_grpc::FindBoysByCityRequest* request, ::grpc::ServerWriter<::birdy_grpc::UserInfo>* writer) override;
    ::grpc::Status UpdateUser(::grpc::ServerContext* context, const ::birdy_grpc::UserInfo* request, ::birdy_grpc::Empty* response) override;
    ::grpc::Status GetTopBirds(::grpc::ServerContext* context, const ::birdy_grpc::GetTopBirdsRequest* request, ::grpc::ServerWriter<::birdy_grpc::EncyclopedicBirdInfo>* writer) override;
};
