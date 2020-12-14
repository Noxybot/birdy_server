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
    *response = m_db->LoginUser(*request);
    return grpc::Status::OK;
}

::grpc::Status MainEndpoint::FindBirdByName(::grpc::ServerContext* context, const ::birdy_grpc::FindBirdByNameRequest* request,
    ::grpc::ServerWriter<::birdy_grpc::FindBirdByNameResponse>* writer)
{
    const auto birds = m_db->FindBirdByName(*request);
    for (const auto& bird : birds)
        writer->Write(bird);
    return grpc::Status::OK;
}

::grpc::Status MainEndpoint::AddBirdWithData(::grpc::ServerContext* context,
    const ::birdy_grpc::AddBirdWithDataRequest* request, ::birdy_grpc::AddBirdWithDataResponse* response)
{
    const auto res = m_db->AddBirdWithData(*request);
    *response = std::move(res);
    return grpc::Status::OK;
}

::grpc::Status MainEndpoint::FindBirdCoordinatesByName(::grpc::ServerContext* context,
    const ::birdy_grpc::FindBirdCoordinatesByNameRequest* request,
    ::grpc::ServerWriter<::birdy_grpc::FindBirdCoordinatesByNameResponse>* writer)
{
    const auto birds = m_db->FindBirdCoordinates(*request);
    for (const auto& bird : birds)
        writer->Write(bird);
    return grpc::Status::OK;
}

::grpc::Status MainEndpoint::BindBoysByCity(::grpc::ServerContext* context,
    const ::birdy_grpc::FindBoysByCityRequest* request, ::grpc::ServerWriter<::birdy_grpc::UserInfo>* writer)
{
    const auto boys = m_db->FindBoysByCity(*request);
    for (const auto& boy : boys)
        writer->Write(boy);
    return grpc::Status::OK;
}

::grpc::Status MainEndpoint::UpdateUser(::grpc::ServerContext* context, const ::birdy_grpc::UserInfo* request,
    ::birdy_grpc::Empty* response)
{
    m_db->UpdateUser(*request);
    return grpc::Status::OK;
}

::grpc::Status MainEndpoint::GetTopBirds(::grpc::ServerContext* context, const ::birdy_grpc::GetTopBirdsRequest* request,
    ::grpc::ServerWriter<::birdy_grpc::EncyclopedicBirdInfo>* writer)
{
    m_db->GetTopBirds(*request, writer);
    return grpc::Status::OK;
}

