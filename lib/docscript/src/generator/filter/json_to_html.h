/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_FILTER_JSON_TO_HTML_H
#define DOCSCRIPT_FILTER_JSON_TO_HTML_H


#include <cstdint>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace docscript
{

	class json_to_html
	{
	public:
		json_to_html(bool linenumbers, boost::uuids::uuid);
		void convert(std::ostream&, std::string const&);

	private:
		bool const linenumbers_;
		boost::uuids::uuid const tag_;
	};

}

#endif




