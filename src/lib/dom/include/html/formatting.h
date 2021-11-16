/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_FORMATTING_H
#define HTML_FORMATTING_H

#include <list>
#include <iostream>
#include <chrono>

namespace dom
{
	void to_html(std::ostream&, double);
	void to_html(std::ostream& os, std::chrono::system_clock::time_point);

}

#endif
