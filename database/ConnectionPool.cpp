#include "ConnectionPool.h"

ConnectionPool::ConnectionPool(const std::string& connection_string, std::size_t num_connections)
{
    for (auto i = 0u; i < num_connections; ++i)
        m_free_connections.emplace_back(std::make_shared<pqxx::connection>(connection_string));
}

ConnectionPool::conn_ptr ConnectionPool::AcquireConnection(std::chrono::steady_clock::duration timeout)
{
    std::unique_lock<decltype(m_mtx)> lock(m_mtx);
    const auto get_conn = [&]
    {
        auto conn = m_free_connections.back();
        m_free_connections.pop_back();
        return conn;
    };
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
    m_free_connections.emplace_back(std::move(connection));
    lock.unlock();
    m_condition.notify_one();
}
