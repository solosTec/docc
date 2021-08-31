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
		//enum class state
		//{
		//	INITIAL_,
		//	FUNCTION_,
		//	OPEN_,	//!<	opening bracket
		//	PARAGRAPH_,
		//};

		//struct parameter {
		//	state const state_;
		//	std::vector<symbol>	symbols_;
		//	parameter(state);
		//	parameter(state, symbol const&);
		//};
		std::stack<symbol> state_;

	public:
		parser(context&);
		void put(symbol const& sym);

	private:
		void next_symbol(symbol const& sym);
		void next_function(symbol const& sym);
		void next_text(symbol const& sym);
		void eod();

		//void emit_function();

		/**
		 * replace current state with this new one
		 */
		//void exchange(state, symbol sym);


	private:
		context& ctx_;
		symbol prev_;
	};
}
#endif //   DOCC_SCRIPT_PARSER_H