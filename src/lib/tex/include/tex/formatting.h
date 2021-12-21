/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef TEX_FORMATTING_H
#define TEX_FORMATTING_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>

#include <list>
#include <iostream>
#include <chrono>

namespace tex
{
	void to_tex(std::ostream&, double);
	void to_tex(std::ostream& os, std::chrono::system_clock::time_point);

	/**
	 * convert an object vector
	 */
	void to_tex(std::ostream& os, cyng::vector_t const& vec, std::string sep);
	std::string to_tex(cyng::vector_t const& vec, std::string sep);

	/**
	 * substitite TeX entities
	 */
	void esc_tex(std::ostream&, std::string const&);

}

#endif
