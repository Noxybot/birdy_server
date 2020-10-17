#pragma once
#include "../interfaces/Database.h"

#include <pqxx/pqxx>

class ConnectionPool;

class DatabasePQxx : public Database
{
    std::shared_ptr<ConnectionPool> m_pool;
public:
    DatabasePQxx(std::shared_ptr<ConnectionPool> pool);
    bool dummy();
    birdy_grpc::RegistrationResponse::Result RegisterUser(const birdy_grpc::RegistrationRequest& request) override;
    birdy_grpc::LoginResponse::Result LoginUser(const birdy_grpc::LoginRequest& request) override;
    std::vector<birdy_grpc::FindBirdResponse> FindBird(const birdy_grpc::FindBirdRequest& request) override;
private:
    pqxx::result ExecuteQuery(const std::string& query, std::chrono::steady_clock::duration timeout = std::chrono::seconds(3));

};