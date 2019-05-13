/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_GENERATOR_H
#define DOCSCRIPT_GENERATOR_H

#include <docscript/generator/numbering.h>

#include <cyng/intrinsics/sets.h>
#include <cyng/vm/controller.h>
#include <cyng/async/scheduler.h>

#include <boost/uuid/name_generator.hpp>
#include <boost/uuid/random_generator.hpp>

namespace docscript
{
	class generator
	{
	public:
		generator(std::vector< boost::filesystem::path > const&);
		void run(cyng::vector_t&&);

	protected:
		/**
		 * register all build-in functions
		 */
		void register_this();

		void init_meta_data(cyng::context& ctx);
		void meta(cyng::context& ctx);
		void var_set(cyng::context& ctx);
		void var_get(cyng::context& ctx);

		void generate_index(cyng::context& ctx);

		virtual void print_symbol(cyng::context& ctx);
		virtual void print_currency(cyng::context& ctx);
		virtual void print_hline(cyng::context& ctx) = 0;	//!<	ruler

		virtual void generate_file(cyng::context& ctx) = 0;
		virtual void generate_meta(cyng::context& ctx) = 0;
		virtual void convert_numeric(cyng::context& ctx) = 0;
		virtual void convert_alpha(cyng::context& ctx) = 0;

		virtual void paragraph(cyng::context& ctx) = 0;
		virtual void quote(cyng::context& ctx) = 0;
		virtual void list(cyng::context& ctx) = 0;
		virtual void link(cyng::context& ctx) = 0;
		virtual void figure(cyng::context& ctx) = 0;
		virtual void code(cyng::context& ctx) = 0;
		virtual void def(cyng::context& ctx) = 0;

		virtual void header(cyng::context& ctx) = 0;
		virtual void section(int, cyng::context& ctx) = 0;
		virtual void make_footnote(cyng::context& ctx) = 0;

		virtual void format_italic(cyng::context& ctx) = 0;
		virtual void format_bold(cyng::context& ctx) = 0;
		virtual void format_color(cyng::context& ctx) = 0;
		virtual void format_sub(cyng::context& ctx) = 0;
		virtual void format_sup(cyng::context& ctx) = 0;

		boost::filesystem::path resolve_path(std::string const& s) const;

	protected:
		boost::uuids::random_generator	uuid_gen_;	//	basic_random_generator<mt19937>
		boost::uuids::name_generator name_gen_;

		cyng::async::scheduler	scheduler_;
		cyng::controller vm_;

		/**
		 * dynamic values during "runtime"
		 */
		cyng::param_map_t	vars_;
		cyng::param_map_t	const_;

		std::string language_;	//!< document language

		std::vector< boost::filesystem::path > const includes_;
		numbering content_table_;

		/**
		 * meta data
		 */
		cyng::param_map_t meta_;

	};

	std::string get_extension(boost::filesystem::path const& p);

	std::string accumulate_plain_text(cyng::object);
	std::string accumulate_plain_text(cyng::tuple_t);
	std::string accumulate_plain_text(cyng::vector_t);

	void replace_all(std::string&, std::string, std::string);

	/**
	 * @brief generate_slug
	 * @param title
	 * @return cleaned up string that can be used in an URL or filesystem
	 */
	std::string generate_slug(std::string title);
}

#endif
