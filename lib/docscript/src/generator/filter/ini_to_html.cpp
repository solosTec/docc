/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "ini_to_html.h"
#include <docscript/sanitizer.h>

#include <cctype>
#include <iomanip>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>

namespace docscript
{
	ini_to_html::ini_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}

	void ini_to_html::convert(std::ostream& os, std::string const& inp)
	{
		convert(os, inp.begin(), inp.end());
	}

	void ini_to_html::convert(std::ostream& os, std::string::const_iterator pos, std::string::const_iterator end)
	{

		std::size_t linenumber{ 0 };
		write_nl(linenumber++, os);

		std::string line;

		while (pos != end) {
			switch (*pos) {
			case '\n':
				write_nl(++linenumber, os);
				os << line;
				line.clear();
				break;
			case '\r':
				break;
			case '<':
				line.append("&lt;");
				break;
			case '>':
				line.append("&gt;");
				break;
			case '"':
				line.append("&quot;");
				break;
			case '&':
				line.append("&amp;");
				break;
			case '#':
				if (line.empty()) {
					write_nl(++linenumber, os);
					pos = comment(os, pos, end);
				}
				else {
					line += *pos;
				}
				break;
			case '[':
				if (line.empty()) {
					write_nl(++linenumber, os);
					pos = section(os, ++pos, end);
				}
				else {
					line += *pos;
				}
				break;
			default:
				line += *pos;
				break;
			}

			//
			//	next char
			//
			++pos;
		}

		os << line << std::endl;

	}

	std::string::const_iterator ini_to_html::comment(std::ostream& os, std::string::const_iterator pos, std::string::const_iterator end)
	{
		os
			<< color_green_
			;

		while (pos != end) {
			switch (*pos) {
			case '\n':
			case '\r':
				os << end_;
				return pos;
			default:
				os << *pos;
				break;
			}

			//
			//	next char
			//
			++pos;

		}
		return pos;
	}

	std::string::const_iterator ini_to_html::section(std::ostream& os, std::string::const_iterator pos, std::string::const_iterator end)
	{
		os
			<< color_red_
			<< '['
			;

		while (pos != end) {
			switch (*pos) {
			case '\n':
			case '\r':
				return pos;
			case ']':
				os 
					<< end_
					<< *pos
					;
				return ++pos;
			default:
				os << *pos;
				break;
			}

			//
			//	next char
			//
			++pos;

		}

		return pos;
	}


	void ini_to_html::write_nl(std::size_t linenumber, std::ostream& os)
	{
		if (linenumber != 0) {
			os 
				<< "</code>"
				<< std::endl
				<< "<code contenteditable spellcheck=\"false\">"
				;
		}
		else {
			os 
				<< "<code contenteditable spellcheck=\"false\">";
		}

		if (linenumbers_) {
			os
				<< "<span style = \"color: DarkCyan; font-size: smaller;\" id=\""
				<< tag_
				<< '-'
				<< linenumber
				<< "\">"
				<< std::setw(4)
				<< std::setfill(' ')
				<< linenumber
				<< "</span> "
				;
		}
	}

	void ini_to_html::write_entity(std::ostream& os, std::string entity)
	{
		for (auto const c : entity) {
			switch (c) {
			case '<':
				os << "&lt;";
				break;
			case '>':
				os << "&gt;";
				break;
			case '&':
				os << "&amp;";
				break;
			case '"':
				os << "&quot;";
				break;
			default:
				os << c;
				break;
			}
		}
	}


	void ini_to_html::print_error(cyng::logging::severity level, std::string msg)
	{
		std::cerr << msg << std::endl;
	}

	//
	//	constants
	//
	std::string const ini_to_html::color_green_ = "<span style=\"color: green;\">";
	std::string const ini_to_html::color_blue_ = "<span style=\"color: blue;\">";
	std::string const ini_to_html::color_grey_ = "<span style=\"color: grey;\">";
	std::string const ini_to_html::color_red_ = "<span style=\"color: sienna;\">";
	std::string const ini_to_html::color_cyan_ = "<span style=\"color: DarkCyan; font-size: smaller;\">";
	std::string const ini_to_html::color_brown_ = "<span style=\"color: brown;\">";
	std::string const ini_to_html::end_ = "</span>";
}


