/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_AST_METHOD_H
#define DOCC_SCRIPT_AST_METHOD_H

#include <ast/params.h>
#include <method.h>

namespace docscript {

	namespace ast {
		class method_base {
		public:
			method_base(std::string const& name, std::optional<docscript::method> optm);
			std::string const& get_name() const;
		protected:
			std::string const name_;
			std::optional<docscript::method>	method_;
		};

		class map_method : public method_base {
			friend struct fmt::formatter<value>;
		public:
			map_method(std::string const& name, std::optional<docscript::method> optm);
			map_method(map_method&&) noexcept;
			~map_method();
			void compile(std::function<void(std::string const&)>) const;
			void set_params(param&&);
			static map_method factory(std::string const&, std::optional<docscript::method>);
		private:
			std::unique_ptr<param> params_;
		};

		class vlist;

		class vec_method : public method_base {
			friend struct fmt::formatter<value>;
		public:
			vec_method(std::string const& name, std::optional<docscript::method> optm);
			vec_method(vec_method&& vecm) noexcept;
			~vec_method();
			void compile(std::function<void(std::string const&)>) const;
			/**
			 * append to value list (vector)
			 */
			void append(value&&);

			static vec_method factory(std::string const&, std::optional<docscript::method>);
		private:
			std::unique_ptr<vlist>  vlist_;
		};

	}
}

namespace fmt {


	template <>
	struct formatter<docscript::ast::map_method> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext& ctx) {
			return ctx.begin();
		}

		template <typename FormatContext>
		auto format(const docscript::ast::map_method& m, FormatContext& ctx) {
			return format_to(ctx.out(), "map_method[{}]", m.name_);
		}
	};
}  // namespace fmt

#endif