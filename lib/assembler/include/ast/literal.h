/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_LITERAL_H
#define DOCC_ASM_AST_LITERAL_H

#include <ast/label.h>
//#include <symbol.h>
//
//#include <cyng/obj/intrinsics/op.h>
//#include <cyng/obj/intrinsics/color.hpp>
//
#include <string>
#include <iostream>
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
		//	-- literal
		//	----------------------------------------------------------*
		//
		struct literal {
			std::string const value_;

			[[nodiscard]] static literal factory(std::string const&);

			friend std::ostream& operator<<(std::ostream& os, literal const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

	}
}

#endif