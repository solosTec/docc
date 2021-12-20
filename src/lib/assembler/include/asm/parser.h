/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_PARSER_H
#define DOCC_ASM_PARSER_H

#include <asm/symbol.h>
#include <asm/nonterminal.h>
#include <asm/ast/root.h>

#include <cyng/obj/intrinsics/op.h>

#include <stack>
#include <map>


namespace docasm {

	class context;
	class parser
    {
		/**
		 * parser state represented by non-terminals
		 */
		class nt_stack : public std::stack<nonterminal, std::vector<nonterminal>> {
		public:
			void print(std::ostream&) const;
		} state_;
		friend std::ostream& operator<<(std::ostream& os, const nt_stack& sym);

	public:
		parser(context&);
		bool put(symbol const& sym);

	private:
		void eod();
		bool state_body(symbol const& sym);
		bool state_program(symbol const& sym);
		bool state_line(symbol const& sym);
		bool state_eol(symbol const& sym);
		bool state_ident(symbol const& sym);
		bool state_value(symbol const& sym);
		bool state_terminal(symbol const& sym);
		bool state_type(symbol const& sym);

		void statement(symbol const& sym);


	private:
		context& ctx_;
		/**
		 * all produced semantic elements (ast)
		 */
		ast::program prg_;

		static const std::map<std::string, cyng::op> ops_;
	};

}

#endif