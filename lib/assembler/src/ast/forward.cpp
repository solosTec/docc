#include <ast/forward.h>
#include <context.h>

#include <cyng/obj/factory.hpp>

#include <boost/uuid/uuid_io.hpp>

namespace docasm {
	namespace ast {


		//
		//	----------------------------------------------------------*
		//	-- forward
		//	----------------------------------------------------------*
		//
		forward forward::factory() {
			return forward{ boost::uuids::uuid()};
		}

		std::ostream& operator<<(std::ostream& os, forward const& c) {
			os << "forward " << c.tag_;
			return os;
		}

		forward forward::finish(boost::uuids::uuid tag) {
			return forward{ tag };
		}

		std::size_t forward::size() const {
			return 2;
		}
		void forward::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(cyng::op::FORWARD));
			ctx.emit(cyng::make_object(tag_));
		}

	}
}
