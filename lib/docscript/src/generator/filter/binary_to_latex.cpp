/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Sylko Olzscher
 *
 */

#include "binary_to_latex.h"
#include <sstream>
#include <iomanip>
#include <bitset>

namespace docscript
{
// 	std::string to_latex(char c)
// 	{
// 		switch (c) {
// 	
// 		case '_':	return "\\_";
// 		case '#':	return "\\#";
// 		case '%':	return "\\%";
// 		case '$':	return "\\$";
// 		case '{':	return "\\{";
// 		case '}':	return "\\}";
// 		//	not required in verbatim mode
// // 		case '\\':	return "\\\\";
// 		default:
// 			break;
// 		}
// 
// 		return ((c >= 0x20) && (c <= 0x7e))
// 			? std::string(1, c)
// 			: "."
// 			;
// 	}

	binary_to_latex::binary_to_latex(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
		, width_(16)
	{}

	void binary_to_latex::convert(std::ostream& os, cyng::buffer_t const& inp)
	{
// 		os << std::string(3, '`') << std::endl;
		convert(os, inp.begin(), inp.end());
// 		os << std::string(3, '`') << std::endl;
	}

	void binary_to_latex::convert(std::ostream& os, cyng::buffer_t::const_iterator begin, cyng::buffer_t::const_iterator end)
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
				<< std::setfill('0')
				<< std::setw(2)
				<< std::hex
				<< ((*pos) & 0xFF)
				<< std::dec	//	reset
				;

			//
			//	LaTeX entities
			//
			ascii_values << *pos;
// 			ascii_values << to_latex(*pos);

			//	detect end of line
			if ((count_ > 0) && ((count_ % width_) == (width_ - 1)))
			{
				//	on
				gap = true;

				os
					<< ascii_values.str()
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


