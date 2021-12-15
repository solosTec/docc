
#include <html/table.h>
#include <html/formatting.h>

#include <cyng/parse/csv/csv_parser.h>
#include <cyng/parse/csv/line_cast.hpp>
#include <cyng/parse/csv.h>
#include <cyng/io/parser/stream.hpp>
#include <cyng/io/serialize.h>

#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

namespace dom
{

	void render_table(std::ostream& os, std::filesystem::path const& p, bool header) {
		if (std::filesystem::exists(p) && std::filesystem::is_regular_file(p)) {
			//
			//	parse CSV file
			//
			cyng::csv::parser csvp(',', [&](cyng::csv::line_t&& line) {
				if (header) {
					os << "<thead>" << std::endl;
					os << "<tr>";
					for (auto const& cell : line) {
						os << "<td>";
						esc_html(os, cell);
						os << "</td>" << std::endl;

					}
					os << "</tr>" << std::endl;
					os << "</thead>" << std::endl;
					header = false;
				}
				else {
					os << "<tr class=\"docc-tr\">";
					for (auto const& cell : line) {
						os << "<td>";
						esc_html(os, cell);
						os << "</td>" << std::endl;

					}
					os << "</tr>" << std::endl;
				}
			});


			os << "<table class=\"docc-table\">" << std::endl;
			std::ifstream  ifs(p.string());
			if (ifs.is_open()) {
				ifs >> std::noskipws;

				auto [start, end] = cyng::get_stream_range<std::istream_iterator<char>>(ifs);
				csvp.read(start, end);
			}
			os << "</table>" << std::endl;
		}
		else {
			os
				<< "<div>"
				<< p.string()
				<< " not found"
				<< "</div>"
				<< std::endl;
		}
	}


}
