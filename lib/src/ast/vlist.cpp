#include <ast/vlist.h>

#include <boost/assert.hpp>

namespace docscript {
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- vlist
		//	----------------------------------------------------------*
		//
		vlist::vlist(value&& v) noexcept
			: value_(std::move(v))
			, next_(nullptr)
		{}
		vlist::vlist(vlist&& vl) noexcept
			: value_(std::move(vl.value_))
			, next_(std::move(vl.next_))
		{}

		vlist::vlist() noexcept
			: value_()
			, next_(nullptr)
		{}

		vlist::~vlist() = default;
		void vlist::compile() {
			std::cout << "vlist::compile()" << std::endl;
		}

		void vlist::append(value && v) {
			if (next_) {
				next_->append(std::move(v));
			}
			else {
				next_ = std::make_unique<vlist>(std::move(v));
			}
		}

		vlist vlist::factory(symbol const& sym) {
			return value::factory(sym);
		}


	}
}
