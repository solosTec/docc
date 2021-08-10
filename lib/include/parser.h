/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_PARSER_H
#define DOCC_SCRIPT_PARSER_H

#include <symbol.h>

#include <cstdint>
#include <stack>

namespace docscript {

	class context;
	class parser
	{
	private:
		enum class state
		{
			INITIAL_,
			FUNCTION_,
			OPEN_,	//!<	opening bracket
			PARAGRAPH_,
		};

		struct parameter {
			state const state_;
			std::vector<symbol>	symbols_;
			parameter(state);
			parameter(state, symbol const&);
		};
		std::stack<parameter> state_;

	public:
		parser(context&);
		void put(symbol const& sym);

	private:
		void state_initial(symbol const& sym);
		void state_function(symbol const& sym);
		void state_open(symbol const& sym);
		void state_paragraph(symbol const& sym);

		void emit_function();

	private:
		context& ctx_;
		symbol prev_;
	};
}
#endif //   DOCC_SCRIPT_PARSER_H