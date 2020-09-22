/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_TOKENIZER_H
#define DOCSCRIPT_TOKENIZER_H

#include <docscript/token.h>
#include <docscript/symbol.h>
#include <cyng/log/severity.h>


namespace docscript
{
	class tokenizer
	{
	public:
		tokenizer(emit_symbol_f, std::function<void(cyng::logging::severity, std::string)>);

		/**
		 * process next token.
		 *
		 * @return false to reject token
		 */
		bool next(token tok);

		//
		//	support diagnostic and error message
		//
		void emit_current_file(std::string);
		void emit_current_line(std::size_t);

	private:
		enum class state
		{
			ERROR_,
			START_,
			DOT_,
			NUMBER_,		//	0 ... 9
			DATETIME_,	//	YYYY-MM-DD[THH:MM:SS]
			TOKEN_,		//	lowercase characters and '_'
			TEXT_,		//	text and punctuation
			QUOTE_,		//	'preserve all white spaces and dots'
			DETECT_,		//	detect . after special (entity) characters like ")" and "
		}	state_;

		friend std::string get_state_name(state);


		std::pair<state, bool> state_start(token);
		std::pair<state, bool> state_text(token);
		std::pair<state, bool> state_dot(token);
		std::pair<state, bool> state_token(token);
		std::pair<state, bool> state_number(token);
		std::pair<state, bool> state_datetime(token);
		std::pair<state, bool> state_quote(token);
               
        /*
         * Handle the case that dot "." follows an ")" or ".
         */
		std::pair<state, bool> state_detect(token);

		void emit(symbol&&) const;

		/**
		 * emit tmp_ as next symbol
		 */
		void emit(symbol_type);

		/**
		 * emit the symbol 1 .. n times
		 */
		void emit(symbol_type, token) const;

		void push(std::string);
		void push(token);
		void push(std::uint32_t, std::size_t);

	private:
		/**
		 * callback for complete tokens
		 */
		emit_symbol_f	emit_;
		std::function<void(cyng::logging::severity, std::string)>	err_;

		/**
		 * temporary buffer for next symbol
		 */
		std::u32string tmp_;

	};
}

#endif	
