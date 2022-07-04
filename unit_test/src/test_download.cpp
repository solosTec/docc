#ifdef STAND_ALONE
#define BOOST_TEST_MODULE unit_test
#endif

#include "root_certificates.hpp"
#include <boost/url.hpp>

#include <boost/test/unit_test.hpp>

#include <cyng/io/hex_dump.hpp>
//#include <cyng/obj/>
//#include <cyng/net/client_factory.hpp>
//#include <cyng/net/server_factory.hpp>
//#include <cyng/io/ostream.h>

#include <iostream>
#include <thread>

#include <openssl/ssl.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/error.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>

BOOST_AUTO_TEST_SUITE(net_suite)

class parser {
  public:
};

// https://stackoverflow.com/questions/63330104/how-to-use-boostbeast-download-a-file-no-blocking-and-with-responses

void session_fail(boost::system::error_code ec, char const *what) { std::cerr << what << ": " << ec.message() << "\n"; }

class session : public std::enable_shared_from_this<session> {
  public:
    enum responses {
        resp_null,
        resp_ok,
        resp_done,
    };
    using response_call_type = std::function<void(responses, std::size_t)>;

  protected:
    boost::asio::ip::tcp::resolver resolver_;
    boost::asio::ssl::stream<boost::asio::ip::tcp::socket> stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    boost::beast::http::request<boost::beast::http::empty_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;
    // boost::beast::http::request_parser<boost::beast::http::string_body> header_parser_;
    boost::beast::http::response_parser<boost::beast::http::file_body> file_parser_;
    response_call_type response_call;
    boost::system::error_code file_open_ec;
    //
    std::size_t file_pos_ = 0;
    std::size_t file_size_ = 0;

  public:
    explicit session(boost::asio::io_context &ioc, boost::asio::ssl::context &ctx, const char *filename)
        : resolver_(ioc)
        , stream_(ioc, ctx) {
        file_parser_.body_limit((std::numeric_limits<std::uint64_t>::max)());
        file_parser_.get().body().open(filename, boost::beast::file_mode::write, file_open_ec);
    }
    void run(char const *host, char const *port, char const *target, int version) {
        std::cout << "run " << host << ':' << port << target << std::endl;
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
            boost::system::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }
        // Set up an HTTP GET request message
        req_.version(version);
        req_.method(boost::beast::http::verb::get);
        req_.target(target);
        req_.set(boost::beast::http::field::host, host);
        req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING); // "Boost.Beast.322"

        // Look up the domain name
        resolver_.async_resolve(
            host, port, std::bind(&session::on_resolve, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
    void on_resolve(boost::system::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
        std::cout << "on_resolve " << results.size() << std::endl;
        if (ec)
            return session_fail(ec, "resolve");

        // Make the connection on the IP address we get from a lookup
        boost::asio::async_connect(
            stream_.next_layer(),
            results.begin(),
            results.end(),
            std::bind(&session::on_connect, shared_from_this(), std::placeholders::_1));
    }
    void on_connect(boost::system::error_code ec) {
        std::cout << "on_connect" << std::endl;
        if (ec)
            return session_fail(ec, "connect");

        // Perform the SSL handshake
        stream_.async_handshake(
            boost::asio::ssl::stream_base::client, std::bind(&session::on_handshake, shared_from_this(), std::placeholders::_1));
    }
    void on_handshake(boost::system::error_code ec) {
        std::cout << "on_handshake" << std::endl;
        if (ec)
            return session_fail(ec, "handshake");

        // Send the HTTP request to the remote host
        boost::beast::http::async_write(
            stream_, req_, std::bind(&session::on_write, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }
    void on_write(boost::system::error_code ec, std::size_t bytes_transferred) {
        std::cout << "on_write " << bytes_transferred << " bytes" << std::endl;
        if (ec)
            return session_fail(ec, "write");
        if (response_call)
            boost::beast::http::async_read_header(
                stream_,
                buffer_,
                file_parser_,
                std::bind(&session::on_startup, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        else
            boost::beast::http::async_read_header(
                stream_,
                buffer_,
                file_parser_,
                std::bind(&session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
    }

    std::size_t on_startup(boost::system::error_code ec, std::size_t bytes_transferred) {
        std::cout << "on_startup: " << bytes_transferred << " bytes" << std::endl;

        auto const cl = file_parser_.content_length();
        auto const clr = file_parser_.content_length_remaining();

        if (cl) {
            std::cout << "content length: " << *cl << std::endl;
            file_size_ = *cl;
        }
        if (clr) {
            std::cout << "remaining content length: " << *clr << std::endl;
        }

        // for (auto pos = file_parser_.get().begin(); pos != file_parser_.get().end(); ++pos) {
        //     std::cout << "on header (" << pos->name() << ": " << pos->value() << ")" << std::endl;
        // };

        BOOST_ASSERT(file_parser_.is_header_done());

        if (file_size_ != 0) {
            boost::beast::http::async_read_some(
                stream_,
                buffer_,
                file_parser_,
                std::bind(&session::on_read_some, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
            return buffer_.size();
        }

        on_shutdown(ec);
        return 0u;
    }

    std::size_t on_read_some(boost::system::error_code ec, std::size_t bytes_transferred) {
        // std::cout << "on_read_some " << bytes_transferred << " bytes" << std::endl;
        if (ec) {
            session_fail(ec, "on_read_some");
            return 0;
        }
        file_pos_ += bytes_transferred;
        // if (!bytes_transferred && file_pos_) {
        if (file_size_ == file_pos_) {
            on_shutdown(ec);
            return 0;
        }
        // auto const clr = file_parser_.content_length_remaining();
        // if (clr) {
        //     std::cout << "on_read_some remaining " << *clr << " bytes" << std::endl;
        //     if (*clr == 0) {
        //         std::cout << "on_read_some complete" << std::endl;
        //         on_shutdown(ec);
        //         return 0;
        //     }
        // }

        response_call(resp_ok, file_pos_);

        // std::cout << "session::on_read_some: " << file_pos << std::endl;
        boost::beast::http::async_read_some(
            stream_,
            buffer_,
            file_parser_,
            std::bind(&session::on_read_some, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        return buffer_.size();
    }
    std::size_t on_read(boost::system::error_code ec, std::size_t bytes_transferred) {
        file_pos_ += bytes_transferred;
        // if (!bytes_transferred && file_pos_) {
        if (file_size_ == file_pos_) {
            on_shutdown(ec);
            return 0;
        }
        std::cout << "on_read: " << bytes_transferred << " bytes" << std::endl;

        boost::beast::http::async_read(
            stream_,
            buffer_,
            file_parser_,
            std::bind(&session::on_read, shared_from_this(), std::placeholders::_1, std::placeholders::_2));
        return buffer_.size();
    }

    void on_shutdown(boost::system::error_code ec) {
        std::cout << "on_shutdown" << std::endl;
        if (ec == boost::asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec.assign(0, ec.category());
        }

        if (response_call) {
            response_call(resp_done, 0);
        }

        auto body = file_parser_.release();
        body.body().file().close(ec);

        switch (body.result()) {
        case boost::beast::http::status::moved_permanently: // 301
            std::cout << "HTTP status code            : 301 - moved permanently" << std::endl;
            break;
        case boost::beast::http::status::found: // 302
            std::cout << "HTTP status code            : 302 - found" << std::endl;
            {
                auto const pos = body.find("Location");
                if (pos != body.end()) {
                    std::cout << "download                    : " << pos->value() << std::endl;
                    // download(ec, pos->value());
                }
            }
            break;
        default:
            std::cout << "HTTP status code            : " << body.result_int() << std::endl;
            for (auto pos = body.begin(); pos != body.end(); ++pos) {
                std::cout << "fields (" << pos->name() << ": " << pos->value() << ")" << std::endl;
            };
            break;
        }

        if (ec)
            return session_fail(ec, "shutdown");
    }

    auto get_file_status() const { return file_open_ec; }
    void set_response_call(response_call_type the_call) { response_call = the_call; }
    std::size_t get_download_size() const { return file_size_; }
    // std::string get_content() const { return "test"; }// return response;}
};

BOOST_AUTO_TEST_CASE(download) {
    // in a UI app you will need to keep a persistant thread/pool;
    std::thread reader_thread;

    // for an application where this never changes, this can just be put in the session class

    // auto const host = "www.openssl.org";
    // auto const port = "443";
    // auto const target = "/source/openssl-3.0.2.tar.gz";

    //  https://github.com/twbs/bootstrap/releases/download/v5.0.2/bootstrap-5.0.2-dist.zip
    auto const host = "github.com";
    auto const port = "443";
    auto const target = "/twbs/bootstrap/releases/download/v5.0.2/bootstrap-5.0.2-dist.zip";

    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{boost::asio::ssl::context::sslv23_client};
    //  see https://stackoverflow.com/a/49511782
    //  not really required
    load_root_certificates(ctx);
    // end, put in the session class
    auto so = std::make_shared<session>(ioc, ctx, "bootstrap.txt"); // may be big binary
    // auto so = std::make_shared<session>(ioc, ctx, "content.txt"); // may be big binary
    so->run(host, port, target, 11); // so->run(target);
    //
    session::responses glb_response = session::resp_null;
    bool test_bool = false;   // stand in for 'SendMessage' values
    std::size_t buf_size = 0; // stand in for 'SendMessage' values
#ifdef NO_BLOCKING
    auto static const lambda = [&glb_response, &buf_size](session::responses response, std::size_t bytes_transferred) {
        glb_response = response;
        buf_size = bytes_transferred;
        // drive your progress bar from here in a GUI app
        // sizes = the_beast_object.get_file_size() - size;//because size is what is left
        // cDownloadProgreess.SetPos((LPARAM)(sizes * 100 / the_beast_object.get_file_size()));
    };
    so->set_response_call(lambda);
#else
    ioc.run();
    std::cout << "ioc run exited" << std::endl;
#endif

#ifdef NO_BLOCKING
    //    reader_thread.swap(std::thread{ [&ioc]() { ioc.run(); } });
    std::thread new_thread{[&ioc]() { ioc.run(); }};
    reader_thread.swap(new_thread);
#endif
    bool quit = false; // true: as if a cancel button was pushed; won't finish download
    // pseudo message pump
    for (int i = 0;; ++i) {

        switch (glb_response) { // ad hoc as if messaged
        case session::responses::resp_ok:
            std::cout << "from sendmessage: " << buf_size << std::endl;
            break;
        case session::responses::resp_done:
            std::cout << "from sendmessage: done" << std::endl;
        } // switch
        glb_response = session::responses::resp_null;
        if (!(i % 10)) {
            std::cout << "in message pump, stopped: " << std::boolalpha << ioc.stopped() << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (quit && i == 10) // the cancel message
            ioc.stop();
        if (ioc.stopped()) // just quit to test join.
            break;
    }
    if (reader_thread.joinable()) { // in the case a thread was never started
        reader_thread.join();
    }
    std::cout << "exiting, program was quit" << std::endl;

    // return EXIT_SUCCESS;
}

/**
 * async SSL client
 */
class client : public std::enable_shared_from_this<client> {
    boost::asio::ip::tcp::resolver resolver_;
    boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
    boost::beast::flat_buffer buffer_; // (Must persist between reads)
    boost::beast::http::request<boost::beast::http::empty_body> req_;
    boost::beast::http::response<boost::beast::http::string_body> res_;

    boost::beast::http::response_parser<boost::beast::http::string_body> header_parser_;
    boost::beast::http::response_parser<boost::beast::http::file_body> body_parser_;

  public:
    client(boost::asio::any_io_executor ex, boost::asio::ssl::context &ctx, char const *path)
        : resolver_(ex)
        , stream_(ex, ctx)
        , buffer_{}
        , req_{}
        , res_{}
        , body_parser_{} {

        body_parser_.body_limit((std::numeric_limits<std::uint64_t>::max)());
        boost::system::error_code ec;
        body_parser_.get().body().open(path, boost::beast::file_mode::write, ec);

        //  buffer size
        buffer_.reserve(4096);
    }

    void run(char const *host, char const *port, char const *target, int version) {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if (!SSL_set_tlsext_host_name(stream_.native_handle(), host)) {
            boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }

        // Set up an HTTP GET request message
        req_.version(version);
        req_.method(boost::beast::http::verb::get);
        req_.target(target);
        req_.set(boost::beast::http::field::host, host);
        req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

        // Look up the domain name
        resolver_.async_resolve(host, port, boost::beast::bind_front_handler(&client::on_resolve, shared_from_this()));
    }

    void on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
        if (ec)
            return session_fail(ec, "resolve");

        // Set a timeout on the operation
        boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(90));

        boost::beast::get_lowest_layer(stream_).async_connect(
            results, boost::beast::bind_front_handler(&client::on_connect, shared_from_this()));
    }

    void on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep) {
        if (ec)
            return session_fail(ec, "connect");

        std::cout << "on connect(" << ep << ")" << std::endl;

        // Perform the SSL handshake
        stream_.async_handshake(
            ssl::stream_base::client, boost::beast::bind_front_handler(&client::on_handshake, shared_from_this()));
    }

    void on_handshake(boost::beast::error_code ec) {
        if (ec)
            return session_fail(ec, "handshake");

        // Set a timeout on the operation
        boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(90));

        // Send the HTTP request to the remote host
        boost::beast::http::async_write(stream_, req_, boost::beast::bind_front_handler(&client::on_write, shared_from_this()));
    }

    void on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return session_fail(ec, "write");

        std::cout << "on write (" << bytes_transferred << " bytes)" << std::endl;

        // Receive the HTTP response
        //  [original code]
        // boost::beast::http::async_read(
        //    stream_, buffer_, res_, boost::beast::bind_front_handler(&client::on_read, shared_from_this()));

        // Parse the HTTP response
        // boost::beast::http::async_read_header(
        //    stream_, buffer_, header_parser_, boost::beast::bind_front_handler(&client::on_read_header, shared_from_this()));

        boost::beast::http::async_read_some(
            stream_, buffer_, body_parser_, boost::beast::bind_front_handler(&client::on_read_all, shared_from_this()));
    }

    void on_read_all(boost::beast::error_code ec, std::size_t bytes_transferred) {

        boost::ignore_unused(bytes_transferred);
        if (ec)
            return session_fail(ec, "read all");

        std::cout << "on header (bytes_transferred: " << bytes_transferred << " bytes)" << std::endl;
        auto const cl = body_parser_.content_length();
        auto const clu = cl ? *cl : 0u;
        std::cout << "on header (Content-Length   : " << clu << " bytes, " << (clu / 1024) << "kB"
                  << ")" << std::endl;
        auto const rcl = body_parser_.content_length_remaining();
        auto rclu = rcl ? *rcl : 0u;
        std::cout << "on header (Remaining-Length : " << rclu << " bytes, " << (rclu / 1024) << "kB, "
                  << ((clu != 0) ? (100u - (100 * rclu / clu)) : 0u) << "%)" << std::endl;
        std::cout << "on header (header complete  : " << (body_parser_.is_header_done() ? "yes" : "no") << ")" << std::endl;
        std::cout << "on header (body complete    : " << (body_parser_.is_done() ? "yes" : "no") << ")" << std::endl;

        // if (body_parser_.is_header_done()) {
        //     for (auto pos = body_parser_.get().begin(); pos != body_parser_.get().end(); ++pos) {
        //         std::cout << "fields (" << pos->name() << ": " << pos->value() << ")" << std::endl;
        //     };
        // }

        if (!body_parser_.is_done()) {
            boost::beast::http::async_read_some(
                stream_, buffer_, body_parser_, boost::beast::bind_front_handler(&client::on_read_all, shared_from_this()));
        } else {
            auto body = body_parser_.release();

            boost::beast::error_code ec;
            body.body().file().close(ec);

            switch (body.result()) {
            case boost::beast::http::status::moved_permanently: // 301
                std::cout << "HTTP status code            : 301 - moved permanently" << std::endl;
                {
                    auto const pos = body.find("Location");
                    if (pos != body.end()) {
                        std::cout << "download                    : " << pos->value() << std::endl;
                        download(ec, pos->value());
                    }
                }
                break;
            case boost::beast::http::status::found: // 302
                std::cout << "HTTP status code            : 302 - found" << std::endl;
                {
                    auto const pos = body.find("Location");
                    if (pos != body.end()) {
                        std::cout << "download                    : " << pos->value() << std::endl;
                        download(ec, pos->value());
                    }
                }
                break;
            default:
                std::cout << "HTTP status code            : " << body.result_int() << std::endl;
                for (auto pos = body.begin(); pos != body.end(); ++pos) {
                    std::cout << "fields (" << pos->name() << ": " << pos->value() << ")" << std::endl;
                };
                break;
            }
        }
    }

    void download(boost::beast::error_code ec, boost::beast::string_view url) {
        std::cout << "continue download:" << std::endl;
        on_shutdown(ec);

        auto const uv = boost::urls::parse_uri(url).value();
        std::cout << "scheme   : " << uv.scheme() << ", " << boost::urls::to_string(uv.scheme_id()) << std::endl;
        std::cout << "scheme id: " << +static_cast<std::uint8_t>(uv.scheme_id()) << std::endl;
        std::cout << "host     : " << uv.encoded_host() << std::endl;
        std::cout << "host+port: " << uv.encoded_host_and_port() << std::endl;
        std::cout << "port     : " << uv.port_number() << std::endl;
        std::cout << "path     : " << uv.encoded_path() << std::endl;
        std::cout << "segments : " << uv.encoded_segments() << std::endl;
        std::cout << "query    : " << uv.query() << std::endl;
        for (auto v : uv.params()) {
            std::cout << "\t" << v.key << ": " << v.value << std::endl;
        }

        // run("github.com", "443", "/twbs/bootstrap/releases/download/v5.0.2/bootstrap-5.0.2-dist.zip", 11);
        std::string const host = uv.encoded_host();
        std::string target = uv.encoded_path();
        target += "&";
        target += uv.query();
        std::cout << "target   : " << target << std::endl;
        run(host.c_str(), "443", target.c_str(), 11);

        //        std::vector<std::string> parts;
        //        boost::split(parts, url, boost::is_any_of("/"), boost::token_compress_on);
        //#ifdef _DEBUG
        //        for (auto const &part : parts) {
        //            std::cout << part << std::endl;
        //        }
        //#endif
        //        if (parts.size() > 3) {
        //            auto const host = parts.at(1);
        //            auto const target =
        //                "/github-production-release-asset-2e65be/2126244/45d95b00-d3a1-11eb-9c4b-0c7347ba205f?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20220320%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20220320T190554Z&X-Amz-Expires=300&X-Amz-Signature=f4d076355f6b8651f88b971016db599e2ad4a3ab787ad6577fae9a8170f6f1d1&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=2126244&response-content-disposition=attachment%3B%20filename%3Dbootstrap-5.0.2-dist.zip&response-content-type=application%2Foctet-stream";
        //            run(host.c_str(), "443", target, 11);
        //        }
    }

    void on_read_header(boost::beast::error_code ec, std::size_t bytes_transferred) {

        if (ec)
            return session_fail(ec, "read header");

        std::cout << "on header (bytes_transferred: " << bytes_transferred << " bytes)" << std::endl;
        std::cout << "on header (Content-Length   : " << *header_parser_.content_length() << ")" << std::endl;

        for (auto pos = header_parser_.get().begin(); pos != header_parser_.get().end(); ++pos) {
            std::cout << "on header (" << pos->name() << ": " << pos->value() << ")" << std::endl;
        };
    }

    void on_read(boost::beast::error_code ec, std::size_t bytes_transferred) {
        boost::ignore_unused(bytes_transferred);

        if (ec)
            return session_fail(ec, "read");

        std::cout << "on read (" << bytes_transferred << " bytes)" << std::endl;

        // Write the message to standard out
        std::cout << res_ << std::endl;

        // Set a timeout on the operation
        boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Gracefully close the stream
        stream_.async_shutdown(boost::beast::bind_front_handler(&client::on_shutdown, shared_from_this()));
    }

    void on_shutdown(boost::beast::error_code ec) {
        if (ec == boost::asio::error::eof) {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if (ec)
            return session_fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
        std::cout << "complete" << std::endl;
    }
};

BOOST_AUTO_TEST_CASE(sslclient) {

    boost::asio::io_context ioc;
    boost::asio::ssl::context ctx{boost::asio::ssl::context::tlsv12_client};
    //  see https://stackoverflow.com/a/49511782
    //  not really required but helps
    load_root_certificates(ctx);
    // Verify the remote server's certificate
    ctx.set_verify_mode(ssl::verify_peer);

    //
    // (1) Contains a redirect
    //
    // https://github.com/twbs/bootstrap/releases/download/v5.0.2/bootstrap-5.0.2-dist.zip
    std::make_shared<client>(boost::asio::make_strand(ioc), ctx, "bootstrap-5.0.2-dist.zip")
        ->run("github.com", "443", "/twbs/bootstrap/releases/download/v5.0.2/bootstrap-5.0.2-dist.zip", 11);

    //
    //  (2) Simple download
    //
    // https://cdn.jsdelivr.net/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css
    // std::make_shared<client>(boost::asio::make_strand(ioc), ctx, "bootstrap.min.css")
    //    ->run("cdn.jsdelivr.net", "443", "/npm/bootstrap@5.0.2/dist/css/bootstrap.min.css", 11);

    // https://www.openssl.org/source/openssl-3.0.2.tar.gz

    // std::make_shared<client>(boost::asio::make_strand(ioc), ctx)->run("www.boost.org", "443", "/", 11);
    // std::make_shared<client>(boost::asio::make_strand(ioc), ctx)->run("www.openssl.org", "443", "/source/openssl-3.0.2.tar.gz",
    // 11);

    // https://objects.githubusercontent.com/github-production-release-asset-2e65be/2126244/45d95b00-d3a1-11eb-9c4b-0c7347ba205f?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20220318%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20220318T124225Z&X-Amz-Expires=300&X-Amz-Signature=5973b7f5f75b79b7e5e6cb82ab290b267d694b4d044d0c59d6e43f01eec04f01&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=2126244&response-content-disposition=attachment%3B%20filename%3Dbootstrap-5.0.2-dist.zip&response-content-type=application%2Foctet-stream
    // std::make_shared<client>(boost::asio::make_strand(ioc), ctx)
    //   ->run(
    //       "objects.githubusercontent.com",
    //       "443",
    //       "/github-production-release-asset-2e65be/2126244/45d95b00-d3a1-11eb-9c4b-0c7347ba205f?X-Amz-Algorithm=AWS4-HMAC-SHA256&X-Amz-Credential=AKIAIWNJYAX4CSVEH53A%2F20220318%2Fus-east-1%2Fs3%2Faws4_request&X-Amz-Date=20220318T124225Z&X-Amz-Expires=300&X-Amz-Signature=5973b7f5f75b79b7e5e6cb82ab290b267d694b4d044d0c59d6e43f01eec04f01&X-Amz-SignedHeaders=host&actor_id=0&key_id=0&repo_id=2126244&response-content-disposition=attachment%3B%20filename%3Dbootstrap-5.0.2-dist.zip&response-content-type=application%2Foctet-stream",
    //       11);

    // https://cdn.jsdelivr.net/npm/katex@0.16.0/dist/katex.css
    // https://cdn.jsdelivr.net/npm/katex@0.16.0/dist/katex.js

    //  will return when the get operation is complete.
    ioc.run();
}

BOOST_AUTO_TEST_SUITE_END()
