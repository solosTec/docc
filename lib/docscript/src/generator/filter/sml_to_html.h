/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_FILTER_SML_TO_HTML_H
#define DOCSCRIPT_FILTER_SML_TO_HTML_H


#include <cyng/intrinsics/buffer.h>
#include <cstdint>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace docscript
{

	class sml_to_html
	{
	public:
		sml_to_html(bool linenumbers, boost::uuids::uuid);
		void convert(std::ostream&, cyng::buffer_t const&);

	private:
		void convert(std::ostream&, cyng::buffer_t::const_iterator, cyng::buffer_t::const_iterator);

	private:
		bool const linenumbers_;
		boost::uuids::uuid const tag_;
	};

}

#endif




