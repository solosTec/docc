/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_PUSH_H
#define DOCC_ASM_AST_PUSH_H

//#include <ast/label.h>
//#include <ast/literal.h>
//#include <ast/operation.h>
#include <ast/value.h>
//#include <symbol.h>
//
//#include <cyng/obj/intrinsics/op.h>
//#include <cyng/obj/intrinsics/color.hpp>
//
//#include <string>
//#include <iostream>
//#include <variant>
//#include <stack>
//#include <chrono>
//#include <map>
//
//#include <boost/uuid/uuid.hpp>
//#include <boost/uuid/string_generator.hpp>

namespace docscript {
	class context;
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- push
		//	----------------------------------------------------------*
		//
		struct push {

			value const val_;

			[[nodiscard]] static push factory();

			friend std::ostream& operator<<(std::ostream& os, push const& c);

			/**
			 * Generate a complete push operation
			 */
			[[nodiscard]] push finish(value&&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

	}
}

#endif