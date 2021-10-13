/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_RT_CONTROLLER_H
#define DOCSCRIPT_RT_CONTROLLER_H

#include <vector>
#include <filesystem>

#include <boost/uuid/uuid.hpp>

namespace docscript {

	class controller
	{
	public:
		controller(std::filesystem::path out
			, int verbose);

		int run(std::filesystem::path&& inp, std::size_t pool_size
			, boost::uuids::uuid tag);

	private:
		void quote(std::string, std::string);
		
	};
}

#endif
