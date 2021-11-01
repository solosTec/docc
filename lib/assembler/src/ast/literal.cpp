#include <ast/literal.h>
#include <context.h>

//#include <cyng/io/ostream.h>
//#include <cyng/parse/timestamp.h>
#include <cyng/obj/factory.hpp>
//
//#include <fmt/core.h>
//#include <fmt/color.h>
//
//#include <utility>
//
//#include <boost/assert.hpp>
//#include <boost/algorithm/string.hpp>
//#include <boost/uuid/string_generator.hpp>
//#include <boost/uuid/uuid_io.hpp>

namespace docscript {
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
