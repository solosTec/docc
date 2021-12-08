/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_CODE_JSON_H
#define HTML_CODE_JSON_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>
#include <cyng/parse/json/json_parser.h>
#include <cyng/io/serializer/json_walker.h>

#include <iostream>
#include <fstream>
#include <iterator>

namespace dom
{
	void json_to_html(std::ostream&, std::istream_iterator<char>, std::istream_iterator<char>, bool numbers);
}

#endif
