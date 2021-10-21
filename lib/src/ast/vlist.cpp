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

		std::size_t vlist::compile(std::function<void(std::string const&)> emit, std::size_t depth, std::size_t index) const {

			std::cout 
				<< "vlist::compile("
				<< depth
				<< ", #"
				<< index
				<< ": "
				<< value_
				<< ")" 
				<< std::endl;

			value_.compile(emit, depth, index);
			return (next_)
				? next_->compile(emit, depth, index + 1u) + 1u
				: 1u
				;
		}

		std::size_t vlist::size() const {
			return (next_)
				? next_->size() + 1u
				: 1u
				;
		}

		std::size_t vlist::append(value && v) {
			if (!next_) {
				next_ = std::make_unique<vlist>(std::move(v));
				return 1;
			}
			return next_->append(std::move(v)) + 1;
		}

		vlist vlist::factory(symbol const& sym) {
			return value::factory(sym);
		}


	}
}
