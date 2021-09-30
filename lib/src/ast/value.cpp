#include <ast/value.h>
#include <ast/constant.h>
#include <ast/method.h>

#include <boost/assert.hpp>

namespace docscript {
	namespace ast {
		//
		//	----------------------------------------------------------*
		//	-- value
		//	----------------------------------------------------------*
		//
		struct value::value_node {

			//	ToDo: add vector and cite
			using value_t = std::variant<constant, map_method, vec_method>;
			value_t value_;

			value_node(constant c)
				: value_{ c }
			{}
			value_node(value_t&& val)
				: value_(std::move(val))
			{}
			value_node(map_method&& mapm)
				: value_{ std::move(mapm) }
			{}
			value_node(vec_method&& vecm)
				: value_{ std::move(vecm) }
			{}

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

		value value::clone() const {
			if (node_) {

				switch (node_->value_.index()) {
				case 0:
					return new value_node(std::get<0>(node_->value_));
				default:
					break;
				}
			}
			return {};

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

		void value::compile() {
			std::cout << "value::compile()" << std::endl;
		}

		std::ostream& operator<<(std::ostream & os, value const&) {
			os << "value[" << "]";
			return os;
		}
	}
}
