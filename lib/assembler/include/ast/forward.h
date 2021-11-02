/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_AST_FORWARD_H
#define DOCC_ASM_AST_FORWARD_H

#include <ast/label.h>

#include <boost/uuid/uuid.hpp>

namespace docasm {
	class context;
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- forward
		//	----------------------------------------------------------*
		//
		struct forward {

			boost::uuids::uuid const tag_;

			[[nodiscard]] static forward factory();

			friend std::ostream& operator<<(std::ostream& os, forward const& c);

			/**
			 * Generate a complete forward operation
			 */
			[[nodiscard]] forward finish(boost::uuids::uuid);


			std::size_t size() const;
			void generate(context&, label_list_t const&) const;
		};

	}
}

#endif