/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_RUNTIME_CURRENCY_H
#define DOCC_RUNTIME_CURRENCY_H

//#include <cyng/obj/object.h>
//#include <cyng/obj/intrinsics/container.h>
//
//#include <memory>
#include <string>
#include <cstdio>

//#include <boost/uuid/uuid.hpp>

namespace docruntime
{
	/**
	 * @param value quantity
	 * @param name currency
	 */
	std::string currency(std::size_t value, std::string const& name);
}

#endif
