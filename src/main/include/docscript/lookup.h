/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_LOOKUP_H
#define DOCSCRIPT_LOOKUP_H

#include <string>
#include <cstdio>

namespace docscript
{
	/**
	 * Provide basic infos about functions
	 */
	namespace lookup
	{

		/**
		 * @return true if function is standalone (not part of a paragraph)
		 */
		bool is_standalone(std::string);

		/**
		 * @return number of return values
		 */
		std::size_t rcount(std::string);
	}
}

#endif
