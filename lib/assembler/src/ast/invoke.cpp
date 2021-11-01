#include <ast/invoke.h>
#include <context.h>

#include <cyng/obj/factory.hpp>

namespace docscript {
	namespace ast {

		template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		// explicit deduction guide (not needed as of C++20)
		template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;


		//
		//	----------------------------------------------------------*
		//	-- invoke
		//	----------------------------------------------------------*
		//
		invoke invoke::factory() {
			return invoke{ std::string() };
		}

		std::ostream& operator<<(std::ostream& os, invoke const& c) {
			os << "invoke " << c.name_;
			return os;
		}

		invoke invoke::finish(std::string const& name) {
			return { name };
		}

		std::size_t invoke::size() const {
			return 2;
		}
		void invoke::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(name_));
			ctx.emit(cyng::make_object(cyng::op::INVOKE));
		}

		//
		//	----------------------------------------------------------*
		//	-- invoke_r
		//	----------------------------------------------------------*
		//
		invoke_r invoke_r::factory() {
			return invoke_r{ std::string() };
		}

		std::ostream& operator<<(std::ostream& os, invoke_r const& c) {
			os << "invoke_r " << c.name_;
			return os;
		}

		invoke_r invoke_r::finish(std::string const& name) {
			return { name };
		}

		std::size_t invoke_r::size() const {
			return 3;
		}
		void invoke_r::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(name_));
			ctx.emit(cyng::make_object(cyng::op::RESOLVE));
			ctx.emit(cyng::make_object(cyng::op::INVOKE_R));
		}


	}
}
