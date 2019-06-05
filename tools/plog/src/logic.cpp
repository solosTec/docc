/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include "logic.h"
#include <cyng/io/serializer.h>

namespace plog 
{
	logic::logic(cyng::logging::log_ptr logger)
		: logger_(logger)
	{}

	void logic::register_this(cyng::controller& vm)
	{
		vm.register_function("http.session.launch", 0, std::bind(&logic::http_session_launch, this, std::placeholders::_1));

	}

	void logic::http_session_launch(cyng::context& ctx)
	{
		const cyng::vector_t frame = ctx.get_frame();
		CYNG_LOG_INFO(logger_, ctx.get_name() << " " << cyng::io::to_str(frame));

	}

}
