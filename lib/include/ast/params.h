/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_PARAMS_H
#define DOCC_SCRIPT_AST_PARAMS_H

#include <cstdint>
#include <limits>
#include <iostream>
#include <string>
#include <memory>

#include <fmt/core.h>
#include <fmt/color.h>

#include <ast/value.h>


namespace docscript {
	struct symbol;
	namespace ast {

		class value;

		/**
		 * parameter list
		 */
		class param {
			friend struct fmt::formatter<param>;

		public:
			param(std::string const&, value&&);
			param(std::string const&);
			param(param&&) noexcept;
			~param();

			/**
			 * Generate a complete parameter
			 */
			param finish(value&&);

			/**
			 * Append a new node to the list
			 */
			void append(param&&);
			void compile();

			static param factory(symbol const&);

			friend std::ostream& operator<<(std::ostream& os, param const&);

		private:
			std::string const key_;
			value value_;
			std::unique_ptr<param> next_;
		};

	}

}

#endif