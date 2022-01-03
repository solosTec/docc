/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Sylko Olzscher
 *
 */

#ifndef DOC2SITE_GENERATOR_H
#define DOC2SITE_GENERATOR_H

#include <docc/context.h>
#include <page.h>
#include <footer.h>
#include <navbar.h>
#include <rt/toc.h>

#include <cyng/obj/intrinsics/container.h>
#include <cyng/obj/object.h>
#include <cyng/vm/mesh.h>
#include <cyng/vm/vm.h>
#include <cyng/io/iso_639_1.h>

#include <filesystem>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/name_generator.hpp>

namespace docscript {

	class generator {
	public:
		generator(std::istream&
			, std::ostream&
			, docscript::context&
			, cyng::mesh& fabric
			, boost::uuids::uuid tag
			, std::string const& name
			, page const& cfg_page
			, navbar const& cfg_navbar
			, footer const& cfg_footer
			, std::string const& locale
			, std::string const& country
			, std::string const& language
			, std::string const& encoding);

		int run();

		cyng::param_map_t& get_vars();
		cyng::param_map_t& get_meta();

		std::string get_language() const;

	private:
		std::string esc(cyng::vector_t);
		std::string quote(cyng::vector_t);
		std::string italic(cyng::vector_t);
		std::string bold(cyng::vector_t);
		std::string typewriter(cyng::vector_t);
		void paragraph(cyng::vector_t);

		void label(cyng::vector_t);
		std::string ref(cyng::vector_t);
		void h1(cyng::vector_t);
		void h2(cyng::vector_t);
		void h3(cyng::vector_t);
		void h4(cyng::vector_t);
		void h5(cyng::vector_t);
		void h6(cyng::vector_t);
		void header(cyng::param_map_t);

		void figure(cyng::param_map_t);
		void embed_svg(std::string const& caption, std::string const& alt, boost::uuids::uuid tag, std::filesystem::path const& source, double scale);
		void embed_base64(std::string const& caption
			, std::string const& alt
			, boost::uuids::uuid tag
			, std::filesystem::path const& source
			, std::string const& ext
			, double scale);

		void table_of_content(cyng::param_map_t);

		void code(cyng::param_map_t);
		void tree(cyng::param_map_t);
		void table(cyng::param_map_t);

		void resource(cyng::param_map_t);
		std::chrono::system_clock::time_point now(cyng::param_map_t);
		boost::uuids::uuid uuid(cyng::param_map_t);

		void set(cyng::param_map_t);
		void meta(cyng::param_map_t);
		cyng::vector_t get(cyng::vector_t);
		cyng::vector_t range(cyng::vector_t);
		std::string fuse(cyng::vector_t);
		std::string cat(cyng::param_map_t pm);
		std::string repeat(cyng::param_map_t pm);
		std::string currency(cyng::param_map_t pm);

		void emit_header(std::size_t level, boost::uuids::uuid tag, std::string const& num, std::string const& title);
		void emit_toc(cyng::vector_t, std::size_t depth);


	private:
		std::function<std::string(cyng::vector_t)> f_esc();
		std::function<std::string(cyng::vector_t)> f_quote();
		std::function<void(cyng::param_map_t)> f_set();
		std::function<cyng::vector_t(cyng::vector_t)> f_get();
		std::function<void(cyng::param_map_t)> f_meta();
		std::function<void(cyng::vector_t)> f_paragraph();
		std::function<std::string(cyng::vector_t)> f_italic();
		std::function<std::string(cyng::vector_t)> f_bold();
		std::function<std::string(cyng::vector_t)> f_typewriter();
		// std::function<std::string(cyng::vector_t)> f_number();
		std::function<void(cyng::vector_t)> f_label();
		std::function<std::string(cyng::vector_t)> f_ref();
		std::function<void(cyng::vector_t)> f_h1();
		std::function<void(cyng::vector_t)> f_h2();
		std::function<void(cyng::vector_t)> f_h3();
		std::function<void(cyng::vector_t)> f_h4();
		std::function<void(cyng::vector_t)> f_h5();
		std::function<void(cyng::vector_t)> f_h6();
		std::function<void(cyng::param_map_t)> f_header();
		std::function<void(cyng::param_map_t)> f_figure();
		std::function<void(cyng::param_map_t)> f_toc();
		std::function<void(cyng::param_map_t)> f_resource();
		std::function<std::chrono::system_clock::time_point(cyng::param_map_t)> f_now();
		std::function<boost::uuids::uuid(cyng::param_map_t)> f_uuid();
		std::function<cyng::vector_t(cyng::vector_t)> f_range();
		std::function<std::string(cyng::vector_t)> f_fuse();
		std::function<std::string(cyng::param_map_t)> f_cat();
		std::function<std::string(cyng::param_map_t pm)> f_repeat();
		std::function<std::string(cyng::param_map_t)> f_currency();
		//std::function<void(std::string)> f_show();
		std::function<void(cyng::param_map_t)> f_code();
		std::function<void(cyng::param_map_t)> f_tree();
		std::function<void(cyng::param_map_t)> f_table();

		std::string compute_title_figure(boost::uuids::uuid tag, std::string caption);
		std::string compute_title_table(boost::uuids::uuid tag, std::string caption);
		cyng::io::language_codes get_language_code() const;

		void emit_navbar(std::ostream&, navbar const&, page const&);
		void emit_svg(std::ostream&);
		void emit_footer(std::ostream&, footer const&);

	private:
		std::istream& is_;
		std::ostream& os_;
		//std::string const& index_file_;
		cyng::param_map_t vars_;
		cyng::param_map_t meta_;
		docruntime::toc toc_;
		//footnotes_t footnotes_;
		//figures_t figures_;
		//tables_t tables_;
		boost::uuids::random_generator uuid_gen_; //	basic_random_generator<mt19937>
		boost::uuids::name_generator_latest name_gen_;	//	basic_name_generator<detail::sha1>
		cyng::vm_proxy vm_;
		docscript::context& ctx_;
		std::string const& page_name_;
		page const& cfg_page_;
		navbar const& cfg_navbar_;
		footer const& cfg_footer_;
	};


}

#endif
