/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_ASM_TOKENIZER_H
#define DOCC_ASM_TOKENIZER_H

#include <asm/token.h>
#include <asm/symbol.h>

#include <cstdint>
#include <functional>

namespace docasm {

	class tokenizer
	{
	public:
		//using callback = std::function<bool(symbol&&)>;

	private:
		enum class state : std::uint32_t
		{
			ERROR_,
			START,
			COMMENT,
			DOT,
			DIRECTIVE,	//!<	function name
			TIMESTAMP_,	//!<	starts with @
			COLOR,		//!<	starts with #
			NUMBER_,	//!<	number
			INTEGER_,	//!<	signed
			UNSIGNED_,	//!<	unsigned
			FLOAT_,		//!<	nnn.
			SIGN_,		//!<	[+|-]
			EXPONENT_,	//!<	scientific notation: n.nnn[+|-]En
			STRING_,	//!<	"..."
			STRING_ESC_,	//!<	"..\?."
			TYPE,		//!<	'type:value'
			VALUE,		//!<	'....:value'	TYPE trailer
			TEXT_,		//!<	anything else including numbers
			INSTRUCTION_,	//!<	instruction name
			NEWLINE_,	//!<	multiple NL
			//INCLUDE_,	//!<	include function
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
		[[nodiscard]] std::pair<state, bool> directive(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> timestamp(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> color(token const& tok, token const& prev);
		//[[nodiscard]] std::pair<state, bool> type(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> number(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> signed_int(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> unsigned_int(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> floating_point(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> sign(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> exponent(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> type(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> value(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> string(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> string_esc(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> text(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> newline(token const& tok, token const& prev);
		[[nodiscard]] std::pair<state, bool> instruction(token const& tok, token const& prev);
		//[[nodiscard]] std::pair<state, bool> include(token const& tok, token const& prev);

		void emit(symbol_type);

		void complete_ts();

	private:
		symbol_type prev_sym_type_;


	};
}
#endif //   DOCC_ASM_TOKENIZER_H