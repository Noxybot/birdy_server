#include "DatabasePQxx.h"
#include "../utils/catch_all.h"
#include "ConnectionPool.h"

#include <fmt/format.h>
#include "../utils/scope_exit.h"
#include "../grpc/dima.grpc.pb.h"
#include <grpcpp/create_channel.h>

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

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

birdy_grpc::LoginResponse DatabasePQxx::LoginUser(const birdy_grpc::LoginRequest& request) try
{
    std::cout << request.Utf8DebugString();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
    if (!conn)
    {
       std::cout << "could not acquire connection\n";
       return {};
    }
    birdy_grpc::LoginResponse res_;
    pqxx::work work(*conn);
    const auto query = fmt::format("SELECT * FROM avpz.login_user({email}, {password});",
           fmt::arg("email", work.quote(request.email())),
           fmt::arg("password", work.quote(request.password())));
    const auto res = work.exec1(query);
    work.commit();
    std::cout << "query=" << query << std::endl;
    auto info = new birdy_grpc::UserInfo;
    res_.set_allocated_info(info);
    res_.mutable_info()->set_email(res[0].as<std::string>(""));
    res_.mutable_info()->set_password(res[1].as<std::string>(""));
    res_.mutable_info()->set_first_name(res[2].as<std::string>(""));
    res_.mutable_info()->set_middle_name(res[3].as<std::string>(""));
    res_.mutable_info()->set_last_name(res[4].as<std::string>(""));
    res_.mutable_info()->set_birth_date(res[5].as<std::string>(""));
    res_.mutable_info()->set_city(res[6].as<std::string>(""));

    return res_;
}
CATCH_ALL({})

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
#include <fstream>
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
    //recognition
    auto test = birdy_grpc::ModelEndpoint::NewStub(grpc::CreateChannel("109.87.116.179:1488", grpc::InsecureChannelCredentials()));
    birdy_grpc::RecognizeBirdRequest req;
    birdy_grpc::RecognizeBirdResponse response;
    grpc::ClientContext ctx;
    ::grpc::Status status;
    std::string photo_name, sound_name;
    if (sound.empty())
    {
        req.set_data(photo);
        status = test->RecognizeBirdByPhoto(&ctx, req, &response);
        photo_name = m_image_mgr->SaveFile(photo);
    }
    else
    {
        req.set_data(photo);
        status = test->RecognizeBirdBySound(&ctx, req, &response);
        sound_name = m_image_mgr->SaveFile(sound);
    }
    if (status.ok())
    {
        std::cout << "bird name= "<<  response.name() << std::endl;
    }
    else
    {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        std::cout << "failed";
    }
    //end
    pqxx::work work(*conn);
    //const auto test = work.quote_raw(photo);
    std::string query = fmt::format("CALL avpz.add_found_bird({longitude}, {latitude}, {time}, {email}, {photo_name}, {sound_name}, {bird_name})",
             fmt::arg("longitude", work.quote(std::to_string(found_point.longitude()))),
             fmt::arg("latitude", work.quote(std::to_string(found_point.latitude()))),
             fmt::arg("time",  work.quote(found_time)),
             fmt::arg("email", work.quote(info.finder_email())),
             fmt::arg("photo_name", work.quote(photo_name)),
             fmt::arg("sound_name", work.quote(sound_name)),
             fmt::arg("bird_name", work.quote(response.name())));
    work.exec0(query);
    work.commit();
    std::cout << "query=" << query << std::endl;
    birdy_grpc::AddBirdWithDataResponse res;
    res.set_bird_name(response.name());
    return res;
}
CATCH_ALL({})

std::vector<birdy_grpc::FindBirdCoordinatesByNameResponse> DatabasePQxx::FindBirdCoordinates(
    const birdy_grpc::FindBirdCoordinatesByNameRequest& request) try
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
    std::vector<birdy_grpc::FindBirdCoordinatesByNameResponse> res;
    const auto name = request.name();
    assert(!name.empty(), "empty name!!!");
    const auto query = fmt::format("SELECT * FROM avpz.foundbirds WHERE bird_name = {name};",
        fmt::arg("name", work.quote(name)));
    std::cout << "query: " << query << std::endl;
    const auto res_ = work.exec(query);
    work.commit();
    for(const auto& row : res_)
    {
        birdy_grpc::FindBirdCoordinatesByNameResponse r;
        auto info = new birdy_grpc::UserBirdInfo;
        auto point = new birdy_grpc::UserBirdInfo::Point;
        info->set_found_time(row[2].as<std::string>(""));
        auto coords = split(row[1].as<std::string>(""), ",");
        point->set_longitude(std::stod(coords[0].substr(1, coords[0].size())));
        coords[1].pop_back();
        point->set_latitude(std::stod(coords[1]));
        info->set_allocated_found_point(point);
        r.set_allocated_info(info);
        res.push_back(std::move(r));
    }
    return std::move(res);
}
CATCH_ALL({})
std::vector<birdy_grpc::UserInfo> DatabasePQxx::FindBoysByCity(const birdy_grpc::FindBoysByCityRequest& req) try
{
    std::vector<birdy_grpc::UserInfo> res;
    std::cout << req.Utf8DebugString();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
    if (!conn)
    {
       std::cout << "could not acquire connection\n";
       return {};
    }
    pqxx::work work(*conn);
    const auto query = fmt::format("SELECT * FROM avpz.users WHERE city = {city};",
           fmt::arg("city", work.quote(req.city())));
        std::cout << "query=" << query << std::endl;
    const auto res_ = work.exec(query);
    work.commit();

    for (const auto& row : res_)
    {
        birdy_grpc::UserInfo info ;
    info.set_email(row[0].as<std::string>(""));
    info.set_password(row[1].as<std::string>(""));
    info.set_first_name(row[2].as<std::string>(""));
    info.set_middle_name(row[3].as<std::string>(""));
    info.set_last_name(row[4].as<std::string>(""));
    info.set_birth_date(row[5].as<std::string>(""));
    info.set_city(row[6].as<std::string>(""));
        res.push_back(std::move(info));
    }
    return res;

}
CATCH_ALL({})

void DatabasePQxx::UpdateUser(const birdy_grpc::UserInfo& info) try
{
    std::cout << info.Utf8DebugString();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
    if (!conn)
    {
       std::cout << "could not acquire connection\n";
       return;
    }
    pqxx::work work(*conn);
    const auto query_without_photo = fmt::format("UPDATE avpz.users\
                                    SET \
                                    user_password = {pass},\
                                    first_name = {f_name},\
                                    middle_name = {s_name},\
                                    last_name = {l_name},\
                                    birth_date = {b_date},\
                                    city = {city} \
                                    WHERE user_email = {email}\
                                    ",
           fmt::arg("email", work.quote(info.email())),
           fmt::arg("pass", work.quote(info.password())),
           fmt::arg("f_name", work.quote(info.first_name())),
           fmt::arg("s_name", work.quote(info.middle_name())),
           fmt::arg("l_name", work.quote(info.last_name())),
           fmt::arg("b_date", work.quote(info.birth_date())),
           fmt::arg("city", work.quote(info.city())));
    work.exec0(query_without_photo);
    work.commit();
}
CATCH_ALL(;)

void DatabasePQxx::GetTopBirds(const ::birdy_grpc::GetTopBirdsRequest& req, grpc::ServerWriter<::birdy_grpc::EncyclopedicBirdInfo>* writer)try
{
    std::cout << req.Utf8DebugString();
    auto conn = m_pool->AcquireConnection();
    SCOPE_EXIT {m_pool->ReleaseConnection(std::move(conn));};
    if (!conn)
    {
       std::cout << "could not acquire connection\n";
       return;
    }


}
CATCH_ALL()