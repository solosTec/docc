#include <asm/token.h>

#include <boost/assert.hpp>

namespace docasm {

    std::ostream& operator<<(std::ostream& os, const token& tok)
	{
		os << '[';

		if (tok.is_eof())
		{
			os << "EOF";
		}
		else
		{
			if (tok.c_ == '\n')	{
				os << "NL";
			}
			else if (tok.c_ == '\r')	{
				BOOST_ASSERT_MSG(false, "illegal character");
				os << "--";
			}
			else if (tok.c_ == ' ')	{
				os << "SP";
			}
			else if (tok.c_ < 0xff)	{
				os << (char)tok.c_;
			}
			else {
				os << tok.c_;
			}
		}

		os << ']';

		return os;
	}     

	std::uint32_t to_upper(token const& tok) {

		auto const c = static_cast<std::uint32_t>(tok);
		return (c > 96 && c < 123)
			? (c - 32)
			: c
			;
	}
	std::uint32_t to_lower(token const& tok) {

		auto const c = static_cast<std::uint32_t>(tok);
		return (c > 64 && c < 91)
			? (c + 32)
			: c
			;
	}
}
