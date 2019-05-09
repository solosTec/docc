/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "text_to_html.h"

namespace docscript
{
	text_to_html::text_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}

	void text_to_html::convert(std::ostream& os, std::string const& inp)
	{
	}
}


