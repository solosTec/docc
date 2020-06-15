/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_GENERATOR_ASCIIDOC_H
#define DOCSCRIPT_GENERATOR_ASCIIDOC_H

#include <docscript/generator/generator.h>

namespace docscript
{
	class gen_asciidoc : public generator
	{
	public:
		gen_asciidoc(std::vector< cyng::filesystem::path > const&);

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
		virtual void abstract(cyng::context& ctx) override;
		virtual void quote(cyng::context& ctx) override;
		virtual void list(cyng::context& ctx) override;
		virtual void link(cyng::context& ctx) override;
		virtual void figure(cyng::context& ctx) override;
		virtual void gallery(cyng::context& ctx) override;
		virtual void code(cyng::context& ctx) override;
		virtual void def(cyng::context& ctx) override;
		virtual void annotation(cyng::context& ctx) override;
		virtual void table(cyng::context& ctx) override;
		virtual void alert(cyng::context& ctx) override;

		virtual void header(cyng::context& ctx) override;
		virtual void section(int, cyng::context& ctx) override;
		virtual void make_footnote(cyng::context& ctx) override;
		virtual void make_ref(cyng::context& ctx) override;
		virtual void make_tok(cyng::context& ctx) override;

		virtual void format_italic(cyng::context& ctx) override;
		virtual void format_bold(cyng::context& ctx) override;
		virtual void format_tt(cyng::context& ctx) override;
		virtual void format_color(cyng::context& ctx) override;
		virtual void format_sub(cyng::context& ctx) override;
		virtual void format_sup(cyng::context& ctx) override;
		virtual void format_mark(cyng::context& ctx) override;

		std::ofstream& emit_file(std::ofstream&, cyng::vector_t::const_iterator, cyng::vector_t::const_iterator) const;
		std::ofstream& emit_meta(std::ofstream&) const;
		std::ofstream& emit_obj(std::ofstream&, cyng::object) const;

	private:
		//footnotes_t footnotes_;
	};

	/**
	 * Substitute markdown entities
	 */
	std::string replace_asciidoc_entities(std::string const& str);

}

#endif
