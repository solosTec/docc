#include <docc/method.h>

namespace docscript {

	method::method(std::string name, parameter_type pt, bool inl, std::initializer_list<std::string> params)
		: name_(name)
		, pt_(pt)
		, inline_(inl)
		, params_(params.begin(), params.end())
	{}

	method::~method()
	{}

	std::string const& method::get_name() const {
		return name_;
	}

	bool method::is_inline() const {
		return inline_;
	}

	method::param_names_t const& method::get_param_names() const {
		return params_;
		//return { params_.cbegin(), params_.cend() };
	}

	method make_placeholder_method(std::string const& name) {
		return { name, parameter_type::VECTOR, true, {} };
	}

}

namespace std {

	bool less<docscript::method>::operator()(docscript::method const& mlh, docscript::method const& mrh) const {
		return mlh.get_name().compare(mrh.get_name()) < 0;
	}
	
}