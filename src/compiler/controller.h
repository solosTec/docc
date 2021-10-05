/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_CONTROLLER_H
#define DOCSCRIPT_CONTROLLER_H

#include <context.h>

#include <vector>

namespace docscript {

	class controller
	{
	public:
		controller(std::filesystem::path const& temp
			, std::filesystem::path out
			, std::vector<std::filesystem::path> inc
			, int verbose);

		int run(std::filesystem::path&& inp, std::size_t pool_size);

	private:

		context ctx_;

	};
}

#endif
