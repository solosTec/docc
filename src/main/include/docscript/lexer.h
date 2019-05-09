/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_LEXER_H
#define DOCSCRIPT_LEXER_H

#include <docscript/symbol.h>
#include <docscript/value.h>
#include <cyng/log/severity.h>
#include <stack>

namespace docscript
{
	class lexer
	{
	public:
		lexer(emit_value_f, std::function<void(cyng::logging::severity, std::string)>);

		/**
		 * process next symbol.
		 */
		void next(symbol sym);

	private:
		enum state
		{
			STATE_ERROR_,
			STATE_START_,
			STATE_ARG_,			//!<	test function arguments
			STATE_FUNCTION_,
		};	//state_;

		friend std::string get_state_name(state);

		/**
		 * state stack
		 */
		std::stack<state>	state_stack_;

	private:
		/**
		 * callback for complete values
		 */
		emit_value_f	emit_;
		std::function<void(cyng::logging::severity, std::string)>	err_;

		void state_start(symbol);
		void state_arg(symbol);
		void state_function(symbol);

		void emit(value_type, symbol);

		/**
		 * saves the current state and returns the new state
		 */
		state save(state);
		state substitute(state);

		/**
		 * Remove the state from the top.
		 *
		 * @return saved state
		 */
		state pop();

		/**
		 * @return saved state
		 */
		state top() const;

	};
}

#endif	
