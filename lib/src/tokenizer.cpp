#include <tokenizer.h>
#include <cyng/io/parser/utf-8.h>

#ifdef _DEBUG
#include <iostream>
#endif

#include <boost/algorithm/string.hpp>

namespace docscript {

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
				emit(symbol_type::PAR);
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
		case state::FUNCTION_:
			std::tie(state_, advance) = function(tok, prev);
			break;
		case state::TIMESTAMP_:
			std::tie(state_, advance) = timestamp(tok, prev);
			break;
		case state::TEXT_:
			std::tie(state_, advance) = text(tok, prev);
			break;
		case state::QUOTE_:
			std::tie(state_, advance) = quote(tok, prev);
			break;
		case state::QUOTE_ESC_:
			std::tie(state_, advance) = quote_esc(tok, prev);
			break;
		case state::NEWLINE_:
			std::tie(state_, advance) = newline(tok, prev);
			break;
		case state::INCLUDE_:
			std::tie(state_, advance) = include(tok, prev);
			//
			//  handle special case of recursive call
			//
			if (state_ != state::INCLUDE_) {
				emit(symbol_type::INC);
			}
			break;
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
			if (prev.is_nl()) return { state::COMMENT_, true };
			break;

		case '.':
			return { state::DOT_, true };

		case ' ':   case '\t':
			//	ommit white spaces
			return { state_, true };

		case '\n':
			if (prev.is_nl()) return { state::NEWLINE_, true };
			return { state_, true };

		case '(':   case ')':
		case '[':   case ']':
		case '{':   case '}':
		case ',':
		case ':':
			value_ += tok;
			emit(symbol_type::SYM);
			return { state_, true };

		case '"':
			value_ += tok;
			emit(symbol_type::DQU);
			return { state_, true };

		case '@':
			return { state::TIMESTAMP_, true };

		case '\'':
			return { state::QUOTE_, true };

		default:
			break;
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
		case '_':
			//  only lower case characters allowed, no numbers
			value_ += tok;
			return { state::FUNCTION_, true };

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

	std::pair<tokenizer::state, bool> tokenizer::function(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k': case 'l': case 'm':
		case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
		case '_':
			value_ += tok;
			return { state_, true };

		case ' ': case '\t':
			if (boost::algorithm::equals(value_, "p")) {
				//	explicit paragraph with ".p"
				emit(symbol_type::PAR);
				value_.clear();
				return { state::START_, false };
			}

		default:
			break;
		}

		if (boost::algorithm::equals(value_, "include")) {

			value_.clear();
			return { state::INCLUDE_, true };
		}

		//
		//  function name complete
		//
		emit(symbol_type::FUN);
		return { state::START_, false };
	}

	std::pair<tokenizer::state, bool> tokenizer::timestamp(token const& tok, token const& prev)
	{
		//  format: YYYY-MM-DD[THH:MM:SS.zzZ]
		switch (static_cast<std::uint32_t>(tok)) {
		case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9':
		case 'Z':
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

		case '.':
			if (value_.size() == 10) {
				complete_ts();
				emit(symbol_type::TST);
			}
			return { state::START_, false };

		default:
			if (value_.size() == 10) {
				complete_ts();
				emit(symbol_type::TST);
				return { state::START_, true };
			}
			else if (value_.size() == 19) {
				complete_ts();
				emit(symbol_type::TST);
				return { state::START_, true };
			}
			else if (value_.size() == 23) {
				emit(symbol_type::TST);
				return { state::START_, true };
			}
			else {
				//  
				//  do not interrupt the word
				//
				return { state::START_, false };
			}
			break;
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
			value_ += '.';
			value_ += '0';
			value_ += '0';
			value_ += 'Z';
		}
		else if (value_.size() == 19) {
			value_ += '.';
			value_ += '0';
			value_ += '0';
			value_ += 'Z';
		}
	}

	std::pair<tokenizer::state, bool> tokenizer::quote(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '\'':
			return { state::TEXT_, true };

		case '\\':
			return { state::QUOTE_ESC_, true };

		default:
			break;
		}

		value_ += tok;
		return { state_, true };
	}

	std::pair<tokenizer::state, bool> tokenizer::quote_esc(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '\\':
		case '\'':
			value_ += tok;
			return { state::QUOTE_, true };
		case 'n':
			value_ += '\n';
			return { state::QUOTE_, true };
		case 't':
			value_ += '\t';
			return { state::QUOTE_, true };
		default:
			break;
		}
		emit(symbol_type::TXT);
		return { state::START_, false };
	}

	std::pair<tokenizer::state, bool> tokenizer::text(token const& tok, token const& prev)
	{
		switch (static_cast<std::uint32_t>(tok)) {
		case '\n':
		case ' ':
		case '\t':
			emit(symbol_type::TXT);
			return { state::START_, true };

		case '.':
		case '!':   case '?':
			value_ += tok;
			emit(symbol_type::TXT);
			return { state::START_, true };

		case '(':   case ')':
		//case '[':   case ']':
		//case '{':   case '}':
		case ';':	case ':':
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

		emit(symbol_type::PAR);
		return { state::START_, false };
	}

	std::pair<tokenizer::state, bool> tokenizer::include(token const& tok, token const& prev) {
		switch (static_cast<std::uint32_t>(tok)) {
		case '\t':
		case '(':
		case ' ':
			return { state_, true };
		case ')':
		case '\n':
			return { state::START_, true };
		default:
			break;
		}
		value_ += tok;
		return { state_, true };
	}

	void tokenizer::emit(symbol_type type)
	{
		switch (type) {
		case symbol_type::PAR:
			if (symbol_type::PAR == prev_sym_type_) {
				//
				//	no duplicates
				//
				value_.clear();
				return;
			}
			break;
		case symbol_type::TXT:
		case symbol_type::DQU:
			if (symbol_type::EOD == prev_sym_type_) {
				//	first TXT symbol ever
				//  pilgrow
				cb_(make_symbol(symbol_type::PAR, std::string("\xc2\xb6")));
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