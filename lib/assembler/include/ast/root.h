/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_ROOT_H
#define DOCC_ASM_AST_ROOT_H

#include <ast/label.h>
#include <ast/literal.h>
#include <ast/operation.h>
#include <ast/value.h>
#include <ast/push.h>
#include <ast/invoke.h>
#include <ast/forward.h>
#include <ast/jump.h>
#include <symbol.h>

#include <stack>

#include <boost/uuid/string_generator.hpp>

namespace docasm {
	class context;
	namespace ast {

		using decl_t = std::variant<
			label,		//	[0]
			operation,	//	[1]
			push,		//	[2]
			invoke,		//	[3]
			invoke_r,	//	[4]
			forward,	//	[5]
			jump,		//	[6]
			literal,	//	[7]
			value		//	[8]
		>;

		class program {
		public:
			program(context&);

			/**
			 * Move current term to program
			 */
			void finalize_statement();
			void finalize_statement(symbol const&);

			void init_label(std::string const&);
			void init_op(cyng::op);
			void init_literal(std::string const&);

			ast::label_list_t build_label_list();
			void generate(ast::label_list_t const&);

		private:
			context& ctx_;

			/**
			 * semantic stack
			 */
			std::stack<decl_t> semantic_stack_;

			/**
			 * the complete programm
			 */
			std::vector<decl_t> asts_;

			boost::uuids::string_generator uuidgen_;

		};
	}
}

#endif