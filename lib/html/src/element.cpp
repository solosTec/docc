/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <html/element.hpp>

namespace html
{
	base::base(std::string const& tag)
	: tag_(tag)
	{}

	base::operator std::string() const
	{
		return to_str();
	}

	std::string	base::operator()() const
	{
		return to_str();
	}

}
