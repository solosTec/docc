/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "sml_to_html.h"
#include <smf/sml/protocol/parser.h>
#include <smf/sml/obis_db.h>

#include <cyng/io/serializer.h>
#include <cyng/io/io_buffer.h>
#include <cyng/core/class_interface.h>
#include <cyng/intrinsics/traits/tag.hpp>

#include <iostream>
#include <iomanip>

#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace docscript	
{
	sml_to_html::sml_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}


	void sml_to_html::convert(std::ostream& os, cyng::buffer_t const& inp)
	{
		convert(os, inp.begin(), inp.end());
	}

	void sml_to_html::convert(std::ostream& os, cyng::buffer_t::const_iterator begin, cyng::buffer_t::const_iterator end)
	{
		os
			<< "<p>"
			<< std::distance(begin, end)
			<< " bytes SML"
			<< "</p>"
			<< std::endl
			;

		std::size_t prev_pos{ 0u };

		node::sml::parser sml([&](cyng::vector_t&& prg) {
			cyng::vector_t vec = std::move(prg);
			//	[37,3,5,0500153B01EAD1]
			//	position, depth, index, data
			std::cout << "data: " << cyng::io::to_str(vec) << std::endl;
			if (vec.size() == 4) {

				auto const tag = vec.at(3).get_class().tag();
				os << "<code>";

				auto const pos = cyng::value_cast<std::size_t>(vec.at(0), 0u);

				if (prev_pos > pos) {
					os
						<< "</code>"
						<< std::endl
						<< "<code>"
						;
				}
				prev_pos = pos;

				if (linenumbers_)
				{
					os
						<< "<span style = \"color: DarkCyan; font-size: smaller;\" id=\""
						<< tag_
						<< '-'
						<< prev_pos
						<< "\">"
						<< std::setw(4)
						<< std::setfill(' ')
						<< prev_pos
						<< "</span> "
						;
				}

				auto const depth = cyng::value_cast<std::size_t>(vec.at(1), 0u);
				auto const idx = cyng::value_cast<std::size_t>(vec.at(2), 0u) - 1u;

				os
					<< std::setw(depth * 2)
					<< std::setfill('.')
					<< idx
					<< ' '
					;

				switch (vec.at(3).get_class().tag()) {
				case cyng::TC_UINT8:
					os 
						<< "[<span style=\"color: blue\">u8</span>] "
						<< cyng::io::to_str(vec.at(3))
						;
					break;
				case cyng::TC_UINT16:
					os 
						<< "[<span style=\"color: blue\">u16</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;
				case cyng::TC_UINT32:
					os 
						<< "[<span style=\"color: blue\">u32</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;
				case cyng::TC_UINT64:
					os 
						<< "[<span style=\"color: blue\">u64</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;

				case cyng::TC_INT8:
					os 
						<< "[<span style=\"color: navy\">i8</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;
				case cyng::TC_INT16:
					os 
						<< "[<span style=\"color: navy\">i16</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;
				case cyng::TC_INT32:
					os 
						<< "[<span style=\"color: navy\">i32</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;
				case cyng::TC_INT64:
					os 
						<< "[<span style=\"color: navy\">i64</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;

				case cyng::TC_BOOL:
					os 
						<< "[<span style=\"color: darkslategray \">bool</span>] "
						<< cyng::io::to_str(vec.at(3));
					break;
				case cyng::TC_BUFFER:
					os 
						<< "[<span style=\"color: sienna\">octet</span>] "
						<< cyng::io::to_str(vec.at(3));
					{
						cyng::buffer_t octet;
						octet = cyng::value_cast(vec.at(3), octet);
						if (cyng::is_ascii(octet)) {
							os
								<< " \"<span style=\"color: green\">"
								<< cyng::io::to_ascii(octet)
								<< "</span>\""
								;
						}
						else if (octet.size() == 6) {
							auto const name = node::sml::get_name(node::sml::obis(octet));
							if (name != "no-entry") {
								os
									<< " '<span style=\"color: olivedrab; font-weight: bold;\">"
									<< name
									<< "</span>'"
									;
							}
						}
					}
					break;
				case cyng::TC_NULL:
					os
						<< "[<span style=\"color: darkgreen\">option</span>] "
						//<< cyng::io::to_str(vec.at(3))
						;
					break;

				case cyng::TC_TUPLE:
					break;

				default:
					os << "[" << vec.at(3).get_class().type_name() << "]";
					break;
				}
				os
					<< "</code>"
					<< std::endl
					;
			}
		}, false, false, true);	//	debug, no logger

		sml.read(begin, end);

		//sml_walker walker(os, linenumbers_, tag_);
		//traverse(obj, walker, 0);

	}

}


