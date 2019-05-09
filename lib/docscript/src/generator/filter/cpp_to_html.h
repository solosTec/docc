/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_FILTER_CPP_TO_HTML_H
#define DOCSCRIPT_FILTER_CPP_TO_HTML_H

#include <cyng/log/severity.h>
#include <cstdint>
#include <set>
#include <boost/uuid/uuid.hpp>

namespace docscript
{

	/**
	 * pretty print C++ code in HTML
	 */
	class cpp_to_html
	{
	public:
		cpp_to_html(bool linenumbers, boost::uuids::uuid);
		void convert(std::ostream&, std::string const&);

	private:

		bool const linenumbers_;
		boost::uuids::uuid const tag_;


	private:
		void print_error(cyng::logging::severity level, std::string msg);
		void write_nl(std::size_t, std::ostream&);
		void write_entity(std::ostream&, std::string);

		static const std::string color_green_;
		static const std::string color_blue_;
		static const std::string color_grey_;
		static const std::string color_red_;
		static const std::string color_cyan_;
		static const std::string color_brown_;
		static const std::string end_;

	};

}

#endif	




