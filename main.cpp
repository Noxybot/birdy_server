
#include "network/MainEndpoint.h"
#include "database/DatabasePQxx.h"
#include "database/ConnectionPool.h"
#include "utils/catch_all.h"

#pragma warning(push, 0)
#include <grpc++/server_builder.h>
#pragma warning(pop)

int main(int argc, char** argv) try
{
  SetConsoleOutputCP(CP_UTF8);
  std::string server_address("0.0.0.0:1488");
  auto db = std::make_shared<DatabasePQxx>(std::make_shared<ConnectionPool>("postgresql://postgres:postgres125@109.87.116.179:5555?application_name=birdy_server", 3));
  db->dummy();
  MainEndpoint service(db);

  grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.SetMaxMessageSize(std::numeric_limits<int>::max());
  builder.RegisterService(&service);
  //builder.AddChannelArgument(GRPC_ARG_KEEPALIVE_TIME_MS, 10000);
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