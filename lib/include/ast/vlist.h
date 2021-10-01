/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_VLIST_H
#define DOCC_SCRIPT_AST_VLIST_H

#include <cstdint>
#include <limits>
#include <iostream>
#include <string>
#include <chrono>
#include <variant>

#include <symbol.h>
#include <ast/value.h>


namespace docscript {
	namespace ast {

		/**
		 * value list (vector)
		 */
		class vlist {
		public:
			vlist() noexcept;
			vlist(value&&) noexcept;
			vlist(vlist&&) noexcept;
			~vlist();

			static vlist factory(symbol const&);

			void compile(std::function<void(std::string const&)>) const;
			/**
			 * append to value list (vector)
			 */
			void append(value&&);

		private:
			value value_;
			std::unique_ptr<vlist> next_;

		};

		//vlist gen_vlist(symbol const&);

	}

}

#endif