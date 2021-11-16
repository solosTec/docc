
#include <html/formatting.h>

#include <cmath>
#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

namespace dom
{

	void to_html(std::ostream& os, double d) {

		//	<span class="math">int.frac<sup>exp</sup></span>

		//
		//	after experimenting with std::frexp(), std::modf(), etc. a pragmatical solution
		//  is used: convert "d" to a string and extract the different parts.
		//  example: 3.230000e+06
		//

		std::stringstream ss;
		ss
			<< std::scientific
			<< d
			;
		auto const s = ss.str();

		std::vector<std::string> parts;
		boost::split(parts, s, boost::is_any_of(".e+-"), boost::token_compress_on);

		//	remove trailing zweor
		parts.at(1).erase(parts.at(1).find_last_not_of('0') + 1, std::string::npos);

		if (parts.size() == 3) {
			os
				<< "<span class =\"math\">"
				<< parts.at(0)
				<< '.'
				<< parts.at(1)
				<< "&nbsp;&middot;&nbsp;10<sup>"
				<< ((d < 1) ? '-' : '+')
				<< parts.at(2)
				<< "</sup></span>"
				;
		}
	}

	void to_html(std::ostream& os, std::chrono::system_clock::time_point tp) {
		const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
		os << std::put_time(std::localtime(&t_c), "%c");
	}

}
