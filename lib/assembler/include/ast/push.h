/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_PUSH_H
#define DOCC_ASM_AST_PUSH_H

#include <ast/value.h>

namespace docasm {
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