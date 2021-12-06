#include <docc/ast/value.h>
#include <docc/ast/constant.h>
#include <docc/ast/method.h>
#include <docc/context.h>

#include <sstream>

#include <boost/assert.hpp>

namespace docscript {
	namespace ast {

		template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		// explicit deduction guide (not needed as of C++20)
		template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

		//
		//	----------------------------------------------------------*
		//	-- value
		//	----------------------------------------------------------*
		//
		struct value::value_node {

			//	all possible declarations
			using value_t = std::variant<constant, map_method, vec_method>;
			value_t value_;

			/**
			 * move constructor
			 */
			value_node(value_t&& val)
				: value_(std::move(val))
			{}
			value_node(constant c)
				: value_{ c }
			{}
			value_node(map_method&& mapm)
				: value_{ std::move(mapm) }
			{}
			value_node(vec_method&& vecm)
				: value_{ std::move(vecm) }
			{}

			friend std::ostream& operator<<(std::ostream& os, value_node const& vn) {
				std::visit(overloaded{
					[&](constant const& arg) { os << arg << ' '; },
					[&](map_method const& arg) { os << arg.get_name() << ' '; },
					[&](vec_method const& arg) { os << arg.get_name() << ' '; }
					}, vn.value_);
				return os;
			}

			void compile(std::function<void(std::string const&)> emit, std::size_t depth, std::size_t index) const {
				std::visit([&](auto const& arg) {
					arg.compile([&](std::string const& s) {
						emit(s);
						}, depth, index);
					}, value_);
			}

			void transform(context const& ctx) {
				std::visit([&](auto& arg) {
					arg.transform(ctx);
				}, value_);
			}

			void rename(docscript::method m) {
				std::visit(overloaded{
					[&](constant& arg) { ; },
					[&](map_method& arg) { arg.rename(m); },
					[&](vec_method& arg) { arg.rename(m); }
				}, value_);
			}

			constexpr bool is_constant_txt() const noexcept {
				return (value_.index() == 0) && (std::get<0>(value_).node_.index() == 0);
			}

			constexpr bool is_map_method() const noexcept {
				return value_.index() == 1;
			}

			constexpr bool is_vec_method() const noexcept {
				return value_.index() == 2;
			}

			std::string to_str() const {
				std::stringstream ss;
				ss << *this;	//	assembler output
				return ss.str();
			}

			std::pair<std::filesystem::path, bool>  resolve_path(context& ctx, std::string ext) {
				auto s = to_str();
				//	unquote
				s.erase(remove(s.begin(), s.end(), '"'), s.end());
				return ctx.lookup(s, ext);
			}

		};

		value::value() noexcept
			: node_(nullptr)
		{}

		value::value(value&& v) noexcept
			: node_(std::move(v.node_))
		{}

		value::~value() = default;

		value::value(value_node * p)
			: node_(p)
		{}

		bool value::empty() const {
			return !node_;
		}

		std::pair<std::string, bool> value::is_constant_txt() const {
			return (!empty() && node_->is_constant_txt())
				? std::make_pair(std::get<0>(node_->value_).value_, true)
				: std::make_pair("", false)
				;
		}

		std::pair<std::string, bool> value::is_vec_method() const {	
			return (!empty() && node_->is_vec_method())
				? std::make_pair(std::get<2>(node_->value_).get_name(), true)
				: std::make_pair("", false)
				;
		}

		void value::rename(docscript::method m) {
			if (!empty()) {
				node_->rename(m);
			}
		}

		std::pair<std::filesystem::path, bool>  value::resolve_path(context& ctx, std::string ext) {
			if (!empty()) {
				return node_->resolve_path(ctx, ext);
			}
			return { "", false};
		}


		void value::merge(value&& v) {
			auto txt = std::get<0>(std::get<0>(node_->value_).node_).append(std::get<0>(std::get<0>(v.node_->value_).node_));
			//auto val = factory(constant::factory(symbol(symbol_type::TXT, std::move(txt))));
			//node_.swap(val.node_);
			swap(factory(constant::factory(symbol(symbol_type::TXT, std::move(txt)))));
		}

		void value::swap(value&& v) {
			node_.swap(v.node_);
		}

		std::size_t value::index() const {
			return (node_)
				? node_->value_.index()
				: std::variant_npos
				;
		}

		value value::factory(symbol const& sym) {
			return { new value_node(constant::factory(sym)) };
		}

		value value::factory(map_method && m) {
			return { new value_node(std::move(m)) };
		}

		value value::factory(vec_method && m) {
			return { new value_node(std::move(m)) };
		}

		value value::factory(constant&& c) {
			return { new value_node(std::move(c)) };
		}

		void value::compile(std::function<void(std::string const&)> emit, std::size_t depth, std::size_t index) const {
			//std::cout << "value::compile()" << std::endl;
			if (node_) {
				node_->compile(emit, depth, index + 1u);
			}
			else {
				emit("push ");
				emit("nil");
				emit("\n");
			}
		}
		void value::transform(context const& ctx) {
			if (node_) {
				node_->transform(ctx);
			}
		}

		std::ostream& operator<<(std::ostream & os, value const& v) {
			//os << "value[" << "]";
			if (v.node_) {
				os << *(v.node_);
			}
			else {
				os << "empty";
			}
			return os;
		}
	}
}
