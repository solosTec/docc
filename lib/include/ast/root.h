/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_ROOT_H
#define DOCC_SCRIPT_AST_ROOT_H

#include <cstdint>
#include <optional>
#include <iostream>
#include <string>
#include <memory>
#include <variant>
#include <stack>

#include <fmt/ostream.h>

#include <method.h>
#include <symbol.h>


#include <ast/constant.h>
#include <ast/value.h>
#include <ast/vlist.h>
#include <ast/params.h>
#include <ast/method.h>


namespace docscript {

	class context;

	namespace ast {

		//struct constant;
		//class value;
		//class map_method;
		//class vec_method;







		using decl_t = std::variant<
			constant,	//	[0]
			value,		//	[1]
			param,		//	[2]
			map_method,	//	[3]
			vec_method	//	[4]
		>;

		class program {
		public:
			program(context&);
			/**
			 * Move current term to program
			 */
			void finalize_param(symbol const& sym);

			/**
			 * initialize top term with a function
			 * @return false if function is not defined
			 */
			bool init_function(std::string const&);
			void init_paragraph(std::string const&);
			void init_param(symbol const& sym);

			/**
			 * reduce stack
			 * @return number of produced asts.
			 */
			std::size_t merge();

			/**
			 * append to value list (vector)
			 */
			bool append(symbol const& sym);
			void append(param&&);

			/**
			 * generate the assembler code
			 */
			void generate();

		private:
			/**
			 * @return top element of semantic stack
			 */
			decl_t& top();

			/**
			 * transfer current ast to program
			 */
			void transfer_ast();

			/**
			 * move top element from semantic AST to 
			 * the bottom.
			 */
			void merge_ast();
			void merge_ast_param();
			void merge_ast_map_method();
			void merge_ast_vec_method();
			void merge_ast_value(value&&);

		private:
			context& ctx_;

			/**
			 * semantic stack
			 */
			std::stack<decl_t> semantic_stack_;

			/**
			 * the complete programm
			 */
			std::vector<decl_t> decls_;
		};

	}

}

#endif