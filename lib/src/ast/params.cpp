#include <ast/params.h>
#include <symbol.h>

#include <iomanip>

#include <boost/assert.hpp>

namespace docscript {
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- param
		//	----------------------------------------------------------*
		//
		param::param(std::string const& key, value&& v)
			: key_(key)
			, value_(std::move(v))
			, next_(nullptr)
		{}

		param::param(std::string const& key)
			: key_(key)
			, value_()
			, next_(nullptr)
		{}

		param::param(param&& p) noexcept
			: key_(p.key_)
			, value_(std::move(p.value_))
			, next_(std::move(p.next_))
		{}

		param::~param() = default;

		void param::compile(std::function<void(std::string const&)> emit) const {
			//std::cout << "param::compile()" << std::endl;
			BOOST_ASSERT_MSG(is_complete(), "param is incomplete");

			emit("PUSH ");
			emit(key_);
			emit("\n");

			value_.compile(emit);

			emit("PARAM");
			emit("\n");

			if (next_) {
				next_->compile(emit);
			}

		}

		bool param::is_complete() const {
			return !value_.empty();
		}

		param param::finish(value && v) {
			BOOST_ASSERT_MSG(!is_complete(), "already complete");
			return { key_, std::move(v) };
		}

		void param::append(param && p) {
			if (next_) {
				return next_->append(std::move(p));
			}
			else {
				next_ = std::make_unique<param>(std::move(p));
			}
		}

		param param::factory(symbol const& sym) {
			return { sym.value_ };
		}

		std::ostream& operator<<(std::ostream & os, param const& p) {
			os << "param[" << p.key_ << ":" << p.value_;
			if (p.next_) {
				os << "->" << *(p.next_);
			}
			os << "]";
			return os;
		}
	}
}
