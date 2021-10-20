/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_PARAMS_H
#define DOCC_SCRIPT_AST_PARAMS_H

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
			 * This is precondition to call the finish() method.
			 *
			 * @return true, if not only the but also the value
			 * is defined.
			 */
			bool is_complete() const;

			/**
			 * Generate a complete parameter
			 */
			param finish(value&&);

			/**
			 * Append a new node to the list
			 */
			void append(param&&);
			std::size_t size() const;
			std::size_t compile(std::function<void(std::string const&)>, std::size_t depth, std::size_t index) const;

			static param factory(symbol const&);

			friend std::ostream& operator<<(std::ostream& os, param const&);

		private:
			std::string const key_;
			value value_;
			std::unique_ptr<param> next_;
		};

	}

}

namespace fmt {
	template <>
	struct formatter<docscript::ast::param> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) {
			return ctx.begin();
		}

		template <typename FormatContext>
		auto format(docscript::ast::param const& p, FormatContext& ctx) {
			std::stringstream ss;
			ss << p;
			return format_to(ctx.out(), "param[{}]", ss.str());
		}
	};
}

#endif