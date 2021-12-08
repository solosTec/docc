#include <docc/ast/params.h>
#include <docc/symbol.h>
#include <docc/context.h>

#include <iomanip>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>

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

		std::size_t param::compile(std::function<void(std::string const&)> emit, std::size_t depth, std::size_t index) const {
			//std::cout << "param::compile()" << std::endl;
			BOOST_ASSERT_MSG(is_complete(), "param is incomplete");

			value_.compile(emit, depth, index);

			emit("push ");
			emit(key_);
			emit("\n");

			emit("make_param\n");

			return (next_) 
				? next_->compile(emit, depth, index + 1) + 1u
				: 1u;
		}
		void param::transform(context const& ctx) {
			auto const r = value_.is_vec_method();
			if (r.second && boost::algorithm::equals(r.first, "quote")) {
				//std::cout << "SUBSTITUTE " << key_ << ": " << r.first << std::endl;
				auto optm = ctx.lookup_method("range");
				if (optm) {
					value_.rename(*optm);
				}
			}
			if (next_) {
				next_->transform(ctx);
			}
		}

		param::param_names_t param::get_param_names() const {
			param_names_t names;
			names.insert(key_);
			if (next_) {
				auto const n = next_->get_param_names();
				names.insert(n.begin(), n.end());
			}
			return names;
		}

		//void param::verify(context& ctx, std::string const& fn) {
		//	//
		//	//	ToDo: What if the source is produced by a method?
		//	//
		//	if (boost::algorithm::equals(fn, "figure") && boost::algorithm::equals(key_, "source")) {
		//		//
		//		//	resolve path
		//		//
		//		auto const r = value_.resolve_path(ctx, "png");
		//		if (!r.second) {
		//			//
		//			//	emit error
		//			//
		//			fmt::print(
		//				stdout,
		//				fg(fmt::color::crimson) | fmt::emphasis::bold,
		//				"{}: error: cannot resolve path [{}]\n", ctx.get_position(), r.first.string());

		//		}
		//		else {
		//			//
		//			//	substitute value
		//			//
		//			value_.swap(value::factory(constant::factory(r.first.string())));
		//		}
		//	}
		//	if (next_) {
		//		next_->verify(ctx, fn);
		//	}

		//	//
		//	//	check parameters
		//	// ToDo: This check is executed on every level!
		//	//
		//	//auto names = get_param_names();
		//	//if (names.find("scale") == names.end()) {
		//	//	append(param("scale", value::factory(make_symbol(symbol_type::FLT, "1.0"))));
		//	//}
		//}

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

		std::size_t param::size() const {
			std::size_t n{ 0 };
			auto p = next_.get();
			while (p != nullptr) {
				++n;
				p = (p->next_)
					? p->next_.get()
					: nullptr
					;
			}
			return n;
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