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
#include <symbol.h>

#include <cyng/obj/intrinsics/op.h>
#include <cyng/obj/intrinsics/color.hpp>

#include <string>
#include <iostream>
#include <variant>
#include <stack>
#include <chrono>
#include <map>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/string_generator.hpp>

namespace docscript {
	class context;
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- forward
		//	----------------------------------------------------------*
		//
		struct forward {

			boost::uuids::uuid const tag_;

			[[nodiscard]] static forward factory();

			friend std::ostream& operator<<(std::ostream& os, forward const& c);

			/**
			 * Generate a complete forward operation
			 */
			[[nodiscard]] forward finish(boost::uuids::uuid);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- jump
		//	----------------------------------------------------------*
		//
		struct jump {

			cyng::op const code_;
			std::string const label_;

			[[nodiscard]] static jump factory(cyng::op);

			friend std::ostream& operator<<(std::ostream& os, jump const& c);

			/**
			 * Generate a complete forward operation
			 */
			[[nodiscard]] jump finish(std::string const&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};


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