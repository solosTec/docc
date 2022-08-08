#include <download.h>

#include <openssl/ssl.h>

#include <boost/asio/ssl/error.hpp>

#include <boost/beast/version.hpp>

#include <fmt/color.h>
#include <fmt/core.h>

namespace docscript {
    namespace net {
        download::download(
            boost::asio::any_io_executor ex,
            boost::asio::ssl::context &ctx,
            char const *path,
            std::function<void()> complete,
            std::function<void(std::string)> redirect)
            : resolver_(ex)
            , stream_(ex, ctx)
            , body_parser_()
            , buffer_{}
            , cb_complete_(complete)
            , cb_redirect_(redirect) {

            body_parser_.body_limit((std::numeric_limits<std::uint64_t>::max)());
            boost::system::error_code ec;
            body_parser_.get().body().open(path, boost::beast::file_mode::write, ec);

            //  buffer size
            buffer_.reserve(4096);
        }

        void download::start(boost::urls::url_view uri) {

            std::string const host = uri.encoded_host();

            fmt::print(
                stdout,
                fg(fmt::color::gray),
                "***info : scheme = {} ({})\n",
                uri.scheme(),
                +static_cast<std::uint8_t>(uri.scheme_id()));

            fmt::print(stdout, fg(fmt::color::gray), "***info : host  = {}\n", host);

            // Set SNI Hostname (many hosts need this to handshake successfully)
            if (!SSL_set_tlsext_host_name(stream_.native_handle(), host.data())) {
                boost::beast::error_code ec{static_cast<int>(::ERR_get_error()), boost::asio::error::get_ssl_category()};
                fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, "***error: {}\n", ec.message());
                return;
            }

            fmt::print(
                stdout, fg(fmt::color::gray), "***info : path  = {}\n", uri.encoded_path()); //  same as uri.encoded_segments()
            if (uri.has_query()) {
                fmt::print(stdout, fg(fmt::color::gray), "***info : query = {}\n", uri.encoded_query());
            }
            if (uri.has_fragment()) {
                fmt::print(stdout, fg(fmt::color::gray), "***info : fragment = {}\n", uri.encoded_fragment());
            }

            std::string target = uri.encoded_path();
            if (uri.has_query()) {
                target += '?';
                target += uri.encoded_query();
            }
            if (uri.has_fragment()) {
                target += '#';
                target += uri.encoded_fragment();
            }

            // Set up an HTTP GET request message
            req_.version(11); //  HTTP1.1
            req_.method(boost::beast::http::verb::get);
            req_.target(target);
            req_.set(boost::beast::http::field::host, host);
            req_.set(boost::beast::http::field::user_agent, BOOST_BEAST_VERSION_STRING);

            std::string const port = (uri.port_number() == 0) ? "443" : std::to_string(uri.port_number());

            // Look up the domain name
            resolver_.async_resolve(host, port, boost::beast::bind_front_handler(&download::on_resolve, shared_from_this()));
        }

        void download::on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results) {
            if (ec) {
                fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, "***error: resolve - {}\n", ec.message());
                return;
            }

            // Set a timeout on the operation
            boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(90));

            boost::beast::get_lowest_layer(stream_).async_connect(
                results, boost::beast::bind_front_handler(&download::on_connect, shared_from_this()));
        }

        void download::on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep) {
            if (ec) {
                fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, "***error: connect - {}\n", ec.message());
                return; // session_fail(ec, "connect");
            }

            //
            //  formatter required
            //
            // fmt::print(stdout, fg(fmt::color::gray), "***info : connect to {}\n", ep);

            // Perform the SSL handshake
            stream_.async_handshake(
                boost::asio::ssl::stream_base::client,
                boost::beast::bind_front_handler(&download::on_handshake, shared_from_this()));
        }

        void download::on_handshake(boost::beast::error_code ec) {
            if (ec) {
                fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, "***error: handshake - {}\n", ec.message());
                return; // session_fail(ec, "handshake");
            }

            // Set a timeout on the operation
            boost::beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(90));

            // Send the HTTP request to the remote host
            boost::beast::http::async_write(
                stream_, req_, boost::beast::bind_front_handler(&download::on_write, shared_from_this()));
        }

        void download::on_write(boost::beast::error_code ec, std::size_t bytes_transferred) {
            boost::ignore_unused(bytes_transferred);

            if (ec) {
                fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, "***error: write - {}\n", ec.message());
                return; // session_fail(ec, "write");
            }

            fmt::print(stdout, fg(fmt::color::gray), "***info : {} bytes sent\n", bytes_transferred);

            //
            //  start reading
            //
            boost::beast::http::async_read_some(
                stream_, buffer_, body_parser_, boost::beast::bind_front_handler(&download::on_read_all, shared_from_this()));
        }

        void download::on_read_all(boost::beast::error_code ec, std::size_t bytes_transferred) {

            boost::ignore_unused(bytes_transferred);
            if (ec) {
                fmt::print(stderr, fg(fmt::color::crimson) | fmt::emphasis::bold, "***error: read all - {}\n", ec.message());
                return; // session_fail(ec, "read all");
            }

            auto const content_length = (body_parser_.content_length()) ? *body_parser_.content_length() : 0u;
            auto const remaining_length = (body_parser_.content_length_remaining()) ? *body_parser_.content_length_remaining() : 0u;

            //
            //  log progress
            //
            fmt::print(
                stdout,
                fg(fmt::color::gray),
                "***info : remaining length = {} ({}%)\n",
                remaining_length,
                ((content_length != 0) ? (100u - (100 * remaining_length / content_length)) : 0u));

            if (!body_parser_.is_done()) {
                //
                //  incomplete - continue reading
                //
                boost::beast::http::async_read_some(
                    stream_, buffer_, body_parser_, boost::beast::bind_front_handler(&download::on_read_all, shared_from_this()));
            } else {
                //
                //  complete
                //
                auto body = body_parser_.release();

                boost::beast::error_code ec;
                body.body().file().close(ec);

                switch (body.result()) {
                case boost::beast::http::status::moved_permanently: // 301
                    fmt::print(
                        stdout, fg(fmt::color::gray), "***info : HTTP status code : {} - moved permanently\n", body.result_int());

                    {
                        auto const pos = body.find("Location");
                        if (pos != body.end()) {
                            // std::cout << "download                    : " << pos->value() << std::endl;
                            std::string const location = pos->value().to_string();
                            // fmt::print(stdout, fg(fmt::color::gray), "***info : redirect to {}\n", location);
                            cb_redirect_(location);
                        }
                    }
                    break;
                case boost::beast::http::status::found: // 302
                    fmt::print(stdout, fg(fmt::color::gray), "***info : HTTP status code : {} - found\n", body.result_int());
                    // std::cout << "HTTP status code            : 302 - found" << std::endl;
                    {
                        auto const pos = body.find("Location");
                        if (pos != body.end()) {
                            // std::cout << "download                    : " << pos->value() << std::endl;
                            std::string const location = pos->value().to_string();
                            // fmt::print(stdout, fg(fmt::color::gray), "***info : redirect to {}\n", location);
                            cb_redirect_(location);
                        }
                    }
                    break;
                default:
                    fmt::print(stdout, fg(fmt::color::gray), "***info : HTTP status code : {}\n", body.result_int());
                    // std::cout << "HTTP status code            : " << body.result_int() << std::endl;
                    for (auto pos = body.begin(); pos != body.end(); ++pos) {
                        //
                        //  formatter required
                        //
                        // fmt::print(stdout, fg(fmt::color::ghost_white), "***trace: {} = {}\n", pos->name(), pos->value());
                        // std::cout << "fields (" << pos->name() << ": " << pos->value() << ")" << std::endl;
                    };
                    break;
                }
            }
        }
    } // namespace net
} // namespace docscript
