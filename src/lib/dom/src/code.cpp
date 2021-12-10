
#include <html/code.h>
#include <html/code_json.h>
#include <html/code_curly.h>
#include <html/code_docscript.h>
#include <html/code_html.h>
#include <html/code_generic.h>
#include <html/code_binary.h>

#include <cyng/io/parser/stream.hpp>

#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

namespace dom
{
	void code_to_html(std::ostream& os, std::filesystem::path const& p, std::string const& lang, bool numbers, std::string const& caption) {
		if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {

			if (boost::algorithm::iequals(lang, "json")) {
				//	JSON
				std::ifstream  ifs(p.string());
				auto [start, end] = cyng::get_stream_range<std::istream_iterator<char>>(ifs);
				//	JSON parser is UTF-8 capable
				if (ifs.is_open()) json_to_html(os, start, end, numbers);
				ifs.close();
			}
			else if (boost::algorithm::equals(lang, "C++") || boost::algorithm::iequals(lang, "cpp") || boost::algorithm::iequals(lang, "h")) {
				//	C++ - curly braced language
				std::ifstream  ifs(p.string());
				auto [start, end] = cyng::utf8::get_utf8_range<std::istream_iterator<char>>(ifs);
				if (ifs.is_open()) curly_to_html(os, start, end, numbers, lang);
				ifs.close();
			}
			else if (boost::algorithm::iequals(lang, "docscript")) {
				//	source language
				std::ifstream  ifs(p.string());
				if (ifs.is_open()) docscript_to_html(os, ifs, numbers);
				ifs.close();
			}
			else if (boost::algorithm::iequals(lang, "bin") || boost::algorithm::iequals(lang, "binary")) {
				//	hex listing
				std::ifstream  ifs(p.string(), std::ios::binary);
				auto start = std::istream_iterator<char>(ifs);
				auto end = std::istream_iterator<char>();
				if (ifs.is_open()) binary_to_html(os, start, end, numbers);
				ifs.close();
			}
			else {
				//	generic listing
				std::ifstream  ifs(p.string());
				if (ifs.is_open()) generic_to_html(os, ifs, numbers);
				ifs.close();
			}
		}
		else {
			os << "<div>source file is not a regular file</div>" << std::endl;
		}
	}




	


}
