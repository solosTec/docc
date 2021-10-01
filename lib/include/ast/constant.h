/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_CONSTANT_H
#define DOCC_SCRIPT_AST_CONSTANT_H

#include <cstdint>
#include <limits>
#include <iostream>
#include <string>
#include <chrono>
#include <variant>

#include <symbol.h>


namespace docscript {
	namespace ast {

		/**
		 * Simple text (mostly a word) or a timestamp
		 */
		struct constant {
			std::string const value_;
			std::variant<std::string, std::chrono::system_clock::time_point, bool, double> node_;

			void compile();

			[[nodiscard]] static constant factory(symbol const&);
		};

		//std::ostream& operator<<(std::ostream& os, constant c);
	}

}

#endif