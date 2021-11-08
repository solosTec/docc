﻿#include <tokenizer.h>
#include <cyng/io/parser/utf-8.h>

#ifdef _DEBUG
#include <iostream>
#endif

#include <boost/algorithm/string.hpp>

namespace docasm {

	tokenizer::tokenizer(emit_symbol_f cb)
		: state_(state::START_)
		, cb_(cb)
		, value_()
		, prev_sym_type_(symbol_type::EOD)
	{}

	bool tokenizer::put(token const& tok, token const& prev)
	{
		if (tok.is_eof()) {
			switch (state_) {
			case state::TEXT_:
				emit(symbol_type::TXT);
				value_ = 0x2205;    //  empty
				emit(symbol_type::EOL);
				break;
			case state::TIMESTAMP_:
				emit(symbol_type::TST);
				break;
			default:
				break;
			}
			emit(symbol_type::EOD);
			return true;
		}

		bool advance{ true };
		switch (state_) {
		case state::START_:
			std::tie(state_, advance) = start(tok, prev);
			break;
		case state::COMMENT_:
			std::tie(state_, advance) = comment(tok, prev);
			break;
		case state::DOT_:
			std::tie(state_, advance) = dot(tok, prev);
			break;
		case state::DIRECTIVE_:
			std::tie(state_, advance) = directive(tok, prev);
			break;
		case state::TIMESTAMP_:
			std::tie(state_, advance) = timestamp(tok, prev);
			break;
		case state::COLOR_:
			std::tie(state_, advance) = color(tok, prev);
			break;
		case state::NUMBER_:
			std::tie(state_, advance) = number(tok, prev);
			break;
		case state::TEXT_:
			std::tie(state_, advance) = text(tok, prev);
			break;
		case state::STRING_:
			std::tie(state_, advance) = string(tok, prev);
			break;
		case state::STRING_ESC_:
			std::tie(state_, advance) = string_esc(tok, prev);
			break;
		case state::QUOTE_:
			std::tie(state_, advance) = quote(tok, prev);
			break;
		case state::QUOTE_TRAIL_:
			std::tie(state_, advance) = quote_trail(tok, prev);
			break;
		case state::NEWLINE_:
			std::tie(state_, advance) = newline(tok, prev);
			break;
		case state::INSTRUCTION_:
			std::tie(state_, advance) = instruction(tok, prev);
			break;
		//case state::INCLUDE_:
		//	std::tie(state_, advance) = include(tok, prev);
		//	//
		//	//  handle special case of recursive call
		//	//
		//	if (state_ != state::INCLUDE_) {
		//		emit(symbol_type::INC);
		//	}
		//	break;
		default:
			break;
		}

		return advance;
	}

	std::pair<tokenizer::state, bool> tokenizer::start(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok))
		{
		case ';':
			return { state::COMMENT_, true };
			//if (prev.is_nl()) return { state::COMMENT_, true };
			break;

		case '.':
			return { state::DOT_, true };

		case ' ':   case '\t':
			//	ommit white spaces
			return { state_, true };

		case '\n':
			if (!prev.is_nl()) return { state::NEWLINE_, true };
			return { state_, true };

		//case '(':   case ')':
		case '[':   case ']':
		case ',':
		case ':':
			value_ += tok;
			emit(symbol_type::SYM);
			return { state_, true };

		case '"':
			//value_ += tok;
			//emit(symbol_type::DQU);
			//return { state_, true };
			return { state::STRING_, true };

		case '@':
			return { state::TIMESTAMP_, true };

		case '#':
			return { state::COLOR_, true };

		case '\'':
			return { state::QUOTE_, true };

		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0':
			return { state::NUMBER_, false };

		default:
			break;
		}

		if (prev.is_nl()) {
			return { state::INSTRUCTION_, false };
		}
		value_ += tok;
		return { state::TEXT_, true };
	}

	std::pair<tokenizer::state, bool> tokenizer::comment(token const& tok, token const& prev)
	{
		if (tok.is_nl())    return { state::START_, false };
		return { state_, true };
	}

	std::pair<tokenizer::state, bool> tokenizer::dot(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '.':
			//  '.' is the escape symbol
			if ('.' == prev) {
				emit(symbol_type::TXT);
				return { state::START_, true };
			}
			break;

		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
		case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '_':
			//  only lower case characters allowed, no numbers
			value_ += tok;
			return { state::DIRECTIVE_, true };

		case ' ':
			// no-break space: 0xC2 0xA0
			value_ += 0xC2A0;
			return { state::START_, true };

		case '\t':
			//  arrow: ↦
			value_ += 0x21A6;
			emit(symbol_type::TXT);
			return { state::START_, true };

		default:
			//  "." as escape symbol - following character remains unchanged
			value_ += tok;
			break;
		}
		return { state::START_, true };
	}

	std::pair<tokenizer::state, bool> tokenizer::directive(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
		case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '_':
			value_ += tok;
			return { state_, true };

		case ' ': case '\t':
			break;
		default:
			break;
		}

		//if (boost::algorithm::equals(value_, "include")) {

		//	value_.clear();
		//	return { state::INCLUDE_, true };
		//}

		//
		//  lookup build-in constants
		//
		if (boost::algorithm::equals(value_, "true") || boost::algorithm::equals(value_, "false")) {
			emit(symbol_type::BOL);
		}
		else [[likely]] {
			emit(symbol_type::DIR);
		}
		return { state::START_, false };
	}

	std::pair<tokenizer::state, bool> tokenizer::timestamp(token const& tok, token const& prev)
	{
		//  format: YYYY-MM-DD[THH:MM:SS[Zzz]]
		switch (static_cast<std::uint32_t>(tok)) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
			value_ += tok;
			break;

		case 'T':
			if (value_.size() != 10) {
				//	this is not a timestamp
				return { state::START_, true };
			}
			value_ += tok;
			break;

		case '-':
			switch (value_.size()) {
			case 4:
			case 7:
				break;
			default:
				//	this is not a timestamp
				return { state::START_, true };
				break;
			}
			value_ += tok;
			break;

		case ':':
			switch (value_.size()) {
			case 13:
			case 16:
				break;
			default:
				//	this is not a timestamp
				return { state::START_, true };
				break;
			}
			value_ += tok;
			break;

		case 'Z':
			if (value_.size() != 19) {
				return { state::START_, false };
			}
			value_ += tok;
			break;

		default:
			if (value_.size() == 10) {
				complete_ts();
				emit(symbol_type::TST);
				return { state::START_, false };
			}
			else if (value_.size() == 19) {
				complete_ts();
				emit(symbol_type::TST);
				return { state::START_, false };
			}
			else {
				//  
				//  do not interrupt the word
				//
				return { state::START_, false };
			}
			break;
		}

		if (value_.size() == 22) {
			emit(symbol_type::TST);
			return { state::START_, true };
		}
		return { state_, true };
	}

	void tokenizer::complete_ts() {
		if (value_.size() == 10) {
			value_ += 'T';
			value_ += '0';
			value_ += '0';
			value_ += ':';
			value_ += '0';
			value_ += '0';
			value_ += ':';
			value_ += '0';
			value_ += '0';
		}
	}

	std::pair<tokenizer::state, bool> tokenizer::color(token const& tok, token const& prev)
	{	// #RRGGBBAA
		switch (static_cast<std::uint32_t>(tok)) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
			value_ += tok;
			if (value_.size() == 8) {
				emit(symbol_type::COL);
				return { state::START_, false };
			}
			return { state_, true };
		default:
			break;
		}
		emit(symbol_type::COL);
		return { state::START_, false };
	}

	std::pair<tokenizer::state, bool> tokenizer::number(token const& tok, token const& prev) {

		switch (static_cast<std::uint32_t>(tok)) {
		case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': case '0':
			value_ += tok;
			return { state::NUMBER_, true };
		default:
			break;
		}

		emit(symbol_type::NUM);
		return { state::START_, false };
	}

	std::pair<tokenizer::state, bool> tokenizer::string(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '\"':	// complete
			return { state::TEXT_, true };

		case '\\':
			return { state::STRING_ESC_, true };

		default:
			break;
		}

		value_ += tok;
		return { state_, true };
	}

	std::pair<tokenizer::state, bool> tokenizer::string_esc(token const& tok, token const& prev) {

		switch (static_cast<std::uint32_t>(tok)) {
		case '\\':
		case '\"':
			value_ += tok;
			return { state::STRING_, true };
		case 'n':
			value_ += '\n';
			return { state::STRING_, true };
		case 't':
			value_ += '\t';
			return { state::STRING_, true };
		default:
			break;
		}
		emit(symbol_type::TXT);
		return { state::START_, false };

	}

	std::pair<tokenizer::state, bool> tokenizer::quote(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '\'':
			emit(symbol_type::LIT);
			return { state::QUOTE_TRAIL_, true };

		default:
			break;
		}

		value_ += tok;
		return { state_, true };
	}

	std::pair<tokenizer::state, bool> tokenizer::quote_trail(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
			value_ += tok;
			return { state_, true };
		default:
			break;
		}
		//	type specified
		emit(symbol_type::TYP);
		return { state::START_, false };
	}

	std::pair<tokenizer::state, bool> tokenizer::text(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '\n':
			emit(symbol_type::TXT);
			return { state::START_, false };
		case ' ':
		case '\t':
			emit(symbol_type::TXT);
			return { state::START_, true };

		case ':':
			emit(symbol_type::LBL);
			return { state::START_, true };

		case '.':
		case '!':   case '?':
			value_ += tok;
			emit(symbol_type::TXT);
			return { state::START_, true };

		case ';':	
		case ',':
			emit(symbol_type::TXT);
			return { state::START_, false };

		case '"':
			emit(symbol_type::TXT);
			return { state::START_, false };

		default:
			break;
		}

		value_ += tok;
		return { state_, true };
	}

	std::pair<tokenizer::state, bool> tokenizer::instruction(token const& tok, token const& prev) {
		switch (static_cast<std::uint32_t>(tok)) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J': case 'K': case 'L': case 'M':
		case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
		case '_':
			value_ += to_upper(tok);
			return { state_, true };

		case ':':
			emit(symbol_type::LBL);
			return { state::START_, true };
		default:
			break;
		}

		emit(symbol_type::INS);
		return { state::START_, false };

	}


	std::pair<tokenizer::state, bool> tokenizer::newline(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '\n':
		case ' ':
		case '\t':
			return { state_, true };

		default:
			break;
		}

		value_.append({ 0xb6 });

		emit(symbol_type::EOL);
		return { state::START_, false };
	}

	void tokenizer::emit(symbol_type type)
	{
		switch (type) {
		case symbol_type::EOL:
			if (symbol_type::EOL == prev_sym_type_) {
				//
				//	no duplicates
				//
				value_.clear();
				return;
			}
			break;
		case symbol_type::TXT:
		//case symbol_type::DQU:
			if (symbol_type::EOD == prev_sym_type_) {
				//	first TXT symbol ever
				//  pilgrow
				cb_(make_symbol(symbol_type::EOL, std::string("\xc2\xb6")));
			}
			break;
		default:
			break;
		}
		cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> start(value_.begin());
		cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> end(value_.end());
		cb_(make_symbol(type, std::string(start, end)));
		value_.clear();
		prev_sym_type_ = type;
	}

}