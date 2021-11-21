/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_RUNTIME_CURRENCY_H
#define DOCC_RUNTIME_CURRENCY_H

#include <string>
#include <cstdio>

namespace docruntime
{
	/**
	 * @param value quantity
	 * @param name currency
	 */
	std::string currency(std::size_t value, std::string const& name);
	std::string currency_html(std::size_t value, std::string const& name);
}

#endif
