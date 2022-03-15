
#include <boost/algorithm/string.hpp>
#include <cmath>
#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>
#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/numeric_cast.hpp>
#include <cyng/obj/tag.hpp>
#include <iomanip>
#include <sstream>
#include <tex/formatting.h>

namespace tex {

    void to_tex(std::ostream& os, double d) { os << "\\num{" << d << "}"; }

    void to_tex(std::ostream& os, std::chrono::system_clock::time_point tp)
    {
	const std::time_t t_c = std::chrono::system_clock::to_time_t(tp);
	os << std::put_time(std::localtime(&t_c), "%c");
    }

    void to_tex(std::ostream& os, cyng::vector_t const& vec, std::string sep)
    {
	bool init = false;
	for (auto const& obj : vec) {
	    if (init) {
		// os << ' ';
		os << sep;
	    }
	    else {
		init = true;
	    }

	    switch (obj.tag()) {
	    case cyng::TC_VECTOR: to_tex(os, cyng::container_cast< cyng::vector_t >(obj), " "); break;
	    case cyng::TC_UINT64:
		// os << "UINT64";
		os << "\\num{" << cyng::numeric_cast< std::uint64_t >(obj, 0) << "}";
		break;
	    case cyng::TC_INT64:
		// os << "INT64";
		os << "\\num{" << cyng::numeric_cast< std::int64_t >(obj, 0) << "}";
		break;
	    case cyng::TC_DOUBLE: to_tex(os, cyng::numeric_cast< double >(obj, 0.0)); break;
	    case cyng::TC_STRING: os << obj; break;
	    case cyng::TC_TIME_POINT:
		to_tex(os, cyng::value_cast(obj, std::chrono::system_clock::now()));
		// os << obj;
		break;
	    case cyng::TC_FS_PATH: os << cyng::value_cast(obj, std::filesystem::current_path()).string(); break;
	    default: os << obj << ':' << obj.rtti().type_name(); break;
	    }
	}
    }

    std::string to_tex(cyng::vector_t const& vec, std::string sep)
    {
	std::stringstream ss;
	to_tex(ss, vec, sep);
	return ss.str();
    }

    void esc_tex(std::ostream& os, std::string const& s)
    {
	for (auto c : s) {
	    switch (c) {
	    case '&': os << "\\&"; break;
	    case '%': os << "\\%"; break;
	    case '$': os << "\\$"; break;
	    case '#': os << "\\#"; break;
	    case '_': os << "\\_"; break;
	    case '{': os << "\\{"; break;
	    case '}': os << "\\}"; break;
	    case '~': os << "\\textasciitilde "; break;
	    case '^': os << "\\textasciicircum "; break;
	    case '\\': os << "\\textbackslash "; break;
	    // case '"': os << "&quot;"; break;
	    // case '/': os << "&sol;"; break;
	    case '<': os << "\\textless "; break;
	    case '>': os << "\\textgreater "; break;
	    // case '‚': os << "&sbquo;"; break;	//	single low quote
	    // case '„': os << "&bdquo;"; break;	//	double low quote
	    case '†': os << "\\dag ;"; break; //	dagger
	    case '‡':
		os << "\\ddag ";
		break; //	double dagger
	    // case '‰': os << "&permil;"; break;	//	per mill sign
	    // case '‹': os << "&lsaquo;"; break;	//	single left angle quote
	    // case '›': os << "&rsaquo;"; break;	//	single right angle quote
	    // case '‘': os << "&lsquo;"; break;	//	left single quote
	    // case '’': os << "&rsquo;"; break;	//	right single quote
	    // case '“': os << "&ldquo;"; break;	//	left double quote
	    // case '”': os << "&rdquo;"; break;	//	right double quote
	    case '|': os << "\\textbar "; break; //	right double quote
	    case '€': os << "\\euro{} "; break; //	right double quote
	    default: os << c; break;
	    }
	}
    }

}
