/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_TOKENIZER_H
#define DOCC_SCRIPT_TOKENIZER_H

#include <docc/token.h>
#include <docc/symbol.h>

#include <cstdint>
#include <functional>

namespace docscript {

	class tokenizer
	{
	private:
		enum class state : std::uint32_t
		{
			ERROR_,
			START_,
			COMMENT_,
			DOT_,
			FUNCTION_,	//!<	function name
			TIMESTAMP_,	//!<	starts with @
			COLOR_,		//!<	starts with #
			TYPE_,		//!<	<type>
			NUMBER_,	//!<	number
			INTEGER_,	//!<	unsigned integer
			FLOAT_,		//!<	floating point
			EXPONENT_,	//!<	specifies the magnitude of the number as a power of 10
			QUOTE_,		//!<	'...'
			QUOTE_ESC_,	//!<	'..\?..'	escape from QUOTE
			TEXT_,		//!<	anything else including numbers
			NEWLINE_,	//!<	multiple NL
			INCLUDE_,	//!<	include function
		}	state_;

		emit_symbol_f cb_;

		std::u32string value_;

	public:
		tokenizer(emit_symbol_f);

		/**
		 * @brief insert next token
		 *
		 * @param c
		 * @return true to advance
		 */
		[[nodiscard]] bool put(token const& tok, token const& look_ahead);

	private:
		[[nodiscard]] std::pair<state, bool> start(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> comment(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> dot(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> function(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> timestamp(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> color(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> type(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> number(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> integer(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> floating_point(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> exponent(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> quote(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> quote_esc(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> text(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> newline(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> include(token const& tok, token const& prev);

		void emit(symbol_type);

		void complete_ts();

	private:
		symbol_type prev_sym_type_;


	};
}
#endif //   DOCC_SCRIPT_TOKENIZER_H