/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef HTML_ATTRIBUTE_H
#define HTML_ATTRIBUTE_H

#include <html/element.hpp>
#include <functional>
#include <type_traits>

namespace html
{
	class attr : public base
	{
	public:
		explicit attr(std::string const &name) 
			: base(name)
			, value_f_()
		{}

		explicit attr(std::string const &name, std::string &&value)
			: base(name)
			, value_f_([value]() { return value; })
		{}

		/**
		 * be carefull with ownership of value
		 */
		explicit attr(std::string const &name, std::string const& value)
			: base(name)
			, value_f_([&value]() { return value; })
		{}

		//attr(attr&& other)
		//	: base("")
		//	, value_f_()
		//{}


		/**
		 * Using SFINAE to handle non-string types by std::to_string()
		 */
		template<typename T, class = decltype(std::to_string(std::declval<T>()))>
		explicit attr(std::string const &name, T &&value) 
			: base(name)
			, value_f_([value]() {return std::to_string(value); })
		{}

		/**
		 * Using SFINAE to handle functions with return type std::string 
		 */
		template<class F, class = std::enable_if<is_string<typename F::result_type>::value>>
		explicit attr(std::string const &name, F f)
			: base(name)
			, value_f_(f)
		{}

		

		virtual std::string to_str() const override;

	protected:
		std::function<std::string()> value_f_;
	};
}

#endif
