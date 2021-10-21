/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_RT_CONTROLLER_H
#define DOCSCRIPT_RT_CONTROLLER_H

#include <cyng/obj/intrinsics/container.h>

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
		std::string quote(cyng::vector_t);
		std::string italic(cyng::vector_t);
		std::string bold(cyng::vector_t);
		std::string paragraph(cyng::vector_t);

		void label(cyng::vector_t);
		std::string ref(cyng::vector_t);
		std::string h1(cyng::vector_t);
		std::string h2(cyng::vector_t);
		std::string h3(cyng::vector_t);
		std::string h4(cyng::vector_t);
		std::string h5(cyng::vector_t);
		std::string h6(cyng::vector_t);
		std::string header(cyng::param_map_t);

		void resource(cyng::param_map_t);
		std::chrono::system_clock::time_point now(cyng::param_map_t);

		void set(cyng::param_map_t);

	private:
		cyng::param_map_t vars_;
	};

	std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext);

}

#endif
