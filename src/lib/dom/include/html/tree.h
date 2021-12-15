/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef HTML_TREE_H
#define HTML_TREE_H

#include <filesystem>
#include <iostream>
#include <fstream>

namespace dom
{
	void render_tree(std::ostream&, std::filesystem::path const& root);
}

#endif
