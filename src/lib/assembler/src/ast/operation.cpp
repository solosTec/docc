#include <asm/ast/operation.h>
#include <asm/context.h>

#include <cyng/io/ostream.h>
#include <cyng/obj/factory.hpp>


namespace docasm {
	namespace ast {


		//
		//	----------------------------------------------------------*
		//	-- operation
		//	----------------------------------------------------------*
		//
		operation operation::factory(cyng::op code) {
			return operation{ code };
		}

		std::ostream& operator<<(std::ostream& os, operation const& c) {
			os << c.code_;
			return os;
		}

		std::size_t operation::size() const {
			return 1;
		}
		void operation::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(code_));
		}

	}
}
