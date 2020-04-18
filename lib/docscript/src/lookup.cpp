/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/lookup.h>
#include <boost/algorithm/string.hpp>

namespace docscript
{
	namespace lookup
	{
		/**
		 * @return true if function is standalone (not part of a paragraph)
		 */
		bool is_standalone(std::string name)
		{
			if (boost::algorithm::equals(name, "i"))	return false;
			if (boost::algorithm::equals(name, "b"))	return false;
			if (boost::algorithm::equals(name, "tt"))	return false;
			if (boost::algorithm::equals(name, "bold"))	return false;
			if (boost::algorithm::equals(name, "color"))	return false;
			if (boost::algorithm::equals(name, "sub"))	return false;
			if (boost::algorithm::equals(name, "sup"))	return false;
			if (boost::algorithm::equals(name, "link"))	return false;
			if (boost::algorithm::equals(name, "now"))	return false;
			//if (boost::algorithm::equals(name, "set"))	return false;
			if (boost::algorithm::equals(name, "get"))	return false;
			if (boost::algorithm::equals(name, "footnote"))	return false;
			if (boost::algorithm::equals(name, "symbol"))	return false;
			if (boost::algorithm::equals(name, "currency"))	return false;
			if (boost::algorithm::equals(name, "tag"))	return false;
			return true;
		}

		/**
		 * @return number of return values
		 */
		std::size_t rcount(std::string name)
		{
			if (boost::algorithm::equals(name, "meta"))	return 0u;
			if (boost::algorithm::equals(name, "set"))	return 0u;
			return 1u;
		}
	}
}


