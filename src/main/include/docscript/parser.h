/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_COMPILER_H
#define DOCSCRIPT_COMPILER_H

#include <docscript/symbol.h>
#include <docscript/ast.h>

#include <cyng/log/severity.h>

#include <boost/filesystem.hpp>

namespace docscript
{
	class parser
	{
	public:
		parser(typename symbol_reader::symbol_list_t&, int verbosity);


		/**
		 * Read the specified range and generate
		 * the parse tree (AST)
		 */
		void parse();

		/**
		 * @return (immutable) parse tree
		 */
		ast const& get_ast() const;

	private:
		symbol_reader producer_;

		void loop(std::size_t depth, node::d_args*);

		node generate_function(std::size_t);
		void generate_arg(std::size_t, node::p_args*);
		void generate_arg(std::size_t, node::v_args*);
		std::size_t generate_list(std::size_t, node::p_args*);
		std::size_t generate_list(std::size_t, node::v_args*);
		std::pair<std::string, node> generate_parameter(std::size_t);
		//void generate_key(std::size_t, std::string);
		node generate_quote();
		node generate_paragraph(std::size_t, node::d_args& args);
		node generate_content(std::size_t);	//!< compare to paragraph but enclosed in ()
		node generate_vector(std::size_t);

		/**
		 * produce next / look ahead symbol
		 */
		bool match(symbol_type);
		void match();

		void print_error(cyng::logging::severity level, std::string msg);

	private:
		ast ast_;	//!< parse tree
	};
}

#endif	
