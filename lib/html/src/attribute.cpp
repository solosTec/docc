/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <html/attribute.hpp>

namespace html
{
	std::string attr::to_str() const
	{
		if (value_f_)
		{
			std::string value = value_f_();
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
			return std::move(res);
		}
		return tag_.get();
	}


}
