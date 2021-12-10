
#include <html/code_binary.h>
#include <html/formatting.h>

#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

namespace dom
{

	void binary_to_html(std::ostream& os, std::istream_iterator<char> pos, std::istream_iterator<char> last, bool numbers) {

		std::size_t width = 16;
		bool gap = false;
		std::size_t	count = 0;
		std::stringstream	ascii_values;

		os << "<table style=\"tab-size: 2;\" class=\"docc-code\" >" << std::endl << "<tbody style=\"white-space: pre;\">" << std::endl;

		//	There is an upper limit of 65.535 lines
		for (; pos != last && count < 0xFFFF; ++pos, ++count)
		{
			//	detect start of a new line
			if ((count % width) == 0)
			{
				os << "<tr>";

				//	off
				gap = false;

				if (numbers) {
					//	dump prefix
					os
						<< "<td class=\"docc-num\" data-line-number=\""
						<< "["
						<< std::setw(4)
						<< std::setfill('0')
						<< std::hex
						<< count
						<< "]\"></td>"
						//<< std::flush	//	7 characters in total
						;
				}

				os << "<td class=\"docc-code\">";

				//	margin
				ascii_values
					<< ' '
					<< ' '
					;
			}

			if (((count % width) != 0) && ((count % (width / 2)) == 0))
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
				<< +(*pos & std::numeric_limits<char>::max())
				<< std::dec	//	reset
				;

			ascii_values
				<< (((*pos >= 0x20) && (*pos <= 0x7e)) ? *pos : '.')
				;

			//	detect end of line
			if ((count > 0) && ((count % width) == (width - 1)))
			{
				//	on
				gap = true;

				os
					<< "<span class=\"docc-comment\">"
					<< ascii_values.str()
					<< "</span>"
					<< "</td>"
					<< "</tr>"
					<< std::endl
					;
				ascii_values.str("");

			}
		}

		//	print remaining ascii values
		auto const r = ascii_values.str();

		//	calculate padding spaces
		auto tmp = (width - (count % width)) * 3;
		tmp -= 2;
		while (tmp-- > 0) {
			os << ' ';
		}

		if (!gap) {
			os << ' ';

		}

		os
			<< "  "
			<< r
			<< std::endl
			;

		//	incomplete
		if (pos != last) {
			os << "<tr>";
			if (numbers) {
				os
					<< "<td class=\"docc-num\" data-line-number=\""
					<< "["
					<< std::setw(4)
					<< std::setfill('0')
					<< std::hex
					<< count
					<< "]\"></td>"
					;
			}

			os 
				<< "<td class=\"docc-string\">"
				<< "  Content was truncated"
				<< "</td>"
				<< "</tr>"
				<< std::endl
				;


		}

		os << "</tbody>" << std::endl << "</table>" << std::endl;

	}


}
