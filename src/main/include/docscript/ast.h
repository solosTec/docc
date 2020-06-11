/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_AST_H
#define DOCSCRIPT_AST_H

#include <docscript/node.h>
#include <cyng/intrinsics/sets.h>
#include <cyng/compatibility/file_system.hpp>

namespace docscript
{

	/**
	 * Parse tree of an docscript document
	 */
	class ast
	{
	public:
		ast(bool log);

		/**
		 * get root list
		 */
		node::d_args* get_node_root();
		node::d_args const* get_node_root() const;

		/**
		 * generate code from parse tree
		 */
		cyng::vector_t generate(cyng::filesystem::path out, bool meta, bool index) const;

		/**
		 * generate HTML
		 */
		void generate_html(std::ostream&, bool) const;

	private:
		cyng::vector_t generate(node::d_args const*, cyng::filesystem::path out, bool meta, bool index) const;
		cyng::vector_t generate(std::size_t depth, node const&) const;
		cyng::vector_t generate_paragraph(std::size_t depth, node::d_args const*) const;
		cyng::vector_t generate_content(std::size_t depth, node::d_args const*) const;
		cyng::vector_t generate_function_par(std::size_t depth, node::p_args const*, std::string) const;
		cyng::vector_t generate_function_vec(std::size_t depth, node::v_args const*, std::string) const;
		cyng::vector_t generate_symbol(std::size_t depth, symbol const*) const;
		cyng::vector_t generate_list(std::size_t depth, node::s_args const*) const;
		cyng::vector_t generate_vector(std::size_t depth, node::v_args const*) const;

		void generate_html(node::d_args const*, std::ostream&, bool) const;
		void generate_html(std::size_t depth, node const&, std::ostream&, bool) const;
		void generate_html_paragraph(std::size_t depth, node::d_args const*, std::ostream&, bool) const;
		void generate_html_content(std::size_t depth, node::d_args const*, std::ostream&, bool) const;
		void generate_html_function_par(std::size_t depth, node::p_args const*, std::string, std::ostream&, bool) const;
		void generate_html_function_vec(std::size_t depth, node::v_args const*, std::string, std::ostream&, bool) const;
		void generate_html_symbol(std::size_t depth, symbol const&, std::ostream&, bool) const;
		void generate_html_list(std::size_t depth, node::s_args const*, std::ostream&, bool) const;
		void generate_html_vector(std::size_t depth, node::v_args const*, std::ostream&, bool) const;

		bool verify_param_range(std::string const& cmd, std::string const& name, node const& val) const;

	private:
		bool const log_;	//!< logging on/off
		node	root_;

		static const std::string color_green_;
		static const std::string color_blue_;
		static const std::string color_brown_;
		static const std::string color_red_;
		static const std::string color_grey_;
		static const std::string end_;

	};

	bool is_valid_list_style(std::string);
}

#endif
