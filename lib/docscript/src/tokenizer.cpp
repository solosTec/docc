/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <docscript/tokenizer.h>
#include <boost/regex/pending/unicode_iterator.hpp>

namespace docscript
{
	tokenizer::tokenizer(emit_symbol_f f, std::function<void(cyng::logging::severity, std::string)> err)
		: state_(STATE_START_)
		, emit_(f)
		, err_(err)
		, tmp_()
	{}

	bool tokenizer::next(token tok)
	{
		bool advance = true;

		//std::cout << "--- " << tok << std::endl;

		switch (state_) {
		case STATE_START_:
			std::tie(state_, advance) = state_start(tok);
			break;
		case STATE_TEXT_:
			std::tie(state_, advance) = state_text(tok);
			break;
		case STATE_DOT_:
			std::tie(state_, advance) = state_dot(tok);
			break;
		case STATE_TOKEN_:
			std::tie(state_, advance) = state_token(tok);
			break;
		case STATE_NUMBER_:
			std::tie(state_, advance) = state_number(tok);
			break;
		case STATE_QUOTE_:
			std::tie(state_, advance) = state_quote(tok);
			break;
		case STATE_DETECT_:
			std::tie(state_, advance) = state_detect(tok);
			break;
		default:
			err_(cyng::logging::severity::LEVEL_FATAL, get_state_name(state_));
			break;
		}

		return advance;
	}

	std::pair<tokenizer::state, bool> tokenizer::state_start(token tok)
	{
		if (tok.eof_)	return std::make_pair(state_, true);

		switch (tok.value_) {

		case '\n':
			//	emit a pilcrow
			if (tok.count_ > 1)	emit(symbol(SYM_PAR, u8"¶"));
			return std::make_pair(state_, true);

		case ' ': case '\t':
			//	ommit white spaces
			return std::make_pair(state_, true);

		case '.':
			//	'.' is the escape symbol
			if (tok.count_ > 1u)	push(tok.value_, tok.count_ / 2u);
			emit(SYM_TEXT);
			return ((tok.count_ % 2) == 0)
				? std::make_pair(state_, true)
				: std::make_pair(STATE_DOT_, true)
				;

		case '"':
			emit(SYM_DQUOTE, tok);
			return std::make_pair(STATE_DETECT_, true);

		case '\'':
			if (tok.count_ == 1u)	return std::make_pair(STATE_QUOTE_, true);
			//
			//	'' => '
			//
			push(tok.value_, tok.count_ / 2u);
			emit(SYM_TEXT);

			//	
			//	if the count of ' characters is odd restart with 
			//	lst one
			//
			return ((tok.count_ % 2) == 0)
				? std::make_pair(state_, true)
				: std::make_pair(state_, false)	//	repeat
				;

		case '(':
			emit(SYM_OPEN, tok);
			return std::make_pair(state_, true);

		case ')':
			emit(SYM_CLOSE, tok);
			return std::make_pair(STATE_DETECT_, true);

		case ',':
			emit(SYM_SEP, tok);
			return std::make_pair(state_, true);

		case ':':
			emit(SYM_KEY, tok);
			return std::make_pair(state_, true);

		case '[':
			emit(SYM_BEGIN, tok);
			return std::make_pair(state_, true);

		case ']':
			emit(SYM_END, tok);
			return std::make_pair(state_, true);

		case '!': case '?': case ';':
			push(tok);
			emit(SYM_TEXT);
			return std::make_pair(state_, true);

		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			return std::make_pair(STATE_NUMBER_, false);

		default:
			break;
		}

		return std::make_pair(STATE_TEXT_, false);
	}

	std::pair<tokenizer::state, bool> tokenizer::state_text(token tok)
	{
		switch (tok.value_) {

		case '\n':
			if (tok.count_ > 1) {
				emit(SYM_TEXT);
				emit(symbol(SYM_PAR, u8"¶"));
				return std::make_pair(STATE_START_, true);
			}

			//
			//	fall through
			//

		case ' ': case '\t':
			emit(SYM_TEXT);
			return std::make_pair(STATE_START_, true);

		case '"':
			//	terminate word and handle " as single character
			emit(SYM_TEXT);
			return std::make_pair(STATE_START_, false);

		case '(': case ')':
		case '[': case ']':
		case '?': case '!':
		case ',': case ':': case ';':
			//
			//	characters that terminate a word
			//
			emit(SYM_TEXT);
			return std::make_pair(STATE_START_, false);

		case '.':
			emit(SYM_TEXT);
			push(tok);	//	take "." as text
			emit(SYM_TEXT);
			//
			//	At the end of a word . cannot be used as escape symbol
			//
			return std::make_pair(STATE_START_, true);

		case '\'':
			//	don't start a quotes section here, when "'" is inside a word,
			//	like in "it's" or "don't".
			//	idea: To make that more clear an additional tokenizer state
			//	could help to indicate a following non-white character.
		default:
			push(tok);
			break;
		}
		return std::make_pair(state_, true);
	}

	std::pair<tokenizer::state, bool> tokenizer::state_dot(token tok)
	{
		switch (tok.value_) {

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case '_':
			push(tok);
			return std::make_pair(STATE_TOKEN_, true);

		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			//	convert ".n" to "0.n"
			push("0.");
			push(tok);
			return std::make_pair(STATE_NUMBER_, true);

		case ' ':
			//	emit the "." as text
			push(".");
			emit(SYM_TEXT);
			break;

		default:
			//	'.' acts like an escape symbol:
			//	'.' + any CHAR == any char 
			push(tok);
			emit(SYM_TEXT);
			break;
		}

		return std::make_pair(STATE_START_, true);
	}

	std::pair<tokenizer::state, bool> tokenizer::state_token(token tok)
	{
		switch (tok.value_) {

		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case '_': //case '.':
			push(tok);
			break;
			
		case '\n':
			emit(SYM_TOKEN);
			if (tok.count_ > 1)	emit(symbol(SYM_PAR, u8"¶"));
			return std::make_pair(STATE_START_, true);

		case ' ': case '\t':
			emit(SYM_TOKEN);
			return std::make_pair(STATE_START_, true);

		default:
			emit(SYM_TOKEN);
			return std::make_pair(STATE_START_, false);
		}
		return std::make_pair(state_, true);
	}

	std::pair<tokenizer::state, bool> tokenizer::state_number(token tok)
	{
		switch (tok.value_) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case '.':
			push(tok);
			break;

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case '_':
			push(tok);
			return std::make_pair(STATE_TEXT_, true);

		case '\n':
			emit(SYM_NUMBER);
			if (tok.count_ > 1)	emit(symbol(SYM_PAR, u8"¶"));
			return std::make_pair(STATE_START_, true);

		case ' ': case '\t':
			emit(SYM_NUMBER);
			return std::make_pair(STATE_START_, true);

		//case '.':
		//	emit(SYM_NUMBER);
		//	//	test "." for escape symbol
		//	return std::make_pair(STATE_START_, false);

		default:
			emit(SYM_NUMBER);
			return std::make_pair(STATE_START_, false);
		}
		return std::make_pair(state_, true);
	}

	std::pair<tokenizer::state, bool> tokenizer::state_quote(token tok)
	{
		switch (tok.value_) {
		case '\'':
			emit(SYM_VERBATIM);
			return std::make_pair(STATE_START_, true);

		case '\n':
			//	multiple lines not allowed
			err_(cyng::logging::severity::LEVEL_ERROR, "quotes with multiple lines not allowed");
			emit(SYM_VERBATIM);
			if (tok.count_ > 1)	emit(symbol(SYM_PAR, u8"¶"));
			return std::make_pair(STATE_START_, true);

		default:
			push(tok);
			break;
		}
		return std::make_pair(state_, true);
	}

	std::pair<tokenizer::state, bool> tokenizer::state_detect(token tok)
	{
		switch (tok.value_) {
		case '.':
			push(tok);
			emit(SYM_TEXT);	//	'.' as text after ')' and '"'
			return std::make_pair(STATE_START_, true);
		default:
			break;
		}
		return std::make_pair(STATE_START_, false);
	}

	void tokenizer::emit(symbol&& s) const
	{
		emit_(std::move(s));
	}

	void tokenizer::emit(symbol_type st)
	{
		if (!tmp_.empty()) {
			emit_(symbol(st, tmp_));
			tmp_.clear();
		}
	}

	void tokenizer::emit(symbol_type st, token tok) const
	{
		std::size_t count{ tok.count_ };
		while (count-- != 0u) {
			emit_(symbol(st, tok.value_));
		}
	}

	void tokenizer::emit_current_file(std::string file)
	{
		emit_(symbol(SYM_FILE, file));
	}

	void tokenizer::emit_current_line(std::size_t line)
	{
		emit_(symbol(SYM_LINE, std::to_string(line)));
	}


	void tokenizer::push(std::string s)
	{
		std::u32string s32(boost::u8_to_u32_iterator<std::string::const_iterator>(s.begin()), boost::u8_to_u32_iterator<std::string::const_iterator>(s.end()));
		tmp_.append(s32);
	}

	void tokenizer::push(token tok)
	{
		tmp_.append(tok.count_, tok.value_);
	}

	void tokenizer::push(std::uint32_t c, std::size_t count)
	{
		tmp_.append(count, c);
	}

	std::string get_state_name(tokenizer::state state)
	{
		switch (state) {
		case tokenizer::STATE_ERROR_:	return "ERROR";
		case tokenizer::STATE_START_:	return "START";
		case tokenizer::STATE_DOT_:		return "DOT";
		case tokenizer::STATE_NUMBER_:	return "NUMBER";
		case tokenizer::STATE_TOKEN_:	return "TOKEN";
		case tokenizer::STATE_QUOTE_:	return "QUOTE";
		case tokenizer::STATE_TEXT_:	return "TEXT";
		default:
			break;
		}
		return "unknown tokenizer state";
	}

}
