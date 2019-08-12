/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_GENERATOR_LATEX_H
#define DOCSCRIPT_GENERATOR_LATEX_H

#include <docscript/generator/generator.h>
#include <boost/regex/pending/unicode_iterator.hpp>

namespace docscript
{
	class gen_latex : public generator
	{
	public:
		gen_latex(std::vector< boost::filesystem::path > const&);

	private:
		/**
		 * register all build-in functions
		 */
		void register_this();

		void demo(cyng::context& ctx);

		virtual void generate_file(cyng::context& ctx) override;
		virtual void generate_meta(cyng::context& ctx) override;
		virtual void convert_numeric(cyng::context& ctx) override;
		virtual void convert_alpha(cyng::context& ctx) override;

		virtual void print_symbol(cyng::context& ctx) override;
		virtual void print_currency(cyng::context& ctx) override;
		virtual void print_hline(cyng::context& ctx) override;	//!<	ruler

		virtual void paragraph(cyng::context& ctx) override;
		virtual void quote(cyng::context& ctx) override;
		virtual void abstract(cyng::context& ctx) override;
		virtual void list(cyng::context& ctx) override;
		virtual void link(cyng::context& ctx) override;
		virtual void figure(cyng::context& ctx) override;
		virtual void code(cyng::context& ctx) override;
		virtual void def(cyng::context& ctx) override;
		virtual void annotation(cyng::context& ctx) override;
		virtual void table(cyng::context& ctx) override;

		virtual void header(cyng::context& ctx) override;
		virtual void section(int, cyng::context& ctx) override;
		virtual void make_footnote(cyng::context& ctx) override;
		virtual void make_ref(cyng::context& ctx) override;

		virtual void format_italic(cyng::context& ctx) override;
		virtual void format_bold(cyng::context& ctx) override;
		virtual void format_color(cyng::context& ctx) override;
		virtual void format_sub(cyng::context& ctx) override;
		virtual void format_sup(cyng::context& ctx) override;

		std::ofstream& emit_file(std::ofstream&, cyng::vector_t::const_iterator, cyng::vector_t::const_iterator) const;
		std::ofstream& emit_class(std::ofstream&) const;
		std::ofstream& emit_title(std::ofstream&) const;
		std::ofstream& emit_document(std::ofstream&, cyng::vector_t::const_iterator, cyng::vector_t::const_iterator) const;
		std::ofstream& emit_document(std::ofstream&, cyng::object) const;

		std::string create_section(std::size_t level, std::string tag, std::string title);

		/**
		 * get language from meta data and select
		 * a matching babel language.
		 */
		std::string get_babel_language() const;
	};

	/**
	 * build a LaTeX command string with:
	 * \cmd{param}
	 */
	std::string build_cmd(std::string cmd, std::string param);

	/**
	 * \cmd[attr]{param}
	 */
	std::string build_cmd(std::string cmd, std::string param, std::string attr);

	/**
	 * \cmd{param}[attr]
	 */
	std::string build_cmd_alt(std::string cmd, std::string attr, std::string param);

	/**
	 * Substitute LaTeX entities
	 */
	std::string replace_latex_entities(std::string const& str);
	std::string replace_unicode(boost::u8_to_u32_iterator<std::string::const_iterator>, boost::u8_to_u32_iterator<std::string::const_iterator>);

	/**
	 * @return true if language is supported by listings package
	 */
	bool is_language_supported(std::string);
	std::string cleanup_language(std::string const&);
}

#endif
