
#include "network/MainEndpoint.h"
#include "database/DatabasePQxx.h"
#include "database/ConnectionPool.h"
#include "utils/catch_all.h"
#include <fstream>

#pragma warning(push, 0)
#include <grpc++/server_builder.h>
#include "grpc/dima.grpc.pb.h"
#include <grpc++/create_channel.h>
#pragma warning(pop)


void test()
{
     auto test = birdy_grpc::ModelEndpoint::NewStub(grpc::CreateChannel("109.87.116.179:1488", grpc::InsecureChannelCredentials()));
    birdy_grpc::RecognizeBirdRequest req;
    birdy_grpc::RecognizeBirdResponse response;
    grpc::ClientContext ctx;
    ::grpc::Status status;
    std::string photo_name, sound_name;
 /*   if (sound.empty())
    {
        req.set_data(photo);
        status = test->RecognizeBirdByPhoto(&ctx, req, &response);
        photo_name = m_image_mgr->SaveFile(photo);
    }
    else*/
    {
        std::ifstream test1 {"sound.mp3"};
        if (!test1.is_open())
        {
            std::cout << "bad";
        }
        req.set_data(std::string {std::istream_iterator<char>{test1}, {}});
        status = test->RecognizeBirdBySound(&ctx, req, &response);
    }
    if (status.ok())
    {
        std::cout << "bird name= "<<  response.name() << std::endl;
    }
}

int main(int argc, char** argv) try
{
    SetConsoleOutputCP(CP_UTF8);
    std::string server_address("0.0.0.0:1488");
    auto db = std::make_shared<DatabasePQxx>(std::make_shared<ConnectionPool>("postgresql://postgres:postgres125@109.87.116.179:5555?application_name=birdy_server", 30));
    //db->dummy();
    MainEndpoint service(db);
    
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.SetMaxMessageSize(std::numeric_limits<int>::max());
    builder.RegisterService(&service);
    //builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 10000);
    builder.AddChannelArgument(GRPC_ARG_MAX_RECEIVE_MESSAGE_LENGTH, -1);
    builder.AddChannelArgument(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, -1);
    //builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 10000);
    //builder.AddChannelArgument(GRPC_ARG_HTTP2_BDP_PROBE, 1);
    //builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
    //builder.AddChannelArgument(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 5000);
    //builder.AddChannelArgument(GRPC_ARG_HTTP2_MIN_SENT_PING_INTERVAL_WITHOUT_DATA_MS, 10000);
    //std::unique_ptr<grpc_impl::ServerCompletionQueue> cq_ = builder.AddCompletionQueue();
    //grpc::experimental::ServerBidiReactor<
    // birdy_grpc::MainEndpoint::AsyncService tmp;
    //  grpc::ServerContext ctx;
    //  const auto data = ctx.client_metadata();
    std::unique_ptr<grpc_impl::Server> server(builder.BuildAndStart());
    //server->SetGlobalCallbacks()
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
    return 0;
}
CATCH_ALL(-1)