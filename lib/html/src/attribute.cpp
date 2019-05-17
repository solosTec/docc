/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <html/attribute.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace html
{
	std::string attr::to_str() const
	{
		if (value_f_)
		{
			std::string value = value_f_();
			if (!is_property(tag_)) {

				std::string res = tag_.get() + "=\"";
				for (std::string::size_type i = 0; i < value.length(); ++i)
				{
					if ('\"' == value.at(i) && (i == 0 || value.at(i - 1) != '\\'))
					{
						res.push_back('\\');
						res.push_back('\"');
					}
					else
					{
						res.push_back(value.at(i));
					}
				}

				res += "\"";
				return res;
			}
		}
		return tag_.get();
	}

	bool is_property(std::string const& name)
	{
		return boost::algorithm::equals(name, "open")
			|| boost::algorithm::equals(name, "closed")
			|| boost::algorithm::equals(name, "disabled")
			|| boost::algorithm::equals(name, "hidden")
			|| boost::algorithm::equals(name, "autofocus")
				;
	}


}
