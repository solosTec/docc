/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_PLOG_LOGIC_H
#define DOCSCRIPT_PLOG_LOGIC_H

#include <cyng/log.h>
#include <cyng/vm/controller.h>
//#include <string>
//#include <cstdint>

namespace plog
{
	class logic 
	{
	public:
		/**
		 */
		logic(cyng::logging::log_ptr logger);
		void register_this(cyng::controller&);

	private:
		void http_session_launch(cyng::context& ctx);

	private:
		cyng::logging::log_ptr logger_;
	};
}

#endif
