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

			std::size_t compile(std::function<void(std::string const&)>, std::size_t depth, std::size_t index) const;
			std::size_t size() const;

			/**
			 * append to value list (vector)
			 */
			std::size_t append(value&&);

		private:
			value value_;
			std::unique_ptr<vlist> next_;

		};
	}
}

#endif