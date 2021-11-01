/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_OPERATION_H
#define DOCC_ASM_AST_OPERATION_H

#include <ast/label.h>
#include <cyng/obj/intrinsics/op.h>

namespace docscript {
	class context;
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- operation
		//	----------------------------------------------------------*
		//
		struct operation {
			cyng::op const code_;

			[[nodiscard]] static operation factory(cyng::op);

			friend std::ostream& operator<<(std::ostream& os, operation const& c);

			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

	}
}

#endif