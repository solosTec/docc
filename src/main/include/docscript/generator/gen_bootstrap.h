/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_GENERATOR_BOOTSTRAP_H
#define DOCSCRIPT_GENERATOR_BOOTSTRAP_H

#include <docscript/generator/generator.h>
#include <docscript/generator/numbering.h>
#include <html/dom.hpp>

namespace docscript
{
	class gen_bootstrap : public generator
	{
	public:
		gen_bootstrap(std::vector< cyng::filesystem::path > const&);

	private:
		/**
		 * register all build-in functions
		 */
		void register_this();

		void demo(cyng::context& ctx);
		void card_deck(cyng::context& ctx);

		virtual void print_symbol(cyng::context& ctx) override;
		virtual void print_currency(cyng::context& ctx) override;
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
		virtual void gallery(cyng::context& ctx) override;
		virtual void code(cyng::context& ctx) override;
		virtual void def(cyng::context& ctx) override;
		virtual void annotation(cyng::context& ctx) override;
		virtual void table(cyng::context& ctx) override;
		virtual void alert(cyng::context& ctx) override;

		virtual void header(cyng::context& ctx) override;
		virtual void section(int, cyng::context& ctx) override;
		std::string create_section(std::size_t level, std::string tag, std::string title);
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
		std::ofstream& emit_body(std::ofstream&, cyng::vector_t::const_iterator, cyng::vector_t::const_iterator) const;
		std::ofstream& emit_body(std::ofstream&, cyng::object) const;
		std::ofstream& emit_footnotes(std::ofstream&) const;
		std::ofstream& emit_intrinsic(std::ofstream&, boost::uuids::uuid) const;
		std::ofstream& emit_toc(std::ofstream&, std::size_t) const;
		std::ofstream& emit_toc(std::ofstream&, cyng::vector_t const&, std::size_t, std::size_t) const;

		std::string compute_fig_title(boost::uuids::uuid tag, std::string caption);
		std::string compute_tbl_title(boost::uuids::uuid tag, std::string caption);

	private:

		footnotes_t footnotes_;
		figures_t figures_;
		tables_t tables_;

		const static std::string icon_info_;
		const static std::string icon_warning_;
		const static std::string icon_caution_;
	};

}

#endif
