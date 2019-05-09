/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_PRE_COMPILER_H
#define DOCSCRIPT_PRE_COMPILER_H

#include <docscript/include.h>

#include <boost/filesystem.hpp>

#include <string>
#include <tuple>

namespace docscript
{
	class pre_compiler
	{
	public:
		pre_compiler(std::string inp);

		incl_t parse_include() const;

	private:
		std::string const input_;
	};
}

#endif	




