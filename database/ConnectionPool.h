#pragma once
#include <pqxx/pqxx>

#include <memory>
#include <set>
#include <mutex>

class ConnectionPool
{
    using conn_ptr = std::shared_ptr<pqxx::connection>;
    std::mutex m_mtx;
    std::set<conn_ptr> m_free_connections;
    std::condition_variable m_condition;

public:
    ConnectionPool(const std::string& connection_string, std::size_t num_connections);
    conn_ptr AcquireConnection(std::chrono::steady_clock::duration timeout = std::chrono::seconds(5));
    void ReleaseConnection(conn_ptr connection);
};
