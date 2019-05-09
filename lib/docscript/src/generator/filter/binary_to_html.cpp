/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "binary_to_html.h"
#include <sstream>
#include <iomanip>
#include <bitset>

namespace docscript
{
	std::string to_html(char c)
	{
		switch (c) {
		case '<':	return "&lt;";
		case '>':	return "&gt;";
		case '"':	return "&quot;";
		case '&':	return "&amp;";
		default:
			break;
		}

		return ((c >= 0x20) && (c <= 0x7e))
			? std::string(1, c)
			: "."
			;
	}

	binary_to_html::binary_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
		, width_(16)
	{}

	void binary_to_html::convert(std::ostream& os, cyng::buffer_t const& inp)
	{
		convert(os, inp.begin(), inp.end());
	}

	void binary_to_html::convert(std::ostream& os, cyng::buffer_t::const_iterator begin, cyng::buffer_t::const_iterator end)
	{
		std::stringstream	ascii_values;
		bool gap = false;
		std::ptrdiff_t count_ = 0;
		for (auto pos = begin; pos != end; ++pos, ++count_)
		{
			//	detect start of a new line
			if ((count_ % width_) == 0)
			{
				//	off
				gap = false;

				//	dump prefix
				os
					<< "<code>"
					<< "["
					<< std::setw(4)
					<< std::setfill('0')
					<< std::uppercase
					<< std::hex
					<< count_
					<< "] "		//	+2 characters
					<< std::flush	//	7 characters in total
					;

				//	margin
				ascii_values
					<< ' '
					<< ' '
					;
			}

			if (((count_ % width_) != 0) && ((count_ % (width_ / 2)) == 0))
			{
				//	on
				gap = true;

				//	gap
				os
					<< ' '	//	space
					;

				ascii_values
					<< ' '
					;
			}

			//	print hex value
			os
				<< ' '		//	space
				<< "<span title=\""
				<< std::bitset<8>((*pos) & 0xFF).to_string()
				<< " / "
				<< std::dec	
				<< ((*pos) & 0xFF)
				<< " / "
				<< to_html(*pos)
				<< "\">"
				<< std::setfill('0')
				<< std::setw(2)
				<< std::hex
				<< ((*pos) & 0xFF)
				<< "</span>"
				<< std::dec	//	reset
				;

			//
			//	HTML entities
			//
			ascii_values << to_html(*pos);

			//	detect end of line
			if ((count_ > 0) && ((count_ % width_) == (width_ - 1)))
			{
				//	on
				gap = true;

				os
					<< ascii_values.str()
					<< "</code>"
					<< std::endl
					;
				ascii_values.str("");
			}
		}

		//	print remaining ascii values
		const std::string r = ascii_values.str();
		//	calculate padding spaces
		auto tmp = (width_ - (count_ % width_)) * 3;
		tmp -= 2;
		while (tmp-- > 0)
		{
			os << ' ';
		}
		if (!gap)
		{
			os << ' ';

		}

		os
			<< "  "
			<< r
			<< std::endl
			;

	}

}


