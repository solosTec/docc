/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "cpp_to_html.h"
#include <docscript/sanitizer.h>

#include <cctype>
#include <iomanip>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

namespace docscript
{
	namespace cpp
	{
		enum symbol_type
		{
			SYM_EOF,		//!<	no more symbols
			SYM_UNKNOWN,	//!<	unknown or error state
			SYM_MARK,		//!<	single symbol like "{", "}", "(", ")", ...
			SYM_ENITIY,		//!<	single symbol like "<", ">", "&", ...
			SYM_PRE,		//!<	precompiler statement like #ifdef, #include, ...
			SYM_INC_SYS,	//!<	<...>
			SYM_INC_USER,	//!<	"...."
			SYM_PRE_STM,	//!<	any preprocessor statement
			SYM_COMMENT,	//!<	// ...
			SYM_KEYWORD,	//!<	int, long, while, ...
			SYM_LITERAL,	//!<	any literal string
			SYM_NUMBER,		//!<	0 ... 9
			SYM_STRING,		//!<	"..."
			SYM_CHAR,		//!<	'.'
			SYM_DBL_OP,		//!<	<< >> ++ -- && ||
			SYM_NL,			//!<	new line

		};

		struct symbol
		{
			symbol(symbol_type t, std::string const& s)
				: type_(t)
				, value_(s)
			{}
			symbol(symbol_type t, std::u32string const& s)
				: type_(t)
				, value_(boost::u32_to_u8_iterator<std::u32string::const_iterator>(s.begin()), boost::u32_to_u8_iterator<std::u32string::const_iterator>(s.end()))
			{}
			symbol(symbol_type t, std::uint32_t c)
				: type_(t)
				, value_(1, c)
			{}
			symbol(symbol const& other)
				: type_(other.type_)
				, value_(other.value_)
			{}
			symbol(symbol&& other)
				: type_(other.type_)
				, value_(std::move(other.value_))
			{
				const_cast<symbol_type&>(other.type_) = SYM_UNKNOWN;
			}

			bool is_equal(std::string test) const
			{
				return boost::algorithm::equals(value_, test);
			}
			bool is_type(symbol_type st) const
			{
				return st == type_;
			}

			void swap(symbol& other)
			{
				std::swap(const_cast<symbol_type&>(type_), const_cast<symbol_type&>(other.type_));
				std::swap(const_cast<std::string&>(value_), const_cast<std::string&>(other.value_));
			}

			symbol_type const type_;
			std::string const value_;
		};

		void swap(symbol& s1, symbol& s2)
		{
			s1.swap(s2);
		}
		std::string name(symbol_type st)
		{
			switch (st)
			{
			case SYM_EOF:		return "EOF";
			case SYM_UNKNOWN:	return "???";
			case SYM_MARK:		return "MRK";	//	single symbol
			case SYM_ENITIY:	return "ETY";	//	single symbol
			case SYM_PRE:		return "PRE";
			case SYM_INC_SYS:	return "INS";	//!<	<...>
			case SYM_INC_USER:	return "INU";	//!<	"...."
			case SYM_PRE_STM:	return "PRS";	//!<	any preprocessor statement
			case SYM_COMMENT:	return "REM";	//	remark
			case SYM_KEYWORD:	return "KEY";
			case SYM_LITERAL:	return "LIT";
			case SYM_NUMBER:	return "NUM";
			case SYM_STRING:	return "STR";
			case SYM_CHAR:		return "CHAR";
			case SYM_DBL_OP:	return "DOP";	//	<< >> && || ++ --
			case SYM_NL:		return "N L";	//	new line
			default:
				break;
			}
			return "ERR";

		}

		std::ostream& operator<<(std::ostream& os, const symbol& sym)
		{
			switch(sym.type_) {
			case SYM_NL:
				os
					<< '{'
					<< name(sym.type_)
					<< "\\n"
					<< '}'
					   ;
				break;
			case SYM_MARK:
				os
					<< '{'
					<< name(sym.type_)
					<< ' '
					<< '"'
					<< sym.value_
					<< '"'
					<< '}'
					   ;
				break;
			default:
				os
					<< '{'
					<< name(sym.type_)
					<< ' '
					<< sym.value_
					<< '}'
					;
				break;
			}
			return os;
		}

		class tokenizer
		{
		private:
			enum state {
				INITIAL_,
				QUOTE_,
				CHAR_,
				//SLASH_,
				COMMENT_,
				LITERAL_,
				NUMBER_,
				PRE_,
				PRE_STM_,
				INCLUDE_,
			} state_;

		public:
			tokenizer(std::function<void(std::size_t, symbol&&)> emit)
				: state_(INITIAL_)
				, emit_(emit)
				, tmp_()
				, linenumber_(0u)
			{
				//
				//	initialize keyword list
				//
				keywords_.emplace("auto");
				keywords_.emplace("bool");
				keywords_.emplace("break");
				keywords_.emplace("case");
				keywords_.emplace("catch");
				keywords_.emplace("char");
				keywords_.emplace("class");
				keywords_.emplace("const");
				keywords_.emplace("const_cast");
				keywords_.emplace("continue");
				keywords_.emplace("decltype");
				keywords_.emplace("default");
				keywords_.emplace("delete");
				keywords_.emplace("do");
				keywords_.emplace("double");
				keywords_.emplace("dynamic_cast");
				keywords_.emplace("else");
				keywords_.emplace("enum");
				keywords_.emplace("explicit");
				keywords_.emplace("extern");
				keywords_.emplace("false");
				keywords_.emplace("float");
				keywords_.emplace("for");
				keywords_.emplace("friend");
				keywords_.emplace("goto");
				keywords_.emplace("if");
				keywords_.emplace("inline");
				keywords_.emplace("int");
				keywords_.emplace("long");
				keywords_.emplace("mutable");
				keywords_.emplace("namespace");
				keywords_.emplace("new");
				keywords_.emplace("nullptr");
				keywords_.emplace("operator");
				keywords_.emplace("private");
				keywords_.emplace("protected");
				keywords_.emplace("public");
				keywords_.emplace("register");
				keywords_.emplace("reinterpret_cast");
				keywords_.emplace("return");
				keywords_.emplace("short");
				keywords_.emplace("signed");
				keywords_.emplace("sizeof");
				keywords_.emplace("static");
				keywords_.emplace("static_assert");
				keywords_.emplace("switch");
				keywords_.emplace("template");
				keywords_.emplace("this");
				keywords_.emplace("throw");
				keywords_.emplace("true");
				keywords_.emplace("try");
				keywords_.emplace("typedef");
				keywords_.emplace("typeid");
				keywords_.emplace("typename");
				keywords_.emplace("union");
				keywords_.emplace("unsigned");
				keywords_.emplace("using");
				keywords_.emplace("virtual");
				keywords_.emplace("void");
				keywords_.emplace("volatile");
				keywords_.emplace("while");
			}

			bool next(token tok)
			{
				bool advance = true;

				//std::cout << "---> " << tok << std::endl;

				switch (state_) {
				case INITIAL_:
					std::tie(state_, advance) = state_initial(tok);
					break;
				case QUOTE_:
					std::tie(state_, advance) = state_quote(tok);
					break;
				case CHAR_:
					std::tie(state_, advance) = state_char(tok);
					break;
				case COMMENT_:
					std::tie(state_, advance) = state_comment(tok);
					break;
				case LITERAL_:
					std::tie(state_, advance) = state_literal(tok);
					break;
				case NUMBER_:
					std::tie(state_, advance) = state_number(tok);
					break;
				case PRE_:
					std::tie(state_, advance) = state_pre(tok);
					break;
				case PRE_STM_:
					std::tie(state_, advance) = state_pre_stm(tok);
					break;
				case INCLUDE_:
					std::tie(state_, advance) = state_include(tok);
					break;
				default:
					break;
				}

				return advance;
			}


		private:
			std::pair<state, bool> state_initial(token tok)
			{
				if (tok.eof_)	return std::make_pair(state_, true);

				switch (tok.value_) {
				case '{':
				case '}':
				case '(':
				case ')':
					emit(SYM_MARK, tok);
					break;
				case '<':
				case '>':
				case '&':
					if (tok.count_ == 2) {
						emit(symbol(SYM_DBL_OP, tok.value_));
					}
					else {
						emit(SYM_ENITIY, tok);
					}
					break;
				case '|':
				case '+':
				case '-':
				case ':':
					if (tok.count_ == 2) {
						emit(symbol(SYM_DBL_OP, tok.value_));
					}
					else {
						emit(SYM_MARK, tok);
					}
					break;
				case '"':
					return std::make_pair(QUOTE_, true);
				case '\'':
					return std::make_pair(CHAR_, true);
				case '/':
					if (tok.count_ > 1) {
						return std::make_pair(COMMENT_, true);
					}
					emit(SYM_MARK, tok);
					break;
				case '#':
					return std::make_pair(PRE_, true);
				case '\n':
					linenumber_ += tok.count_;
					emit(SYM_NL, tok);
					break;
				default:
					if ((tok.value_ < 255) && std::isalpha(tok.value_))
					{
						push(tok);
						return std::make_pair(LITERAL_, true);
					}
					else if ((tok.value_ < 255) && std::isdigit(tok.value_))
					{
						push(tok);
						return std::make_pair(NUMBER_, true);
					}
					else
					{
						//	single symbol
						emit(SYM_MARK, tok);
					}
					break;
				}
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_pre(token tok)
			{
				if ((tok.value_ < 255) && std::isalpha(tok.value_))
				{
					push(tok);
					return std::make_pair(state_, true);
				}
				else if (tok.value_ == ' ' || tok.value_ == '\t')
				{
					//	skip spaces
					return std::make_pair(state_, true);
				}
				else if (tok.value_ == '<')
				{
					emit(SYM_PRE);
					return std::make_pair(INCLUDE_, true);
				}
				else if (tok.value_ == '"')
				{
					emit(SYM_PRE);
					return std::make_pair(INCLUDE_, true);
				}

				//
				//	other
				//
				emit(SYM_PRE);
				return std::make_pair(PRE_STM_, false);

			}

			std::pair<state, bool> state_pre_stm(token tok)
			{
				if (tok.value_ == '\n')
				{
					emit(SYM_PRE_STM);
					return std::make_pair(INITIAL_, false);
				}
				push(tok);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_include(token tok)
			{
				if (tok.eof_)	return std::make_pair(state_, true);
				if (tok.value_ == '>')
				{
					emit(SYM_INC_SYS);
					return std::make_pair(INITIAL_, true);
				}
				else if (tok.value_ == '"')
				{
					emit(SYM_INC_USER);
					return std::make_pair(INITIAL_, true);
				}
				else if (tok.value_ == '\n')
				{
					emit(SYM_INC_USER);
					return std::make_pair(INITIAL_, false);
				}

				push(tok);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_comment(token tok)
			{
				if (tok.value_ == '\n')
				{
					emit(SYM_COMMENT);
					return std::make_pair(INITIAL_, false);
				}
				push(tok);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_quote(token tok)
			{
				if (tok.value_ == '"') {
					emit(SYM_STRING);
					return std::make_pair(INITIAL_, true);
				}
				push(tok);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_char(token tok)
			{
				if (tok.value_ == '\'') {
					emit(SYM_CHAR);
					return std::make_pair(INITIAL_, true);
				}
				push(tok);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_literal(token tok)
			{
				if (tok.value_ < 128 && std::isalnum(tok.value_)) {

					push(tok);
					return std::make_pair(state_, true);
				}

				emit_literal();
				return std::make_pair(INITIAL_, false);
			}

			std::pair<state, bool> state_number(token tok)
			{
				if (tok.value_ < 128 && std::isdigit(tok.value_)) {
					push(tok);
					return std::make_pair(state_, true);
				}
				emit(SYM_NUMBER);
				return std::make_pair(INITIAL_, false);

			}

			/**
			 * emit tmp_ as next symbol
			 */
			void emit(symbol_type st)
			{
				if (!tmp_.empty()) {
					emit_(linenumber_, symbol(st, tmp_));
					tmp_.clear();
				}
			}

			/**
			 * emit tmp_ as next symbol
			 */
			void emit(symbol&& s) const
			{
				emit_(linenumber_, std::move(s));
			}


			void emit(symbol_type st, token tok) const
			{
				std::size_t count{ tok.count_ };
				while (count-- != 0u) {
					emit_(linenumber_, symbol(st, tok.value_));
				}
			}

			void emit_literal()
			{
				if (is_keyword(tmp_)) {
					emit(SYM_KEYWORD);
				}
				else {
					emit(SYM_LITERAL);
				}
			}

			void push(token tok)
			{
				tmp_.append(tok.count_, tok.value_);
			}

			bool is_keyword(std::string const& str) const
			{
				return keywords_.find(str) != keywords_.end();
			}

		private:
			std::function<void(std::size_t, symbol&&)> emit_;

			/**
			 * temporary buffer for next symbol
			 */
			std::string tmp_;

			std::set< std::string >	keywords_;

			std::size_t linenumber_;

		};
	}

	cpp_to_html::cpp_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}

	void cpp_to_html::convert(std::ostream& os, std::string const& inp)
	{
		/**
		 * Process the stream of input tokens and generate symbols
		 */
		cpp::tokenizer _tokenizer([&](std::size_t linenumber, cpp::symbol&& sym) {
			
//			std::cout << sym << std::endl;
			switch (sym.type_) {
			case cpp::SYM_EOF:
				os << "EOFs";
				break;
			case cpp::SYM_UNKNOWN:
				os << "???";
				break;
			case cpp::SYM_MARK:
				os << sym.value_;
				break;
			case cpp::SYM_ENITIY:
				write_entity(os, sym.value_);
				break;
			case cpp::SYM_PRE:
				os
					<< '#'
					<< color_grey_
					<< sym.value_
					<< end_
					<< ' '
					;
				break;
			case cpp::SYM_INC_SYS:
				os
					<< '<'
					<< color_red_
					<< sym.value_
					<< end_
					<< '>'
					;
				break;
			case cpp::SYM_INC_USER:
				os
					<< '"'
					<< color_red_
					<< sym.value_
					<< end_
					<< '"'
					;
				break;
			case cpp::SYM_PRE_STM:
				os
					<< color_brown_
					<< sym.value_
					<< end_
					;
				break;
			case cpp::SYM_COMMENT:
				os
					<< "// "
					<< color_green_
					<< sym.value_
					<< end_
					;
				break;
			case cpp::SYM_KEYWORD:
				os
					<< color_blue_
					<< sym.value_
					<< end_
					;
				break;
			case cpp::SYM_LITERAL:
				os
					<< sym.value_
					;
				break;
			case cpp::SYM_NUMBER:
				os
					<< color_brown_
					<< sym.value_
					<< end_
					;
				break;
			case cpp::SYM_STRING:
				os
					<< '"'
					<< color_red_
					<< sym.value_
					<< end_
					<< '"'
					;
				break;
			case cpp::SYM_CHAR:
				os
					<< '\''
					<< color_red_
					<< sym.value_
					<< end_
					<< '\''
					;
				break;
			case cpp::SYM_DBL_OP:
				os
					<< color_green_
					<< sym.value_
					<< sym.value_
					<< end_
					;
				break;
			case cpp::SYM_NL:
				write_nl(linenumber, os);
				/* fall through */
			default:
				break;
			}

			});


		/**
		 * create tokens for tokenizer
		 */
		sanitizer san([&](token&& tok) {

			while (!_tokenizer.next(tok)) {
				;
			}

			}, std::bind(&cpp_to_html::print_error, this, std::placeholders::_1, std::placeholders::_2));

		write_nl(0, os);

		//
		//	read input
		//
		auto const start = std::begin(inp);
		auto const stop = std::end(inp);
		san.read(boost::u8_to_u32_iterator<std::string::const_iterator>(start), boost::u8_to_u32_iterator<std::string::const_iterator>(stop));
		san.flush(true);
	}

	void cpp_to_html::write_nl(std::size_t linenumber, std::ostream& os)
	{
		if (linenumber != 0) {
			os 
				<< "</code>"
				<< std::endl
				<< "<code contenteditable spellcheck=\"false\">"
				;
		}
		else {
			os << "<code contenteditable spellcheck=\"false\">";
		}

		if (linenumbers_) {
			os
				<< "<span style = \"color: DarkCyan; font-size: smaller;\" id=\""
				<< tag_
				<< '-'
				<< linenumber
				<< "\">"
				<< std::setw(4)
				<< std::setfill(' ')
				<< linenumber
				<< "</span> "
				;
		}
	}

	void cpp_to_html::write_entity(std::ostream& os, std::string entity)
	{
		for (auto const c : entity) {
			switch (c) {
			case '<':
				os << "&lt;";
				break;
			case '>':
				os << "&gt;";
				break;
			case '&':
				os << "&amp;";
				break;
			case '"':
				os << "&quot;";
				break;
			default:
				os << c;
				break;
			}
		}
	}


	void cpp_to_html::print_error(cyng::logging::severity level, std::string msg)
	{
		std::cerr << msg << std::endl;
	}

	//
	//	constants
	//
	std::string const cpp_to_html::color_green_ = "<span style=\"color: green;\">";
	std::string const cpp_to_html::color_blue_ = "<span style=\"color: blue;\">";
	std::string const cpp_to_html::color_grey_ = "<span style=\"color: grey;\">";
	std::string const cpp_to_html::color_red_ = "<span style=\"color: sienna;\">";
	std::string const cpp_to_html::color_cyan_ = "<span style=\"color: DarkCyan; font-size: smaller;\">";
	std::string const cpp_to_html::color_brown_ = "<span style=\"color: brown;\">";
	std::string const cpp_to_html::end_ = "</span>";
}


