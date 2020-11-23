/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/token.h>
#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <limits>
#include <boost/predef.h>

namespace docscript
{
	token::token(std::uint32_t c, std::size_t count, bool eof)
		: value_(c)
		, count_(count)
		, eof_(eof)
	{}

	token::token(token const& tok)
		: value_(tok.value_)
		, count_(tok.count_)
		, eof_(tok.eof_)
	{}

	token::token(token&& tok)
		: value_(tok.value_)
		, count_(tok.count_)
		, eof_(tok.eof_)
	{
		const_cast<std::uint32_t&>(tok.value_) = 0u;
		const_cast<std::size_t&>(tok.count_) = 0u;
		const_cast<bool&>(tok.eof_) = true;
	}

	token make_eof()
	{
		return { std::numeric_limits<std::uint32_t>::max(), 0, true };
	}

	token make_token(std::uint32_t c, std::size_t count)
	{
		return { c, count, false };
	}

	token make_nl()
	{
		return make_token('\n', 1);
	}

	std::ostream& operator<<(std::ostream& os, const token& tok)
	{
		os << '[';

		if (tok.eof_)
		{
			os << "EOF";
		}
		else
		{
			os
				<< tok.count_
				<< '*'
				;

			if (tok.value_ == '\n')	{
				os << "NL";
			}
			else if (tok.value_ == '\r')	{
				os << "CR";
			}
			else if (tok.value_ == ' ')	{
				os << "SP";
			}
#if BOOST_OS_WINDOWS
			//	codepage 1252
			else if (tok.value_ == 0xb6) {
				os << "¶";
			}
			else if (tok.value_ == 0xbd) {
				os << "1/2";
			}
			else if (tok.value_ == 0xbe) {
				os << "3/4";
			}
			else if (tok.value_ == 0xbc) {
				os << "1/4";
			}
#endif
			else if (tok.value_ < 0xff)	{
				os << (char)tok.value_;
			}
			else {
				os << tok.value_;
			}
		}

		os << ']';

		return os;
	}
}


