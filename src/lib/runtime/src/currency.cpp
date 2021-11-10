
#include <rt/currency.h>

#include <sstream>

#include <boost/algorithm/string.hpp>

namespace docruntime
{
	std::string currency(std::size_t value, std::string const& name) {
		std::stringstream ss;
		if (boost::algorithm::equals(name, "dollar")) {
			ss << "$" << value;
		}
		else if (boost::algorithm::equals(name, "cent")) {
			ss << value << std::string(" \xC2\xA2");
		}
		else if (boost::algorithm::equals(name, "pound")) {
			ss << value << std::string(" \xC2\xA3");	// £ == \xC2\xA3
		}
		else if (boost::algorithm::equals(name, "yen")) {
			ss << value << std::string(" \xEF\xBF\xA5");
		}
		else if (boost::algorithm::equals(name, "rupee")) {
			//	indian rupee sign: ₹
			ss << value << std::string(" \xE2\x82\xA8");
		}
		else if (boost::algorithm::equals(name, "dram")) {
			//	armenian dram
			ss << value << std::string(" \xD6\x8F");
		}
		else if (boost::algorithm::equals(name, "afghani")) {
			//	afghani
			ss << value << std::string(" \xD8\x8B");
		}
		else if (boost::algorithm::equals(name, "bitcoin")) {
			ss << value << std::string(" \xE2\x82\xBF");
		}
		else if (boost::algorithm::equals(name, "pfennig")) {
			//	deutscher Pfennig: ₰
			ss << value << std::string(" \xE2\x82\xB0");
		}
		else if (boost::algorithm::equals(name, "sheqel")) {
			//	new sheqel: ₪
			ss << value << std::string(" \xE2\x82\xAA");
		}
		else if (boost::algorithm::equals(name, "tugrik")) {
			//	mongolian currency: ₮
			ss << value << std::string(" \xE2\x82\xAE");
		}
		else if (boost::algorithm::equals(name, "rial")) {
			//	iranian currency: ﷼
			ss << value << std::string(" \xEF\xB7\xBC");
		}
		else {
			//	euro
			ss << value << std::string(" \xE2\x82\xAC");
		}
		return ss.str();
	}

	std::string currency_html(std::size_t value, std::string const& name) {
		std::stringstream ss;
		if (boost::algorithm::equals(name, "dollar")) {
			ss << "&#x24; " << value;
		}
		else if (boost::algorithm::equals(name, "cent")) {
			ss << value << " &#xa2;";
		}
		else if (boost::algorithm::equals(name, "pound")) {
			ss << value << "&#xa3;";	// £ == \xC2\xA3
		}
		else if (boost::algorithm::equals(name, "yen")) {
			ss << value << " &#xa5;";
		}
		else if (boost::algorithm::equals(name, "rupee")) {
			//	indian rupee sign: ₹
			ss << value << " &#x20B9;";
		}
		else if (boost::algorithm::equals(name, "dram")) {
			//	armenian dram
			ss << value << " &#x58f;";
		}
		else if (boost::algorithm::equals(name, "afghani")) {
			//	afghani: ؋
			ss << value << " &#x60b;";
		}
		else if (boost::algorithm::equals(name, "bitcoin")) {
			ss << value << " &#x20BF;";
		}
		else if (boost::algorithm::equals(name, "pfennig")) {
			//	deutscher Pfennig: ₰
			ss << value << " &#x20B0;";
		}
		else if (boost::algorithm::equals(name, "sheqel")) {
			//	new sheqel: ₪
			ss << value << " &#x20AA";
		}
		else if (boost::algorithm::equals(name, "tugrik")) {
			//	mongolian currency: ₮
			ss << value << " &#x20AE;";
		}
		else if (boost::algorithm::equals(name, "rial")) {
			//	iranian currency: ﷼
			ss << value << " &#xFDFC";
		}
		else {
			//	euro
			ss << value << " &#x20AC;";
		}
		return ss.str();

	}


}

