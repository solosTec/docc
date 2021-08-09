/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_PARSER_H
#define DOCC_SCRIPT_PARSER_H

#include <symbol.h>

#include <cstdint>
//#include <functional>

namespace docscript {

	class context;
	class parser
	{

	public:
		parser(context const&);
		void put(symbol const& sym);

	private:
		context const& ctx_;
	};
}
#endif //   DOCC_SCRIPT_PARSER_H