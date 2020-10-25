#pragma once
#pragma warning(push, 0)
#include "../grpc/birdy.pb.h"
#pragma warning(pop)
class Database
{
public:
    virtual ~Database() = default;
    virtual birdy_grpc::RegistrationResponse::Result RegisterUser(const birdy_grpc::RegistrationRequest& request) = 0;
    virtual birdy_grpc::LoginResponse::Result LoginUser(const birdy_grpc::LoginRequest& request) = 0;
    virtual std::vector<birdy_grpc::FindBirdByNameResponse> FindBirdByName(const birdy_grpc::FindBirdByNameRequest& request) = 0;
    virtual birdy_grpc::AddBirdWithDataResponse AddBirdWithData(const birdy_grpc::AddBirdWithDataRequest& request) = 0;
};
