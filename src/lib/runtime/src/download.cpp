// Copyright (c) 2016-2017 Vinnie Falco (vinnie dot falco at gmail dot com)
// Official repository: https://github.com/boostorg/beast
// Example: HTTP SSL client, asynchronous downloads
// https://stackoverflow.com/questions/63330104/how-to-use-boostbeast-download-a-file-no-blocking-and-with-responses

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <chrono>
#include <thread>

//don't need the cert in a file method or use 
    //don't need the cert in a file method or use 
#include "root_certificates.hpp"
#pragma comment(lib, "C:\\cpp\\openssl-master\\libcrypto.lib")
#pragma comment(lib, "C:\\cpp\\openssl-master\\libssl.lib")

using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

void session_fail(boost::system::error_code ec, char const* what) {
    std::cerr << what << ": " << ec.message() << "\n";
}

class session : public std::enable_shared_from_this<session>
{
public:
    enum responses {
        resp_null,
        resp_ok,
        resp_done,
    };
    using response_call_type = std::function< void(responses, std::size_t)>;
protected:
    tcp::resolver resolver_;
    ssl::stream<tcp::socket> stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::empty_body> req_;
    http::response<http::string_body> res_;
    boost::beast::http::request_parser<boost::beast::http::string_body> header_parser_;
    http::response_parser<http::file_body> file_parser_;
    response_call_type response_call;
    boost::system::error_code file_open_ec;
    //
    std::size_t file_pos = 0;
    std::size_t file_size = 0;

public:
    explicit session(boost::asio::io_context& ioc, ssl::context& ctx, const char* filename)
        : resolver_(ioc)
        , stream_(ioc, ctx)
    {
        file_parser_.body_limit((std::numeric_limits<std::uint64_t>::max)());
        file_parser_.get().body().open(filename, boost::beast::file_mode::write, file_open_ec);
    }
    void run(char const* host, char const* port, char const* target, int version)
    {
        std::cout << "run" << std::endl;
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host))
        {
            boost::system::error_code ec{ static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category() };
            std::cerr << ec.message() << "\n";
            return;
        }
        // Set up an HTTP GET request message
        req_.version(version);
        req_.method(http::verb::get);
        req_.target(target);
        req_.set(http::field::host, host);
        req_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Look up the domain name
        resolver_.async_resolve(host, port, std::bind(
            &session::on_resolve,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
    }
    void on_resolve(boost::system::error_code ec, tcp::resolver::results_type results)
    {
        std::cout << "on_resolve" << std::endl;
        if (ec)
            return session_fail(ec, "resolve");

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(stream_.next_layer(), results.begin(), results.end(), std::bind(
            &session::on_connect,
            shared_from_this(),
            std::placeholders::_1));
    }
    void on_connect(boost::system::error_code ec)
    {
        std::cout << "on_connect" << std::endl;
        if (ec)
            return session_fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(ssl::stream_base::client, std::bind(
            &session::on_handshake,
            shared_from_this(),
            std::placeholders::_1));
    }
    void on_handshake(boost::system::error_code ec)
    {
        std::cout << "on_handshake" << std::endl;
        if (ec)
            return session_fail(ec, "handshake");

        // Send the HTTP request to the remote host
        http::async_write(stream_, req_, std::bind(
            &session::on_write,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
    }
    void on_write(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        std::cout << "on_write" << std::endl;
        if (ec)
            return session_fail(ec, "write");
        if (response_call)
            http::async_read_header(stream_, buffer_, file_parser_, std::bind(
                &session::on_startup,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
        else
            http::async_read_header(stream_, buffer_, file_parser_, std::bind(
                &session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
    }
    std::size_t on_startup(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        std::cout << "on_startup: " << bytes_transferred << std::endl;
        std::string_view view((const char*)buffer_.data().data(), bytes_transferred);
        auto pos = view.find("Content-Length:");
        if (pos == std::string_view::npos)
            assert(true);//error
        file_size = std::stoi(view.substr(pos + sizeof("Content-Length:")).data());
        if (!file_size)
            assert(true);//error
        std::cout << "filesize: " << file_size << std::endl;
        http::async_read_some(stream_, buffer_, file_parser_, std::bind(
            &session::on_read_some,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
        return buffer_.size();
    }
    std::size_t on_read_some(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        //std::cout << "on_read_some" << std::endl;
        if (ec) {
            session_fail(ec, "on_read_some");
            return 0;
        }
        file_pos += bytes_transferred;
        if (!bytes_transferred && file_pos) {
            on_shutdown(ec);
            return 0;
        }
        response_call(resp_ok, file_pos);

        //std::cout << "session::on_read_some: " << file_pos << std::endl;
        http::async_read_some(stream_, buffer_, file_parser_, std::bind(
            &session::on_read_some,
            shared_from_this(),
            std::placeholders::_1,
            std::placeholders::_2));
        return buffer_.size();
    }
    std::size_t on_read(boost::system::error_code ec, std::size_t bytes_transferred)
    {
        file_pos += bytes_transferred;
        if (!bytes_transferred && file_pos) {
            on_shutdown(ec);
            return 0;
        }
        std::cout << "on_read: " << bytes_transferred << std::endl;
        http::async_read(stream_, buffer_, file_parser_,
            std::bind(&session::on_read,
                shared_from_this(),
                std::placeholders::_1,
                std::placeholders::_2));
        return buffer_.size();
    }
    void on_shutdown(boost::system::error_code ec)
    {
        std::cout << "on_shutdown" << std::endl;
        if (ec == boost::asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }
        if (response_call)
            response_call(resp_done, 0);
        if (ec)
            return session_fail(ec, "shutdown");
    }
    auto get_file_status() const { return file_open_ec; }
    void set_response_call(response_call_type the_call) { response_call = the_call; }
    std::size_t get_download_size() const { return file_size; }
    //std::string get_content() const { return "test"; }// return response;}
};

#define NO_BLOCKING
int main(int argc, char** argv)
{
    //in a UI app you will need to keep a persistant thread/pool;
    std::thread reader_thread;
    //for an application where this never changes, this can just be put in the session class
    auto const host = "reserveanalyst.com";
    auto const port = "443";
#ifdef NO_BLOCKING // the large file
    auto const target = "/downloads/demo.msi";
#else // the small file
    auto const target = "/server.xml";
#endif
    boost::asio::io_context ioc;
    ssl::context ctx{ ssl::context::sslv23_client };
    load_root_certificates(ctx);
    //end, put in the session class
    auto so = std::make_shared<session>(ioc, ctx, "content.txt");//may be big binary
    so->run(host, port, target, 11);//so->run(target);
    //
    session::responses glb_response = session::resp_null;
    bool test_bool = false; //stand in for 'SendMessage' values
    std::size_t buf_size = 0; //stand in for 'SendMessage' values
#ifdef NO_BLOCKING
    auto static const lambda = [&glb_response, &buf_size](session::responses response, std::size_t bytes_transferred) {
        glb_response = response;
        buf_size = bytes_transferred;
        //drive your progress bar from here in a GUI app
        //sizes = the_beast_object.get_file_size() - size;//because size is what is left
        //cDownloadProgreess.SetPos((LPARAM)(sizes * 100 / the_beast_object.get_file_size()));
    };
    so->set_response_call(lambda);
#else
    ioc.run();
    std::cout << "ioc run exited" << std::endl;
#endif

#ifdef NO_BLOCKING
    //    reader_thread.swap(std::thread{ [&ioc]() { ioc.run(); } });
    std::thread new_thread{ [&ioc]() { ioc.run(); } };
    reader_thread.swap(new_thread);
#endif
    bool quit = false; //true: as if a cancel button was pushed; won't finish download
    //pseudo message pump
    for (int i = 0; ; ++i) {

        switch (glb_response) { //ad hoc as if messaged
        case session::responses::resp_ok:
            std::cout << "from sendmessage: " << buf_size << std::endl;
            break;
        case session::responses::resp_done:
            std::cout << "from sendmessage: done" << std::endl;
        }//switch
        glb_response = session::responses::resp_null;
        if (!(i % 10))
            std::cout << "in message pump, stopped: " << std::boolalpha << ioc.stopped() << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (quit && i == 10) //the cancel message
            ioc.stop();
        if (ioc.stopped())//just quit to test join.
            break;
    }
    if (reader_thread.joinable())//in the case a thread was never started
        reader_thread.join();
    std::cout << "exiting, program was quit" << std::endl;

    return EXIT_SUCCESS;
}