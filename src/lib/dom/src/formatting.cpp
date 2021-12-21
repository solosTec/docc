
#include <html/formatting.h>

#include <cyng/obj/tag.hpp>
#include <cyng/obj/numeric_cast.hpp>
#include <cyng/obj/container_cast.hpp>
#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>

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
				<< "&thinsp;&middot;&thinsp;10<sup>"
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

	void to_html(std::ostream& os, cyng::vector_t const& vec, std::string sep) {
		bool init = false;
		for (auto const& obj : vec) {
			if (init) {
				//os << ' ';
				os << sep;
			}
			else {
				init = true;
			}

			switch (obj.rtti().tag()) {
			case cyng::TC_VECTOR:
				to_html(os, cyng::container_cast<cyng::vector_t>(obj), " ");
				break;
			case cyng::TC_UINT64:
				//os << "UINT64";
				os
					<< "<span class=\"math\">"
					<< cyng::numeric_cast<std::uint64_t>(obj, 0)
					<< "</span>"
					;
				break;
			case cyng::TC_INT64:
				//os << "INT64";
				os
					<< "<span class=\"math\">"
					<< cyng::numeric_cast<std::int64_t>(obj, 0)
					<< "</span>"
					;
				break;
			case cyng::TC_DOUBLE:
				to_html(os, cyng::numeric_cast<double>(obj, 0.0));
				break;
			case cyng::TC_STRING:
				os << obj;
				break;
			case cyng::TC_TIME_POINT:
				to_html(os, cyng::value_cast(obj, std::chrono::system_clock::now()));
				//os << obj;
				break;
			case cyng::TC_FS_PATH:
				os << cyng::value_cast(obj, std::filesystem::current_path()).string();
				break;
			default:
				os << obj << ':' << obj.rtti().type_name();
				break;
			}
		}

	}

	std::string to_html(cyng::vector_t const& vec, std::string sep) {
		std::stringstream ss;
		to_html(ss, vec, sep);
		return ss.str();
	}

	void esc_html(std::ostream& os, std::string const& s) {
		for (auto c : s) {
			switch (c) {
			case '"': os << "&quot;"; break;
			case '&': os << "&amp;"; break;
			case '/': os << "&sol;"; break;
			case '<': os << "&lt;"; break;
			case '>': os << "&gt;"; break;
			case '‚': os << "&sbquo;"; break;	//	single low quote
			case '„': os << "&bdquo;"; break;	//	double low quote
			case '†': os << "&dagger;"; break;	//	dagger
			case '‡': os << "&Dagger;"; break;	//	double dagger
			case '‰': os << "&permil;"; break;	//	per mill sign
			case '‹': os << "&lsaquo;"; break;	//	single left angle quote
			case '›': os << "&rsaquo;"; break;	//	single right angle quote
			case '‘': os << "&lsquo;"; break;	//	left single quote
			case '’': os << "&rsquo;"; break;	//	right single quote
			case '“': os << "&ldquo;"; break;	//	left double quote
			case '”': os << "&rdquo;"; break;	//	right double quote
			default:
				os << c;
				break;
			}
		}
	}

}
