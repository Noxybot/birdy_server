#include "DatabasePQxx.h"
#include "../utils/catch_all.h"
#include "ConnectionPool.h"

#include <fmt/format.h>
#include "../utils/scope_exit.h"


DatabasePQxx::DatabasePQxx(std::shared_ptr<ConnectionPool> pool)
    : m_pool(std::move(pool))
    , m_image_mgr(std::make_unique<FileManager>())
{}

void DatabasePQxx::dummy() try
{
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
      if (!conn)
    {
        std::cout << "could not acquire connection\n";
       return;
    }
    pqxx::work work(*conn);
    const static auto query = fmt::format("SELECT bird_name, photo, description FROM avpz.find_bird_by_name({bird}) AS (bird_name TEXT, photo BYTEA, description TEXT);",
        fmt::arg("bird", work.quote(u8"Вор")));
    
    const auto res = work.exec(query);
    for (const auto& row : res)
        std::cout << row["bird_name"].as<std::string>() << std::endl;
}
CATCH_ALL(;)

birdy_grpc::RegistrationResponse::Result DatabasePQxx::RegisterUser(const birdy_grpc::RegistrationRequest& request) try
{
    std::cout << request.Utf8DebugString();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
      if (!conn)
    {
        std::cout << "could not acquire connection\n";
       return {};
    }
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
    const auto res = work.exec1(query);
    work.commit();
    std::cout << "query=" << query << std::endl;
    if (res[0].as<bool>())
        return birdy_grpc::RegistrationResponse_Result_OK;
    return birdy_grpc::RegistrationResponse_Result_EMAIL_ALREADY_TAKEN;
}
CATCH_ALL(birdy_grpc::RegistrationResponse_Result_DB_ERROR)

birdy_grpc::LoginResponse::Result DatabasePQxx::LoginUser(const birdy_grpc::LoginRequest& request) try
{
    std::cout << request.Utf8DebugString();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
      if (!conn)
    {
        std::cout << "could not acquire connection\n";
       return {};
    }
    pqxx::work work(*conn);
    const auto query = fmt::format("SELECT avpz.login_user({email}, {password});",
           fmt::arg("email", work.quote(request.email())),
           fmt::arg("password", work.quote(request.password())));
    const auto res = work.exec1(query);
    work.commit();
    std::cout << "query=" << query << std::endl;
    const auto login_res =  res[0].as<int>();
    return static_cast<birdy_grpc::LoginResponse_Result>(login_res);
}
CATCH_ALL(birdy_grpc::LoginResponse_Result_DB_ERROR)

std::vector<birdy_grpc::FindBirdByNameResponse> DatabasePQxx::FindBirdByName(const birdy_grpc::FindBirdByNameRequest& request) try
{
    std::cout << request.Utf8DebugString();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
      if (!conn)
    {
        std::cout << "could not acquire connection\n";
       return {};
    }
    pqxx::work work(*conn);
    std::vector<birdy_grpc::FindBirdByNameResponse> result;
    if (!request.name().empty())
    {
        const auto query = fmt::format("SELECT bird_name, photo_name, description FROM avpz.find_bird_by_name({bird}) AS (bird_name TEXT, photo_name TEXT, description TEXT);",
        fmt::arg("bird", work.quote(request.name())));
        const auto res = work.exec(query);
        work.commit();
        std::cout << "query=" << query << std::endl;
        for (const auto& row : res)
        {
            birdy_grpc::FindBirdByNameResponse response;
            response.set_res(birdy_grpc::FindBirdByNameResponse_Result_FOUND);
            auto enc_info = new birdy_grpc::EncyclopedicBirdInfo;
            if (!row["bird_name"].is_null())
                enc_info->set_name(row["bird_name"].as<std::string>());
            if (!row["photo_name"].is_null())
            {
                auto photo = m_image_mgr->GetFile(row["photo_name"].as<std::string>());
                enc_info->set_photo(std::move(photo));
            }
               
            if (!row["description"].is_null())
            {
                auto test = row["description"].as<std::string>();
                enc_info->set_description(test);
            }
           // auto bird_info = new birdy_grpc::BirdInfo;
            //bird_info->set_allocated_enc_info(enc_info);
           // response.mutable_info()->AddAllocated(bird_info);
            response.set_allocated_enc_info(enc_info);
            result.emplace_back(std::move(response));
        }
        return result;
    }
    else
    {
         birdy_grpc::FindBirdByNameResponse response;
         response.set_res(birdy_grpc::FindBirdByNameResponse_Result_NOT_FOUND);
         return {response};
    }
    
    return {};
}
CATCH_ALL({})

birdy_grpc::AddBirdWithDataResponse DatabasePQxx::AddBirdWithData(const birdy_grpc::AddBirdWithDataRequest& request) try
{
    //std::cout << request.Utf8DebugString();
    const auto& info = request.info();
    const auto& found_point = info.found_point();
    const auto& found_time = info.found_time();
    if (!request.has_info() ||
        !info.has_found_point()||
        !found_point.latitude() ||
        !found_point.longitude() ||
        found_time.empty())
    {
        std::cout << "not enough info\n";
        return {};
    }
    const auto& photo = request.photo();
    const auto& sound = request.sound();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
    if (!conn)
    {
        std::cout << "could not acquire connection\n";
       return {};
    }
    const auto photo_name = m_image_mgr->SaveFile(photo);
    pqxx::work work(*conn);
    //const auto test = work.quote_raw(photo);
    std::string query = fmt::format("CALL avpz.add_found_bird({longitude}, {latitude}, {time}, {email}, {photo_name}, {sound_name}, {bird_name})",
             fmt::arg("longitude", work.quote(std::to_string(found_point.longitude()))),
             fmt::arg("latitude", work.quote(std::to_string(found_point.latitude()))),
             fmt::arg("time",  work.quote(found_time)),
             fmt::arg("email", work.quote(info.finder_email())),
             fmt::arg("photo_name", work.quote(photo_name)),
             fmt::arg("sound_name", work.quote("")),
             fmt::arg("bird_name", work.quote(u8"Воронкин")));
    work.exec0(query);
    work.commit();
    std::cout << "query=" << query << std::endl;
    birdy_grpc::AddBirdWithDataResponse res;
    res.set_bird_name(u8"Воронкин");
    return res;
}
CATCH_ALL({})
