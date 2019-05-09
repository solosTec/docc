/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "html_to_html.h"

namespace docscript
{
	html_to_html::html_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}

	void html_to_html::convert(std::ostream& os, std::string const& inp)
	{
	}
}


