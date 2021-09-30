/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_METHOD_H
#define DOCC_SCRIPT_METHOD_H

#include <cstdint>
#include <string>
#include <set>

namespace docscript {

	enum class parameter_type {
		VECTOR,
		MAP,
	};
	enum class return_type {
		ANY,
		STRING,
		BOOL,
		U32,
		I32,
		DOUBLE,
	};

	class method 
	{
	public:
		method(std::string, return_type rt, parameter_type pt, bool inl, std::initializer_list<std::string> = {});
		virtual ~method();

		/**
		 * Functions accepts MAPs or VECTORs as input.
		 * 
		 * @return true if function expects the specified input type
		 */
		constexpr bool is_parameter_type(parameter_type pt) const {
			return pt_ == pt;
		}

		/**
		 * @return the (unique) function name
		 */
		std::string const& get_name() const;

		/**
		 * An inline method creates an inline element. This is the default.
		 * Otherwise the method creates a block element and terminates 
		 * a a possibly open paragraph.
		 */
		bool is_inline() const;

	private:
		std::string const name_;
		return_type const rt_;
		parameter_type const pt_;
		bool const inline_;

		/**
		 * parameter names
		 */
		std::set<std::string> const params_;
	};

	/**
	 * Create a placeholder if method is unknown.
	 */
	method make_placeholder_method(std::string const&);
}

namespace std {
	template<>
	struct less<docscript::method>
	{
		bool operator()(docscript::method const& mlh, docscript::method const& mrh) const;
	};
}

#endif
