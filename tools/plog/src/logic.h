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
#include <cyng/store/db.h>

namespace plog
{
	class logic 
	{
	public:
		/**
		 * Initialize database
		 */
		logic(cyng::logging::log_ptr logger);

		/**
		 * Register the same functions to different VMs
		 */
		void register_this(cyng::controller&);

	private:
		void http_session_launch(cyng::context& ctx);

		/**
		 * Cannot use the implementation of the store_domain (CYNG) because
		 * of the additional source (UUID) parameter.
		 */
		void db_req_insert(cyng::context& ctx);
		void db_req_remove(cyng::context& ctx);
		void db_req_modify_by_param(cyng::context& ctx);

	private:
		cyng::logging::log_ptr logger_;

		/**
		 * global data cache
		 */
		cyng::store::db db_;

	};
}

#endif
