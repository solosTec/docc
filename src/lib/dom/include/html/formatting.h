/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_FORMATTING_H
#define HTML_FORMATTING_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>

#include <list>
#include <iostream>
#include <chrono>

namespace dom
{
	void to_html(std::ostream&, double);
	void to_html(std::ostream& os, std::chrono::system_clock::time_point);
	/**
	 * convert an object vector
	 */
	void to_html(std::ostream& os, cyng::vector_t const& vec, std::string sep);
	std::string to_html(cyng::vector_t const& vec, std::string sep);

}

#endif
