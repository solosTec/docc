#include <ast/value.h>
#include <ast/constant.h>
#include <ast/method.h>

#include  <sstream>

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
					[&](map_method const& arg) { os << "map_method" << ' '; },
					[&](vec_method const& arg) { os << "vec_method" << ' '; }
					}, vn.value_);
				return os;
			}

			void compile(std::function<void(std::string const&)> emit) const {
				std::visit([&](auto const& arg) {
					arg.compile([&](std::string const& s) {
						emit(s);
						});
					}, value_);
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

		void value::compile(std::function<void(std::string const&)> emit) const {
			//std::cout << "value::compile()" << std::endl;
			if (node_) {
				node_->compile(emit);
			}
			else {
				emit("PUSH ");
				emit("nil");
				emit("\n");
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
