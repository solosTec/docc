#include <asm/ast/push.h>
#include <asm/context.h>

namespace docasm {
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
			val_.generate(ctx, ll);
		}

	}
}
