/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_VALUE_H
#define DOCC_SCRIPT_AST_VALUE_H

#include <ast/constant.h>

namespace docscript {
	namespace ast {

		class map_method;
		class vec_method;

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

			//[[nodiscard]] value clone() const;

			void compile(std::function<void(std::string const&)>) const;

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

namespace fmt {
	template <>
	struct formatter<docscript::ast::value> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) {
			return ctx.begin();
		}

		template <typename FormatContext>
		auto format(docscript::ast::value const& val, FormatContext& ctx) {
			if (val.node_) {
				std::stringstream ss;
				ss << val;
				return format_to(ctx.out(), "value[{}]", ss.str());
			}
			return format_to(ctx.out(), "value[empty]");
		}
	};
}

#endif