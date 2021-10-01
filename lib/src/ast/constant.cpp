#include <ast/constant.h>

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
				//	ToDo: parse string with timestamp info
				//	format: YYYY-MM-DD[THH:MM:SS.zzZ]
				return constant{ sym.value_, std::chrono::system_clock::now() };
			case symbol_type::BOL:
				return constant{ sym.value_, boost::algorithm::equals(sym.value_, "true")};
			case symbol_type::NUM:
				return constant{ sym.value_, std::stod(sym.value_, nullptr) };
			default:
				break;
			}
			return constant{ sym.value_, sym.value_ };
		}

		void constant::compile(std::function<void(std::string const&)> emit) const {
			//std::cout << "constant::compile()" << std::endl;
			std::stringstream ss;
			ss << *this;

			emit("PUSH ");
			//emit(this->value_);
			emit(ss.str());
			emit("\n");
		}

		template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		// explicit deduction guide (not needed as of C++20)
		template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;

		std::ostream& operator<<(std::ostream& os, constant const& c) {
			std::visit(overloaded{
			[&](double arg) { os << std::fixed << arg; },
			[&](std::string const& arg) { os << std::quoted(arg); },
			[&](std::chrono::system_clock::time_point const& arg) { os << "tp"; },
			[&](bool arg) { os << (arg ? "true" : "false"); }
				}, c.node_);
			return os;
		}

	}
}
