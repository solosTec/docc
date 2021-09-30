#include <ast/params.h>
#include <symbol.h>

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

		void param::compile() {
			std::cout << "param::compile()" << std::endl;
		}

		param param::finish(value && v) {
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
			os << "param[" << p.key_ << ": " << p.value_;
			if (p.next_) {
				os << ", next " << *(p.next_);
			}
			os << "]";
			return os;
		}
	}
}
