/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_JUMP_H
#define DOCC_ASM_AST_JUMP_H

#include <asm/ast/label.h>

#include <cyng/obj/intrinsics/op.h>

namespace docasm {
	class context;
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- jump
		//	----------------------------------------------------------*
		//
		struct jump {

			cyng::op const code_;
			std::string const label_;

			[[nodiscard]] static jump factory(cyng::op);

			friend std::ostream& operator<<(std::ostream& os, jump const& c);

			/**
			 * Generate a complete forward operation
			 */
			[[nodiscard]] jump finish(std::string const&);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};


	}
}

#endif