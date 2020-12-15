/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include "logic.h"
#include <smf/shared/db_schemes.h>
#include <cyng/io/serializer.h>
#include <cyng/tuple_cast.hpp>
#include <cyng/table/key.hpp>
#include <cyng/table/body.hpp>

namespace plog 
{
	logic::logic(cyng::logging::log_ptr logger)
		: logger_(logger)
		, db_()
	{
		if (!node::create_table(db_, "_HTTPSession", false))
		{
			CYNG_LOG_FATAL(logger_, "cannot create table _HTTPSession");
		}
	}

	void logic::register_this(cyng::controller& vm)
	{
		vm.register_function("http.session.launch", 0, std::bind(&logic::http_session_launch, this, std::placeholders::_1));
		vm.register_function("db.req.insert", 4, std::bind(&logic::db_req_insert, this, std::placeholders::_1));
		vm.register_function("db.req.remove", 3, std::bind(&logic::db_req_remove, this, std::placeholders::_1));
		vm.register_function("db.req.modify.by.param", 5, std::bind(&logic::db_req_modify_by_param, this, std::placeholders::_1));

	}

	void logic::http_session_launch(cyng::context& ctx)
	{
		const cyng::vector_t frame = ctx.get_frame();
		CYNG_LOG_INFO(logger_, ctx.get_name() << " " << cyng::io::to_str(frame));

	}

	void logic::db_req_insert(cyng::context& ctx)
	{
		const cyng::vector_t frame = ctx.get_frame();
		//
		//	[*Session,[2ce46726-6bca-44b6-84ed-0efccb67774f],[00000000-0000-0000-0000-000000000000,2018-03-12 17:56:27.10338240,f51f2ae7,data-store,eaec7649-80d5-4b71-8450-3ee2c7ef4917,94aa40f9-70e8-4c13-987e-3ed542ecf7ab,null,session],1]
		//
		//	* table name
		//	* record key
		//	* record data
		//	* generation
		//	* source
		//	
		CYNG_LOG_TRACE(logger_, "db.req.insert - " << cyng::io::to_str(frame));

		//
		//	note: this cast is different from the cast in the store_domain (CYNG).
		//
		auto tpl = cyng::tuple_cast<
			std::string,			//	[0] table name
			cyng::table::key_type,	//	[1] table key
			cyng::table::data_type,	//	[2] record
			std::uint64_t,			//	[3] generation
			boost::uuids::uuid		//	[4] source - explicit
		>(frame);

		//
		//	assemble a record
		//
		std::reverse(std::get<1>(tpl).begin(), std::get<1>(tpl).end());
		std::reverse(std::get<2>(tpl).begin(), std::get<2>(tpl).end());

		db_.insert(std::get<0>(tpl)	//	[0] table name
			, std::get<1>(tpl)		//	[1] table key
			, std::get<2>(tpl)		//	[2] record
			, std::get<3>(tpl)		//	[3] generation
			, std::get<4>(tpl));	//	[4] origin	
	}

	void logic::db_req_remove(cyng::context& ctx)
	{
		const cyng::vector_t frame = ctx.get_frame();
		//
		//	[*Session,[e72bc048-cb37-4a86-b156-d07f22608476]]
		//
		//	* table name
		//	* record key
		//	* source
		//	
		CYNG_LOG_TRACE(logger_, "db.req.remove - " << cyng::io::to_str(frame));

		auto tpl = cyng::tuple_cast<
			std::string,			//	[0] table name
			cyng::table::key_type,	//	[1] table key
			boost::uuids::uuid		//	[2] source
		>(frame);

		//
		//	reordering table key
		//	
		std::reverse(std::get<1>(tpl).begin(), std::get<1>(tpl).end());

		db_.erase(std::get<0>(tpl)		//	[0] table name
			, std::get<1>(tpl)		//	[1] table key
			, std::get<2>(tpl));	//	[2] source

	}

	void logic::db_req_modify_by_param(cyng::context& ctx)
	{
		//	
		//	[*Session,[35d1d76d-56c3-4df7-b1ff-b7ad374d2e8f],("rx":33344),327,35d1d76d-56c3-4df7-b1ff-b7ad374d2e8f]
		//	[*Cluster,[1e4527b3-6479-4b2c-854b-e4793f40d864],("ping":00:00:0.003736),4,1e4527b3-6479-4b2c-854b-e4793f40d864]
		//
		//	* table name
		//	* record key
		//	* param [column,value]
		//	* generation
		//	* source
		//	
		const cyng::vector_t frame = ctx.get_frame();
		CYNG_LOG_TRACE(logger_, "db.req.modify.by.param - " << cyng::io::to_str(frame));

		auto tpl = cyng::tuple_cast<
			std::string,			//	[0] table name
			cyng::table::key_type,	//	[1] table key
			cyng::param_t,			//	[2] parameter
			std::uint64_t,			//	[3] generation
			boost::uuids::uuid		//	[4] source
		>(frame);

		//
		//	reordering table key
		//	
		std::reverse(std::get<1>(tpl).begin(), std::get<1>(tpl).end());

		db_.modify(std::get<0>(tpl)		//	[0] table name
			, std::get<1>(tpl)			//	[1] table key
			, std::get<2>(tpl)		//	[2] parameter
			//, std::get<3>(tpl)		//	[3] generation
			, std::get<4>(tpl));	//	[4] source
	}

}
