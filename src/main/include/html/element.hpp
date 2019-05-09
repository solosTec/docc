/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef HTML_ELEMENT_H
#define HTML_ELEMENT_H

#include <cstdint>
#include <string>
#include <type_traits>
#include <boost/flyweight.hpp>

namespace html
{
	/**
	 * detect string types
	 */
	template< typename T >
	struct is_string
		: std::integral_constant<
		bool,
		std::is_same<std::string, typename std::remove_cv<T>::type>::value ||
		std::is_same<char const*, typename std::remove_cv<T>::type>::value
		> 
	{};

	template< std::size_t N >
	struct is_string< const char(&)[N]>
		: std::true_type
	{};

	/**
	 * base class
	 */
	class base
	{
	public:
		base() = delete;

		/**
		 * Create an empty element of the type specified.
		 *
		 * Optionally, you can specify a name as well as content
		 * which will be added according to the type.
		 */
		base(std::string const& tag = "div");

		/**
		 * @return a valid HTML reperesentation of the element
		 */
		virtual std::string to_str() const = 0;

		operator std::string() const;
		std::string	operator()() const;

	protected:

		/**
		 * <tag> name
		 */
		boost::flyweight<std::string> const tag_;

	};

}

#endif
