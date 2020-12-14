#pragma once
#pragma warning(push, 0)
#include "../grpc/birdy.pb.h"
#include <grpcpp/impl/codegen/sync_stream.h>
#pragma warning(pop)
class Database
{
public:
    virtual ~Database() = default;
    virtual birdy_grpc::RegistrationResponse::Result RegisterUser(const birdy_grpc::RegistrationRequest& request) = 0;
    virtual birdy_grpc::LoginResponse LoginUser(const birdy_grpc::LoginRequest& request) = 0;
    virtual std::vector<birdy_grpc::FindBirdByNameResponse> FindBirdByName(const birdy_grpc::FindBirdByNameRequest& request) = 0;
    virtual birdy_grpc::AddBirdWithDataResponse AddBirdWithData(const birdy_grpc::AddBirdWithDataRequest& request) = 0;
    virtual std::vector<birdy_grpc::FindBirdCoordinatesByNameResponse> FindBirdCoordinates(const birdy_grpc::FindBirdCoordinatesByNameRequest& request) = 0;
    virtual std::vector<birdy_grpc::UserInfo> FindBoysByCity(const birdy_grpc::FindBoysByCityRequest& req) = 0;
    virtual void UpdateUser(const birdy_grpc::UserInfo& info) = 0;
    virtual void GetTopBirds(const ::birdy_grpc::GetTopBirdsRequest& req, grpc::ServerWriter<::birdy_grpc::EncyclopedicBirdInfo>* writer) = 0;
};
