#include <ast/constant.h>
#include <boost/assert.hpp>

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
