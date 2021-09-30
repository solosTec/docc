/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_VALUE_H
#define DOCC_SCRIPT_AST_VALUE_H

#include <cstdint>
#include <limits>
#include <iostream>
#include <string>
#include <chrono>
#include <variant>

#include <symbol.h>

#include <fmt/core.h>
#include <fmt/color.h>


namespace docscript {
	namespace ast {

		class map_method;
		class vec_method;
		struct constant;

		/**
		 * Hold a node with values: constant, method oder cite (" vector ")
		 */
		class value {
			friend struct fmt::formatter<value>;
		public:
			value() noexcept;
			value(value&&) noexcept;
			//value& operator=(value&&) = default;
			~value();

			[[nodiscard]] value clone() const;

			void compile();

			static value factory(symbol const& sym);
			static value factory(map_method&&);
			static value factory(vec_method&&);
			static value factory(constant&&);

			friend std::ostream& operator<<(std::ostream& os, value const&);

		private:
			std::size_t index() const;
		private:
			//
			//	constant
			//	method or
			//	cite (" vector ")
			//
			struct value_node;
			std::unique_ptr<value_node> node_;
			value(value_node*);
		};

	}

}

#endif