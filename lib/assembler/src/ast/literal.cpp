#include <ast/literal.h>
#include <context.h>

#include <cyng/obj/factory.hpp>

namespace docasm {
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- literal
		//	----------------------------------------------------------*
		//
		literal literal::factory(std::string const& name) {
			return literal{ name };
		}

		std::ostream& operator<<(std::ostream& os, literal const& c) {
			os << c.value_;
			return os;
		}

		std::size_t literal::size() const {
			return 1;
		}
		void literal::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(value_));
		}
	}
}
