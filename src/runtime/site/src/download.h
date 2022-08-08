/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Sylko Olzscher
 *
 */
#ifndef CYNG_NET_DOWNLOAD_H
#define CYNG_NET_DOWNLOAD_H

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>

#include <boost/url.hpp>

#include <functional>
#include <memory>

namespace docscript {
    namespace net {

        /**
         * Provides a simple interface to download files over HTTP/s.
         */
        class download : public std::enable_shared_from_this<download> {

          public:
            /**
             * @param ex i/o object
             * @param ctx SSL context
             * @param path path to store the received file
             */
            download(
                boost::asio::any_io_executor ex,
                boost::asio::ssl::context &ctx,
                char const *path,
                std::function<void()> complete,
                std::function<void(std::string)> redirect);

            /**
             * start download
             */
            void start(boost::urls::url_view);

          private:
            void on_resolve(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type results);
            void on_connect(boost::beast::error_code ec, boost::asio::ip::tcp::resolver::results_type::endpoint_type ep);
            void on_handshake(boost::beast::error_code ec);
            void on_write(boost::beast::error_code ec, std::size_t bytes_transferred);
            void on_read_all(boost::beast::error_code ec, std::size_t bytes_transferred);

          private:
            boost::asio::ip::tcp::resolver resolver_;
            boost::beast::ssl_stream<boost::beast::tcp_stream> stream_;
            boost::beast::http::response_parser<boost::beast::http::file_body> body_parser_;
            /**
             * HTTP request
             */
            boost::beast::http::request<boost::beast::http::empty_body> req_;

            /**
             * receive buffer
             */
            boost::beast::flat_buffer buffer_;

            std::function<void()> cb_complete_;
            std::function<void(std::string)> cb_redirect_;
        };
    } // namespace net
} // namespace docscript

#endif //	CYNG_NET_DOWNLOAD_H
