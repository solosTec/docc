/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_GENERATOR_HTML_H
#define DOCSCRIPT_GENERATOR_HTML_H

#include <docscript/generator/generator.h>
#include <docscript/generator/numbering.h>

namespace docscript
{
	class gen_html : public generator
	{
	public:
		gen_html(std::vector< boost::filesystem::path > const&, bool body_only);

	private:
		/**
		 * register all build-in functions
		 */
		void register_this();

		void demo(cyng::context& ctx);

		virtual void print_symbol(cyng::context& ctx) override;
		virtual void print_hline(cyng::context& ctx) override;	//!<	ruler

		virtual void generate_file(cyng::context& ctx) override;
		virtual void generate_meta(cyng::context& ctx) override;

		virtual void convert_numeric(cyng::context& ctx) override;
		virtual void convert_alpha(cyng::context& ctx) override;

		virtual void paragraph(cyng::context& ctx) override;
		virtual void abstract(cyng::context& ctx) override;
		virtual void quote(cyng::context& ctx) override;
		virtual void list(cyng::context& ctx) override;
		virtual void link(cyng::context& ctx) override;
		virtual void figure(cyng::context& ctx) override;
		virtual void code(cyng::context& ctx) override;
		virtual void def(cyng::context& ctx) override;

		virtual void header(cyng::context& ctx) override;
		virtual void section(int, cyng::context& ctx) override;
		std::string create_section(std::size_t level, std::string tag, std::string title);
		virtual void make_footnote(cyng::context& ctx) override;

		virtual void format_italic(cyng::context& ctx) override;
		virtual void format_bold(cyng::context& ctx) override;
		virtual void format_color(cyng::context& ctx) override;
		virtual void format_sub(cyng::context& ctx) override;
		virtual void format_sup(cyng::context& ctx) override;

		std::ofstream& emit_file(std::ofstream&, cyng::vector_t::const_iterator, cyng::vector_t::const_iterator) const;
		std::ofstream& emit_doctype(std::ofstream&) const;
		std::ofstream& emit_head(std::ofstream&) const;
		std::ofstream& emit_meta(std::ofstream&) const;
		std::ofstream& emit_styles(std::ofstream&) const;
		std::ofstream& emit_body(std::ofstream&, cyng::vector_t::const_iterator, cyng::vector_t::const_iterator) const;
		std::ofstream& emit_body(std::ofstream&, cyng::object) const;
		std::ofstream& emit_footnotes(std::ofstream&) const;

	private:
		footnotes_t footnotes_;

		/**
		 * @brief generate only the HTML <body>...</body>
		 */
		bool const body_only_;
	};

	/**
	 * Substitute HTML entities
	 */
	std::string replace_html_entities(std::string const& str);

}

#endif
