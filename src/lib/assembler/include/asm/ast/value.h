/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_VALUE_H
#define DOCC_ASM_AST_VALUE_H

#include <asm/ast/label.h>
#include <asm/symbol.h>

#include <cyng/obj/intrinsics/color.hpp>

#include <variant>
#include <chrono>

#include <boost/uuid/uuid.hpp>

namespace docasm {
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
				std::int64_t,		//	[6]
				double,				//	[7]
				boost::uuids::uuid	//	[8]
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