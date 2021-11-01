#include <ast/operation.h>
#include <context.h>

#include <cyng/io/ostream.h>
//#include <cyng/parse/timestamp.h>
#include <cyng/obj/factory.hpp>

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
