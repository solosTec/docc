#include <ast/push.h>
#include <context.h>

//#include <cyng/io/ostream.h>
//#include <cyng/parse/timestamp.h>
//#include <cyng/obj/factory.hpp>
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
		//	-- push
		//	----------------------------------------------------------*
		//
		push push::factory() {
			return push{ std::monostate()};
		}

		std::ostream& operator<<(std::ostream& os, push const& c) {
			os << "push " << c.val_;
			return os;
		}

		push push::finish(value&& val) {
			return { val };
		}

		std::size_t push::size() const {
			return 2;
		}
		void push::generate(context& ctx, label_list_t const& ll) const {
			//ctx.emit(cyng::make_object(cyng::op::PUSH));	//	push is implicit
			val_.generate(ctx, ll);
		}

	}
}
