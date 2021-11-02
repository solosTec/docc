/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_LABEL_H
#define DOCC_ASM_AST_LABEL_H

#include <string>
#include <map>
#include <iostream>

namespace docasm {
	class context;
	namespace ast {

		using label_list_t = std::map<std::string, std::size_t>;

		//
		//	----------------------------------------------------------*
		//	-- label
		//	----------------------------------------------------------*
		//
		struct label {
			std::string const name_;

			[[nodiscard]] static label factory(std::string const&);

			friend std::ostream& operator<<(std::ostream& os, label const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

	}
}

#endif