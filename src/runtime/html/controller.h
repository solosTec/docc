/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_RT_CONTROLLER_H
#define DOCSCRIPT_RT_CONTROLLER_H

#include <toc.h>
#include <context.h>

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
			, int verbose);

		int run(std::filesystem::path&& inp, std::size_t pool_size
			, boost::uuids::uuid tag);

	private:
		std::string quote(cyng::vector_t);
		std::string italic(cyng::vector_t);
		std::string bold(cyng::vector_t);
		std::string typewriter(cyng::vector_t);
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

		std::string figure(cyng::param_map_t);

		void resource(cyng::param_map_t);
		std::chrono::system_clock::time_point now(cyng::param_map_t);
		boost::uuids::uuid uuid(cyng::param_map_t);

		void set(cyng::param_map_t);
		void meta(cyng::param_map_t);
		cyng::vector_t get(cyng::vector_t);
		cyng::vector_t range(cyng::vector_t);
		std::string cat(cyng::vector_t);
		std::string repeat(cyng::param_map_t pm);
		std::string currency(cyng::param_map_t pm);

	private:
		std::function<std::string(cyng::vector_t)> f_quote();
		std::function<void(cyng::param_map_t)> f_set();
		std::function<cyng::vector_t(cyng::vector_t)> f_get();
		std::function<void(cyng::param_map_t)> f_meta();
		std::function<std::string(cyng::vector_t)> f_paragraph();
		std::function<std::string(cyng::vector_t)> f_italic();
		std::function<std::string(cyng::vector_t)> f_bold();
		std::function<std::string(cyng::vector_t)> f_typewriter();
		std::function<void(cyng::vector_t)> f_label();
		std::function<std::string(cyng::vector_t)> f_ref();
		std::function<std::string(cyng::vector_t)> f_h1();
		std::function<std::string(cyng::vector_t)> f_h2();
		std::function<std::string(cyng::vector_t)> f_h3();
		std::function<std::string(cyng::vector_t)> f_h4();
		std::function<std::string(cyng::vector_t)> f_h5();
		std::function<std::string(cyng::vector_t)> f_h6();
		std::function<std::string(cyng::param_map_t)> f_header();
		std::function<std::string(cyng::param_map_t)> f_figure();
		std::function<void(cyng::param_map_t)> f_resource();
		std::function<std::chrono::system_clock::time_point(cyng::param_map_t)> f_now();
		std::function<boost::uuids::uuid(cyng::param_map_t)> f_uuid();
		std::function<cyng::vector_t(cyng::vector_t)> f_range();
		std::function<std::string(cyng::vector_t)> f_cat();
		std::function<std::string(cyng::param_map_t pm)> f_repeat();
		std::function<std::string(cyng::param_map_t)> f_currency();
		std::function<void(std::string)> f_show();

	private:
		cyng::param_map_t vars_;
		docruntime::toc toc_;
		boost::uuids::random_generator	uuid_gen_;	//	basic_random_generator<mt19937>
	};

	std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext);

}

#endif
