/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_RT_HTML_CONTROLLER_H
#define DOCSCRIPT_RT_HTML_CONTROLLER_H

#include <asm/reader.h>
#include <docc/context.h>
#include <rt/toc.h>

#include <cyng/obj/intrinsics/container.h>
#include <cyng/obj/object.h>

#include <filesystem>

#include <boost/uuid/uuid.hpp>

namespace docruntime {

    class controller {
      public:
        controller(
            std::filesystem::path out,
            std::vector<std::filesystem::path> inc,
            std::filesystem::path const &tmp_asm,
            std::filesystem::path const &tmp_html,
            int verbose);

        int
        run(std::filesystem::path &&inp,
            std::size_t pool_size,
            boost::uuids::uuid tag,
            bool generate_body_only,
            bool generate_meta,
            std::string const &index_file,
            std::string type);

      private:
        void emit_header(cyng::param_map_t &meta);

      private:
        std::ofstream ofs_;
        std::ofstream tmp_html_;
        std::filesystem::path const tmp_html_path_;
        docscript::context ctx_;
        docasm::reader assembler_;
    };

    void emit_styles(std::size_t depth, std::ostream &ofs);

    /**
     * @see https://katex.org/
     */
    void emit_katex(std::size_t depth, std::ostream &ofs);

} // namespace docruntime

#endif
