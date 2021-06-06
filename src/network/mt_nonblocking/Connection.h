#ifndef AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <mutex>
#include <deque>
#include <memory>

#include <sys/epoll.h>
#include <sys/uio.h>

#include <afina/Storage.h>
#include <afina/execute/Command.h>
#include <spdlog/logger.h>

#include "protocol/Parser.h"
namespace Afina {
namespace Network {
namespace MTnonblock {

class Connection {
public:
    // Added storage and logger to constructor.
    Connection(
        int s,
        std::shared_ptr<Afina::Storage> storage,
        std::shared_ptr<spdlog::logger> logger
    ) : _socket(s), pStorage(storage), _logger(logger)
    {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

    inline bool isAlive() const
    {
        return _is_active;
    }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class Worker;
    friend class ServerImpl;

    std::atomic<bool> _is_active;
    int _socket;

    const size_t SIZE = 4096;
    char client_buffer[4096] = "";

    int _parsed = 0;
    std::size_t _shift = 0;
    std::size_t MAX = 128;
    std::size_t arg_remains = 0;
    Protocol::Parser parser;
    std::string argument_for_command;
    std::unique_ptr<Execute::Command> command_to_execute;
    std::deque<std::string> responses;
    std::shared_ptr<spdlog::logger> _logger; // one logger for all connections.
    std::shared_ptr<Afina::Storage> pStorage; // one storage for all connections.

    struct epoll_event _event;
};

} // namespace MTnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
