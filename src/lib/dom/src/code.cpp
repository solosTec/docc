
#include <html/code.h>
#include <html/code_json.h>
#include <html/code_curly.h>
#include <html/code_docscript.h>
#include <html/code_html.h>
#include <html/code_generic.h>
#include <html/code_binary.h>
#include <html/code_cmake.h>

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
			else if (boost::algorithm::equals(lang, "C++") 
				|| boost::algorithm::iequals(lang, "cpp") 
				|| boost::algorithm::iequals(lang, "h")) {

				//	C++ - curly braced language
				std::ifstream  ifs(p.string());
				ifs >> std::noskipws;
				auto [start, end] = cyng::utf8::get_utf8_range<std::istream_iterator<char>>(ifs);
				if (ifs.is_open()) curly_to_html(os, start, end, numbers, "C++");
				ifs.close();
			}
			else if (boost::algorithm::equals(lang, "Java") 
				|| boost::algorithm::iequals(lang, "java")) {

				//	Java - curly braced language
				std::ifstream  ifs(p.string());
				ifs >> std::noskipws;
				auto [start, end] = cyng::utf8::get_utf8_range<std::istream_iterator<char>>(ifs);
				if (ifs.is_open()) curly_to_html(os, start, end, numbers, "Java");
				ifs.close();
			}
			else if (boost::algorithm::equals(lang, "JavaScript") 
				|| boost::algorithm::iequals(lang, "JS") 
				|| boost::algorithm::iequals(lang, "js")) {

				//	JavaScript - curly braced language
				std::ifstream  ifs(p.string());
				ifs >> std::noskipws;
				auto [start, end] = cyng::utf8::get_utf8_range<std::istream_iterator<char>>(ifs);
				if (ifs.is_open()) curly_to_html(os, start, end, numbers, "JavaScript");
				ifs.close();
			}
			else if (boost::algorithm::equals(lang, "C#") 
				|| boost::algorithm::iequals(lang, "CSharp") 
				|| boost::algorithm::iequals(lang, "csharp")) {

				//	C# - curly braced language
				std::ifstream  ifs(p.string());
				ifs >> std::noskipws;
				auto [start, end] = cyng::utf8::get_utf8_range<std::istream_iterator<char>>(ifs);
				if (ifs.is_open()) curly_to_html(os, start, end, numbers, "C#");
				ifs.close();
			}
			else if (boost::algorithm::equals(lang, "Zig") 
				|| boost::algorithm::iequals(lang, "zig") 
				|| boost::algorithm::iequals(lang, "ZigLang")) {

				//	Zig - curly braced language
				std::ifstream  ifs(p.string());
				ifs >> std::noskipws;
				auto [start, end] = cyng::utf8::get_utf8_range<std::istream_iterator<char>>(ifs);
				if (ifs.is_open()) curly_to_html(os, start, end, numbers, "Zig");
				ifs.close();
			}
			//
			//	ToDo: Rust, Groovy, Kotlin, Perl, PHP, Scala, Swift, Go
			//
			else if (boost::algorithm::iequals(lang, "docscript")) {
				//	source language
				std::ifstream  ifs(p.string());
				if (ifs.is_open()) docscript_to_html(os, ifs, numbers);
				ifs.close();
			}
			else if (boost::algorithm::iequals(lang, "bin") 
				|| boost::algorithm::iequals(lang, "binary")) {
				//	hex listing
				std::ifstream  ifs(p.string(), std::ios::binary);
				auto start = std::istream_iterator<char>(ifs);
				auto end = std::istream_iterator<char>();
				if (ifs.is_open()) binary_to_html(os, start, end, numbers);
				ifs.close();
			}
			else if (boost::algorithm::iequals(lang, "cmake") 
				|| boost::algorithm::iequals(lang, "CMake") 
				|| boost::algorithm::iequals(lang, "CMakeLists.txt")) {
				//	CMake
				std::ifstream  ifs(p.string());
				ifs >> std::noskipws;
				auto [start, end] = cyng::utf8::get_utf8_range<std::istream_iterator<char>>(ifs);
				if (ifs.is_open()) cmake_to_html(os, start, end, numbers);
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
