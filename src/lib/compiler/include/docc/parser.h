/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_PARSER_H
#define DOCC_SCRIPT_PARSER_H

#include <docc/symbol.h>
#include <docc/nonterminal.h>
#include <docc/ast/root.h>

#include <cstdint>
#include <stack>

namespace docscript {

	class context;
	class parser
	{
	private:
		/**
		 * parser state represented by non-terminals
		 */
		class nt_stack : public std::stack<nonterminal, std::vector<nonterminal>> {
		public:
			void print(std::ostream&) const;
		} state_;
		friend std::ostream& operator<<(std::ostream& os, const nt_stack& sym);

		/**
		 * semantic stack, ast
		 */

	public:
		parser(context&);
		bool put(symbol const& sym);

	private:
		void eod();

		bool state_body(symbol const& sym);
		bool state_list(symbol const& sym);
		bool state_term(symbol const& sym);
		bool state_vector(symbol const& sym);
		bool state_map(symbol const& sym);
		bool state_tail(symbol const& sym);
		bool state_value(symbol const& sym);
		bool state_svm(symbol const& sym);
		bool state_params(symbol const& sym);
		bool state_terminal(symbol const& sym);

		/**
		 * @return true if a MAP is expected
		 */
		bool assess_method_type(std::string const&);
		/**
		 * @return true if inline method
		 */
		bool is_display_inline(std::string const&);

		/**
		 * build the program to generate the document(s)
		 */
		void build();

	private:
		context& ctx_;

		/**
		 * all produced semantic elements (ast)
		 */
		ast::program prg_;
	};
}
#endif //   DOCC_SCRIPT_PARSER_H