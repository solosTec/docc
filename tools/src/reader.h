/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCC_READER_H
#define DOCC_READER_H

#include <docscript/include.h>
#include <cstdint>
#include <functional>
#include <cyng/compatibility/file_system.hpp>

namespace docscript
{
	class driver;
	class reader 
	{
		using include_f = std::function<int(incl_t inp, std::size_t)>;

	public:
		reader(driver&, cyng::filesystem::path const&, std::size_t start, std::size_t count);
		bool run(std::size_t depth);

	private:
		incl_t parse_include(std::string const& line);
		void tokenize(std::string const& );

	private:
		driver& driver_;
		cyng::filesystem::path const source_;
		std::size_t const start_;
		std::size_t const count_;
		std::size_t	curr_line_;
	};
}

#endif
