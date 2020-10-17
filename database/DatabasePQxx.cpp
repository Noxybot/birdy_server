#include "DatabasePQxx.h"
#include "../utils/catch_all.h"
#include "ConnectionPool.h"

#include <fmt/format.h>
#include "../utils/scope_exit.h"


DatabasePQxx::DatabasePQxx(std::shared_ptr<ConnectionPool> pool)
    : m_pool(std::move(pool))
{}

bool DatabasePQxx::dummy() try
{
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
    if (!conn)
        return {};
    pqxx::work work(*conn);
    const static auto query = fmt::format("SELECT bird_name, photo, description FROM avpz.find_bird_by_name({bird}) AS (bird_name TEXT, photo BYTEA, description TEXT);",
        fmt::arg("bird", work.quote(u8"Вор")));
    
    const auto res = work.exec(query);
    for (const auto& row : res)
        std::cout << row["bird_name"].as<std::string>() << std::endl;
}
CATCH_ALL({})

birdy_grpc::RegistrationResponse::Result DatabasePQxx::RegisterUser(const birdy_grpc::RegistrationRequest& request) try
{
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
    if (!conn)
        return {};
    pqxx::work work(*conn);
    const auto query = fmt::format("SELECT avpz.registrer_user({email}, {password}, {first_name}, {middle_name}, {last_name}, {birth_date}, {city});",
           fmt::arg("email", work.quote(request.email())),
                 fmt::arg("password", work.quote(request.password())),
                 fmt::arg("first_name",  work.quote(request.first_name())),
                 fmt::arg("middle_name", work.quote(request.middle_name())),
                 fmt::arg("last_name", work.quote(request.last_name())),
                 fmt::arg("birth_date", work.quote(request.birth_date())),
                 fmt::arg("city", work.quote(request.city())));
    std::cout << "query : " << query << std::endl;
    const auto res  = work.exec1(query);
    work.commit();
    if (res[0].as<bool>())
        return birdy_grpc::RegistrationResponse_Result_OK;
    return birdy_grpc::RegistrationResponse_Result_EMAIL_ALREADY_TAKEN;
}
CATCH_ALL(birdy_grpc::RegistrationResponse_Result_DB_ERROR)

birdy_grpc::LoginResponse::Result DatabasePQxx::LoginUser(const birdy_grpc::LoginRequest& request) try
{
    return {};
}
CATCH_ALL(birdy_grpc::LoginResponse_Result_DB_ERROR)

std::vector<birdy_grpc::FindBirdResponse> DatabasePQxx::FindBird(const birdy_grpc::FindBirdRequest& request) try
{
    return {};
}
CATCH_ALL({})
