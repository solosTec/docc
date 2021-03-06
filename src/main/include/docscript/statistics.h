/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_STATISTICS_H
#define DOCSCRIPT_STATISTICS_H

#include <cstdint>
#include <map>

namespace docscript
{
	//
	//	Container to hold statistical data 
	//
	using frequency_t = std::map<std::uint32_t, std::size_t>;

	/**
	 * @returns the total size of symbols
	 */
	std::size_t calculate_size(frequency_t const& stat);


	/**
	 * @returns Shannon entropy 
	 */
	double calculate_entropy(frequency_t const& stat);

}

#endif	




