/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOC2SITE_INITIALIZE_H
#define DOC2SITE_INITIALIZE_H

#include <cyng/obj/intrinsics/container.h>

#include <chrono>
#include <filesystem>
#include <vector>

namespace docscript {

    class initialize {
      public:
        initialize(std::vector<std::filesystem::path> inc, int verbose);

        int run(std::filesystem::path working_dir);

      private:
        cyng::tuple_t generate_control(std::tm const &tm) const;
        cyng::tuple_t generate_page_home() const;
        cyng::tuple_t generate_navbar_main() const;
        cyng::tuple_t generate_footer_main(std::tm const &tm) const;
        cyng::tuple_t generate_downloads() const;

        void generate_logo(std::filesystem::path);

        void download_bootstrap(std::filesystem::path, std::string v);

      private:
        std::vector<std::filesystem::path> const inc_;
        int const verbose_;
        // boost::asio::io_context ioc_;
        // boost::asio::ssl::context ctx_; // {boost::asio::ssl::context::tlsv12_client};
    };
} // namespace docscript
#endif
