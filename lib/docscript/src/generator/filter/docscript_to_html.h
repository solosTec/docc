/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_FILTER_DOSCRIPT_TO_HTML_H
#define DOCSCRIPT_FILTER_DOSCRIPT_TO_HTML_H

#include <cyng/log/severity.h>
#include <cstdint>
#include <set>
#include <boost/uuid/uuid.hpp>

namespace docscript
{

	class docscript_to_html
	{
	public:
		docscript_to_html(bool linenumbers, boost::uuids::uuid);
		void convert(std::ostream&, std::string const&);

	private:
		void print_error(cyng::logging::severity level, std::string msg);

	private:

		bool const linenumbers_;
		boost::uuids::uuid const tag_;
	};

}

#endif