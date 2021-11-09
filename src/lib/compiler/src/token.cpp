#include <docc/token.h>
#include <boost/assert.hpp>

namespace docscript {

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
}
