#include "MainEndpoint.h"

MainEndpoint::MainEndpoint(std::shared_ptr<Database> db)
    : m_db(std::move(db))
{}

::grpc::Status MainEndpoint::RegisterUser(::grpc::ServerContext* context,
    const ::birdy_grpc::RegistrationRequest* request, ::birdy_grpc::RegistrationResponse* response)
{
    const auto result = m_db->RegisterUser(*request);
    response->set_result(result);
    switch (result) {
        case birdy_grpc::RegistrationResponse_Result_OK:
        case birdy_grpc::RegistrationResponse_Result_EMAIL_ALREADY_TAKEN:
            return grpc::Status::OK;
    default:
        assert(false);
        return grpc::Status::CANCELLED;
    }
}

::grpc::Status MainEndpoint::LoginUser(::grpc::ServerContext* context, const ::birdy_grpc::LoginRequest* request,
    ::birdy_grpc::LoginResponse* response)
{
    const auto result = m_db->LoginUser(*request);
    response->set_result(result);
    switch (result) {
        case birdy_grpc::LoginResponse_Result_OK:
        case birdy_grpc::LoginResponse_Result_WRONG_PASSWORD:
        case birdy_grpc::LoginResponse_Result_LOGIN_NOT_FOUND:
            return grpc::Status::OK;
        default:
            assert(false);
            return grpc::Status::CANCELLED;
    }
}


::grpc::Status MainEndpoint::FindBird(::grpc::ServerContext* context, const ::birdy_grpc::FindBirdRequest* request,
    ::birdy_grpc::FindBirdResponse* response)
{
    return {};
}
