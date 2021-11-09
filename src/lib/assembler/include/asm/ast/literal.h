/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_LITERAL_H
#define DOCC_ASM_AST_LITERAL_H

#include <asm/ast/label.h>

#include <string>
#include <iostream>

namespace docasm {
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