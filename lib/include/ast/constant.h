/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_CONSTANT_H
#define DOCC_SCRIPT_AST_CONSTANT_H

#include <symbol.h>
//#include <cyng/obj/intrinsics/color.h>

#include <limits>
#include <chrono>
#include <variant>

#include <fmt/core.h>
#include <fmt/color.h>
#include <fmt/ostream.h>


namespace docscript {
	namespace ast {

		/**
		 * Simple text (mostly a word) or a timestamp
		 */
		struct constant {
			std::string const value_;
			std::variant<std::string, std::chrono::system_clock::time_point, bool, double> node_;

			void compile(std::function<void(std::string const&)>) const;

			[[nodiscard]] static constant factory(symbol const&);

			friend std::ostream& operator<<(std::ostream& os, constant const& c);
		};

	}

}

namespace fmt {


	template <>
	struct formatter<docscript::ast::constant> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) {
			return ctx.begin();
		}

		template <typename FormatContext>
		auto format(const docscript::ast::constant& c, FormatContext& ctx) {
			std::stringstream ss;
			ss << c;
			return format_to(ctx.out(), "{}", ss.str());
		}
	};
}  // namespace fmt

#endif