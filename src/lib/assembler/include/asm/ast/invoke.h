/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_INVOKE_H
#define DOCC_ASM_AST_INVOKE_H

#include <asm/ast/label.h>

namespace docasm {
	class context;
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- invoke
		//	----------------------------------------------------------*
		//
		struct invoke {

			std::string const name_;

			[[nodiscard]] static invoke factory();

			friend std::ostream& operator<<(std::ostream& os, invoke const& c);

			/**
			 * Generate a complete invoke operation
			 */
			[[nodiscard]] invoke finish(std::string const&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

		//
		//	----------------------------------------------------------*
		//	-- invoke_r
		//	----------------------------------------------------------*
		//
		struct invoke_r {

			std::string const name_;

			[[nodiscard]] static invoke_r factory();

			friend std::ostream& operator<<(std::ostream& os, invoke_r const& c);

			/**
			 * Generate a complete invoke operation
			 */
			[[nodiscard]] invoke_r finish(std::string const&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

	}
}

#endif