
#include <html/code_curly.h>
#include <html/formatting.h>

#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

namespace dom
{

	void curly_to_html(std::ostream&, 
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> start, 
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> end,
		bool numbers, 
		std::string const& lang) {

	}


}
