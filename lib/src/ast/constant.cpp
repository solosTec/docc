#include <ast/constant.h>

#include <cyng/parse/timestamp.h>
#include <cyng/parse/color.h>
#include <cyng/io/ostream.h>

#include  <sstream>

#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>

namespace docscript {
	namespace ast {
		//
		//	----------------------------------------------------------*
		//	-- constant
		//	----------------------------------------------------------*
		//
		constant constant::factory(symbol const& sym) {

			switch (sym.type_) {
			case symbol_type::TST:
				//	parse string with timestamp info
				//	format: "YYYY-MM-DD[THH:MM:SS[Z|[+|-]hh::mm]]
				return constant{ sym.value_, cyng::to_timestamp(sym.value_) };
			case symbol_type::BOL:
				return constant{ sym.value_, boost::algorithm::equals(sym.value_, "true")};
			case symbol_type::NUM:
				try {
					return constant{ sym.value_, static_cast<std::uint64_t>(std::stoull(sym.value_, nullptr, 10)) };
				}
				catch (...) {
					return constant{ sym.value_, static_cast<std::uint64_t>(0) };
				}
			case symbol_type::FLT:
			case symbol_type::EXP:
				try {
					return constant{ sym.value_, std::stod(sym.value_, nullptr) };
				}
				catch (...) {
					return constant{ sym.value_, static_cast<double>(0.0) };
				}
			case symbol_type::COL:
				//	color
				return constant{ sym.value_, cyng::to_color<std::uint8_t>(sym.value_) };
			default:
				break;
			}
			return constant{ sym.value_, sym.value_ };
		}

		void constant::compile(std::function<void(std::string const&)> emit) const {
			//std::cout << "constant::compile()" << std::endl;
			std::stringstream ss;
			ss << *this;

			emit("push ");
			emit(ss.str());
			emit("\n");
		}

		template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		// explicit deduction guide (not needed as of C++20)
		template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

		std::ostream& operator<<(std::ostream& os, constant const& c) {
			std::visit(overloaded{
			[&](double arg) { os << std::fixed << arg; },
			[&](std::uint64_t arg) { os << std::dec << arg; },
			[&](std::string const& arg) { os << std::quoted(arg); },
			[&](std::chrono::system_clock::time_point const& arg) { 
				const std::time_t t_c = std::chrono::system_clock::to_time_t(arg);
				os << std::put_time(std::localtime(&t_c), "%FT%T\n");
				},
			[&](bool arg) { os << (arg ? "true" : "false"); },
			[&](cyng::color_8 const& arg) { os << arg; },

				}, c.node_);
			return os;
		}

	}
}
