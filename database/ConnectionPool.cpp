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
    const auto get_conn = [&]
    {
        const auto conn_it = m_free_connections.begin();
        const auto res = *conn_it;
        m_free_connections.erase(conn_it);
        return res;
    };
    std::unique_lock<decltype(m_mtx)> lock(m_mtx);
    if (!m_free_connections.empty())
        return get_conn();

    switch (m_condition.wait_for(lock, timeout))
    {
        case std::cv_status::no_timeout:
            return get_conn();
        case std::cv_status::timeout:
        default:
            return nullptr;;
    }
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
