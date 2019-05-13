/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_FILTER_TEXT_TO_HTML_H
#define DOCSCRIPT_FILTER_TEXT_TO_HTML_H

#include <cyng/log/severity.h>
#include <cstdint>
#include <set>
#include <boost/uuid/uuid.hpp>

namespace docscript
{

	class text_to_html
	{
	public:
		text_to_html(bool linenumbers, boost::uuids::uuid);
		void convert(std::ostream&, std::string const&);

	private:
		void convert(std::ostream& os, std::string::const_iterator begin, std::string::const_iterator end);
		void write_nl(std::size_t linenumber, std::ostream& os);

	private:

		bool const linenumbers_;
		boost::uuids::uuid const tag_;
	};

}	

#endif