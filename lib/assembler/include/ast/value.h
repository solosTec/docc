/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_VALUE_H
#define DOCC_ASM_AST_VALUE_H

#include <ast/label.h>
//#include <ast/literal.h>
//#include <ast/operation.h>
#include <symbol.h>

//#include <cyng/obj/intrinsics/op.h>
#include <cyng/obj/intrinsics/color.hpp>
//
//#include <string>
//#include <iostream>
#include <variant>
//#include <stack>
#include <chrono>
//#include <map>
//
#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/string_generator.hpp>

namespace docscript {
	class context;
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- value
		//	----------------------------------------------------------*
		//
		struct value {
			std::variant<
				std::monostate,		//	[0] empty
				std::string,		//	[1] txt
				std::chrono::system_clock::time_point,	//	[2]
				cyng::color_8,		//	[3]
				bool,				//	[4]
				std::uint64_t,		//	[5]
				boost::uuids::uuid	//	[6]
			> val_;

			[[nodiscard]] static value factory(symbol const&);
			[[nodiscard]] static value factory(boost::uuids::uuid);

			friend std::ostream& operator<<(std::ostream& os, value const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

	}
}

#endif