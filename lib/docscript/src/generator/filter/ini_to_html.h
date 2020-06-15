/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_FILTER_INI_TO_HTML_H
#define DOCSCRIPT_FILTER_INI_TO_HTML_H

#include <cyng/log/severity.h>
#include <cstdint>
#include <set>
#include <boost/uuid/uuid.hpp>

namespace docscript
{

	/**
	 * pretty print INI file in HTML
	 */
	class ini_to_html
	{
	public:
		ini_to_html(bool linenumbers, boost::uuids::uuid);
		void convert(std::ostream&, std::string const&);

	private:

		bool const linenumbers_;
		boost::uuids::uuid const tag_;


	private:
		void convert(std::ostream& os, std::string::const_iterator begin, std::string::const_iterator end);
		std::string::const_iterator comment(std::ostream& os, std::string::const_iterator begin, std::string::const_iterator end);
		std::string::const_iterator section(std::ostream& os, std::string::const_iterator begin, std::string::const_iterator end);

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




