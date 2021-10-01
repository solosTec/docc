#include <ast/constant.h>

//#include <string>

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

		void constant::compile() {
			std::cout << "constant::compile()" << std::endl;

		}
	}
}
