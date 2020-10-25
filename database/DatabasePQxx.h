#pragma once
#include "../interfaces/Database.h"

#include <pqxx/pqxx>

class ConnectionPool;

class DatabasePQxx : public Database
{
    std::shared_ptr<ConnectionPool> m_pool;
public:
    DatabasePQxx(std::shared_ptr<ConnectionPool> pool);
    void dummy();
    birdy_grpc::RegistrationResponse::Result RegisterUser(const birdy_grpc::RegistrationRequest& request) override;
    birdy_grpc::LoginResponse::Result LoginUser(const birdy_grpc::LoginRequest& request) override;
    std::vector<birdy_grpc::FindBirdByNameResponse> FindBirdByName(const birdy_grpc::FindBirdByNameRequest& request) override;
    birdy_grpc::AddBirdWithDataResponse AddBirdWithData(const birdy_grpc::AddBirdWithDataRequest& request) override;
};