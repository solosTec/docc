#include <ast/label.h>

#include <boost/assert.hpp>

namespace docasm {
	namespace ast {

		//
		//	----------------------------------------------------------*
		//	-- label
		//	----------------------------------------------------------*
		//
		label label::factory(std::string const& name) {
			return label{ name };
		}

		std::ostream& operator<<(std::ostream& os, label const& c) {
			os << c.name_ << ':';
			return os;
		}

		std::size_t label::size() const {
			return 0;
		}

		void label::generate(context&, label_list_t const&) const {

		}


	}
}
