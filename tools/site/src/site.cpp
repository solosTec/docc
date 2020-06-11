/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 


#include "site.h"
#include "../../src/driver.h"

#include <cyng/json.h>
#include <cyng/dom/reader.h>
#include <cyng/io/serializer.h>
#include <cyng/io/io_chrono.hpp>
#include <cyng/factory.h>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace docscript
{

	site::site(std::vector< std::string >const& inc
		, int verbose)
	: includes_(inc.begin(), inc.end())
		, verbose_(verbose)
	{}

	site::~site()
	{}

	int site::run(cyng::filesystem::path const& cfg
		, cyng::filesystem::path const& out
		, bool gen_robot
		, bool gen_sitemap)
	{

		//if (cyng::filesystem::is_directory(inp)) {

		//	for (auto const& entry : boost::make_iterator_range(cyng::filesystem::directory_iterator(inp), {})) {

		//		process_file(entry, out);
		//	}

		//	//
		//	//	generate index file
		//	//
		//	generate_index(out, gen_robot, gen_sitemap);

		//	return EXIT_SUCCESS;
		//}

		//
		//	not a directory
		//
		//std::cerr << "***error: " << inp << " is not a directory" << std::endl;
		return EXIT_FAILURE;
	}
}
