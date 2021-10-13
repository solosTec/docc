/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_ROOT_H
#define DOCC_ASM_AST_ROOT_H

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

		using label_list_t = std::map<std::string, std::size_t>;

		//
		//	----------------------------------------------------------*
		//	-- label
		//	----------------------------------------------------------*
		//
		struct label {
			std::string const name_;

			[[nodiscard]] static label factory(std::string const&);

			friend std::ostream& operator<<(std::ostream& os, label const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- literal
		//	----------------------------------------------------------*
		//
		struct literal {
			std::string const value_;

			[[nodiscard]] static literal factory(std::string const&);

			friend std::ostream& operator<<(std::ostream& os, literal const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- operation
		//	----------------------------------------------------------*
		//
		struct operation {
			cyng::op const code_;

			[[nodiscard]] static operation factory(cyng::op);

			friend std::ostream& operator<<(std::ostream& os, operation const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- value
		//	----------------------------------------------------------*
		//
		struct value {
			std::variant<
				std::monostate,		//	[0] empty
				std::string,		//	[1] txt
				std::chrono::system_clock::time_point,	//	[2]
				cyng::color_8,		//	[3]
				bool,				//	[4]
				std::uint64_t,		//	[5]
				boost::uuids::uuid	//	[6]
			> val_;

			[[nodiscard]] static value factory(symbol const&);
			[[nodiscard]] static value factory(boost::uuids::uuid);

			friend std::ostream& operator<<(std::ostream& os, value const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- push
		//	----------------------------------------------------------*
		//
		struct push {

			value const val_;

			[[nodiscard]] static push factory();

			friend std::ostream& operator<<(std::ostream& os, push const& c);

			/**
			 * Generate a complete push operation
			 */
			[[nodiscard]] push finish(value&&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- invoke
		//	----------------------------------------------------------*
		//
		struct invoke {

			std::string const name_;

			[[nodiscard]] static invoke factory();

			friend std::ostream& operator<<(std::ostream& os, invoke const& c);

			/**
			 * Generate a complete invoke operation
			 */
			[[nodiscard]] invoke finish(std::string const&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- invoke_r
		//	----------------------------------------------------------*
		//
		struct invoke_r {

			std::string const name_;

			[[nodiscard]] static invoke_r factory();

			friend std::ostream& operator<<(std::ostream& os, invoke_r const& c);

			/**
			 * Generate a complete invoke operation
			 */
			[[nodiscard]] invoke_r finish(std::string const&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

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