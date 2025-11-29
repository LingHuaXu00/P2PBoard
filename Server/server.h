#ifndef CLIPBOARD_SERVER_H
#define CLIPBOARD_SERVER_H

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <memory>
#include <unordered_set>
#include <mutex>

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;
namespace websocket = beast::websocket;

class session_manager
{
public:
    void add(std::shared_ptr<websocket::stream<tcp::socket>> ws);
    void remove(websocket::stream<tcp::socket> *ws);
    void broadcast(const std::string &message);

private:
    std::unordered_set<websocket::stream<tcp::socket> *> sessions_;
    std::mutex sessions_mutex_;
};

class clipboard_server
{
public:
    clipboard_server(net::io_context &ioc, tcp::endpoint endpoint, session_manager &manager);

private:
    void do_accept();
    void do_handshake(std::shared_ptr<websocket::stream<tcp::socket>> ws);
    void do_read(std::shared_ptr<websocket::stream<tcp::socket>> ws);

    net::io_context &ioc_;
    tcp::acceptor acceptor_;
    session_manager &manager_;
};

#endif
