/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "text_to_html.h"
#include <iomanip>
#include <boost/uuid/uuid_io.hpp>

namespace docscript
{
	text_to_html::text_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}

	void text_to_html::convert(std::ostream& os, std::string const& inp)
	{
		convert(os, inp.begin(), inp.end());
	}

	void text_to_html::convert(std::ostream& os, std::string::const_iterator pos, std::string::const_iterator end)
	{
		std::size_t linenumber{ 0 };
		write_nl(linenumber++, os);
		while (pos != end) {
			switch (*pos) {
			case '\n':
				write_nl(++linenumber, os);
				break;
			case '\r':
				break;
			case '<':	
				os << "&lt;";
				break;
			case '>':	
				os << "&gt;";
				break;
			case '"':
				os << "&quot;";
				break;
			case '&':
				os << "&amp;";
				break;
			default:
				os << *pos;
				break;
			}

			//
			//	next char
			//
			++pos;
		}
	}

	void text_to_html::write_nl(std::size_t linenumber, std::ostream& os)
	{
		if (linenumber != 0) {
			os
				<< "</code>"
				<< std::endl
				<< "<code>"
				;
		}
		else {
			os << "<code>";
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

}


