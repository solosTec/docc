/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_RT_CONTROLLER_H
#define DOCSCRIPT_RT_CONTROLLER_H

#include <docc/context.h>
#include <asm/reader.h>

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>

#include <filesystem>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/random_generator.hpp>

namespace docruntime {

	class controller
	{
	public:
		controller(std::filesystem::path out
			, std::vector<std::filesystem::path> inc
			, std::filesystem::path const& tmp_asm
			, std::filesystem::path const& tmp_latex
			, int verbose);

		int run(std::filesystem::path&& inp, std::size_t pool_size
			, boost::uuids::uuid tag);

	private:
		void emit_header(cyng::param_map_t& meta);

	private:
		std::ofstream ofs_;
		std::ofstream tmp_tex_;
		std::filesystem::path const tmp_tex_path_;
		docscript::context ctx_;
		docasm::reader	assembler_;
	};


}

#endif
