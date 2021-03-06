#pragma once
#include "../interfaces/Database.h"

#include <pqxx/pqxx>
#include "../FileManager.h"

class ConnectionPool;

class DatabasePQxx : public Database
{
    std::shared_ptr<ConnectionPool> m_pool;
    std::unique_ptr<FileManager> m_image_mgr;
public:
    DatabasePQxx(std::shared_ptr<ConnectionPool> pool);
    void dummy();
    birdy_grpc::RegistrationResponse::Result RegisterUser(const birdy_grpc::RegistrationRequest& request) override;
    birdy_grpc::LoginResponse LoginUser(const birdy_grpc::LoginRequest& request) override;
    std::vector<birdy_grpc::FindBirdByNameResponse> FindBirdByName(const birdy_grpc::FindBirdByNameRequest& request) override;
    birdy_grpc::AddBirdWithDataResponse AddBirdWithData(const birdy_grpc::AddBirdWithDataRequest& request) override;
    std::vector<birdy_grpc::FindBirdCoordinatesByNameResponse> FindBirdCoordinates(const birdy_grpc::FindBirdCoordinatesByNameRequest& request) override;
    std::vector<birdy_grpc::UserInfo> FindBoysByCity(const birdy_grpc::FindBoysByCityRequest& req) override;
    void UpdateUser(const birdy_grpc::UserInfo& info) override;
    void GetTopBirds(const ::birdy_grpc::GetTopBirdsRequest& req, grpc::ServerWriter<::birdy_grpc::EncyclopedicBirdInfo>* writer) override;
};