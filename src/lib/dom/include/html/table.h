/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_TABLE_H
#define HTML_TABLE_H

#include <filesystem>
#include <iostream>
#include <fstream>

namespace dom
{
	void render_table(std::ostream&, std::filesystem::path const&, bool header);
}

#endif
