#include "ConnectionPool.h"
#include "../utils/catch_all.h"

ConnectionPool::ConnectionPool(const std::string& connection_string, std::size_t num_connections)
{
    try
    {
        for (auto i = 0u; i < num_connections; ++i)
            m_free_connections.emplace(std::make_shared<pqxx::connection>(connection_string));
    }
    CATCH_ALL(;)
}

ConnectionPool::conn_ptr ConnectionPool::AcquireConnection(std::chrono::steady_clock::duration timeout)
{
    const auto get_conn = [&] () -> conn_ptr
    {
        const auto conn_it = m_free_connections.begin();
        if (conn_it == std::end(m_free_connections))
        {
            std::cout <<"TOMARA INFROM EDIK IF YOU SEE THIS IN LOG\n";
            return {};
        }
        const auto res = *conn_it;
        m_free_connections.erase(conn_it);
        return res;
    };
    std::unique_lock<decltype(m_mtx)> lock(m_mtx);
    if (!m_free_connections.empty())
        return get_conn();

    if (m_condition.wait_for(lock, timeout, [this]
    {
        return !m_free_connections.empty();
    }))
    {
        return get_conn();
    }
     std::cout << "no connection for more than 5 seconds\n";
     return {};
}

void ConnectionPool::ReleaseConnection(conn_ptr connection)
{
    if (!connection)
        return;
    std::unique_lock<decltype(m_mtx)> lock(m_mtx);
    m_free_connections.emplace(std::move(connection));
    lock.unlock();
    m_condition.notify_one();
}
