#pragma once
#include "../grpc/birdy.pb.h"

class Database
{
public:
    virtual ~Database() = default;
    virtual birdy_grpc::RegistrationResponse::Result RegisterUser(const birdy_grpc::RegistrationRequest& request) = 0;
    virtual birdy_grpc::LoginResponse::Result LoginUser(const birdy_grpc::LoginRequest& request) = 0;
    virtual std::vector<birdy_grpc::FindBirdResponse> FindBird(const birdy_grpc::FindBirdRequest& request) = 0;
};
