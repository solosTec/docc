#include <ast/value.h>
#include <context.h>

#include <cyng/io/ostream.h>
#include <cyng/parse/timestamp.h>
#include <cyng/obj/factory.hpp>
//
//#include <fmt/core.h>
//#include <fmt/color.h>
//
//#include <utility>
//
#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>
//#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

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
		value value::factory(symbol const& sym) {
			switch (sym.type_) {
			case symbol_type::TST:
				return { cyng::to_timestamp(sym.value_) };
			case symbol_type::COL:
				return { cyng::color_8() };
			case symbol_type::BOL:
				return { boost::algorithm::equals(sym.value_, "true") };
			case symbol_type::NUM:
				try {
					return { static_cast<std::uint64_t>(std::stoul(sym.value_, nullptr, 10)) };
				}
				catch (...) {
					//fmt::print(
					//	stdout,
					//	fg(fmt::color::orange) | fmt::emphasis::bold,
					//	"{}: warning: invalid number [{}]\n", ctx_.get_position(), sym.value_);
					return { static_cast<std::uint64_t>(0) };
				}
				break;
			default:
				break;
			}
			return { sym.value_ };
		}

		value value::factory(boost::uuids::uuid oid) {
			return { oid };
		}

		std::ostream& operator<<(std::ostream& os, value const& v) {
			std::visit(overloaded{
				[](auto& arg) {},
				[&](std::string const& val) {
					os << std::quoted(val);
					},
				[&](std::chrono::system_clock::time_point const& val) {
					const std::time_t t_c = std::chrono::system_clock::to_time_t(std::get<2>(v.val_));
					os << std::put_time(std::localtime(&t_c), "%FT%T\n");
					},
				[&](cyng::color_8 const& val) {
					os << val;
					},
				[&](bool val) {
					os << (val ? "true" : "false");
					},
				[&](std::uint64_t val) {
					os << std::dec << val << "u64";
					},
				[&](boost::uuids::uuid val) {
					os << val;
					},
				[&](double val) {
					os << std::fixed << val;
					}
				}, v.val_);

			return os;
		}

		std::size_t value::size() const {
			return 1;
		}
		void value::generate(context& ctx, label_list_t const&) const {
			std::visit([&](auto&& arg) {
				ctx.emit(cyng::make_object(arg));
				}, val_);
		}



	}
}
