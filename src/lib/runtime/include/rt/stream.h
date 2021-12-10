/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_RUNTIME_FILE_H
#define DOCC_RUNTIME_FILE_H

#include <cyng/obj/intrinsics/buffer.h>
#include <cyng/io/parser/stream.hpp>

#include <string>
#include <cstdio>
#include <fstream>
#include <utility>
#include <filesystem>
#include <iterator>

namespace docruntime
{
	/**
	 * It's a good idea to set the stream mode to binary.
	 */
	std::pair<cyng::buffer_t, std::streamsize> prepare_buffer(std::istream&);
	std::pair<cyng::buffer_t, std::streamsize> stream_to_buffer(std::istream&);

	/**
	 * @return extension without the '.'
	 */
	std::string get_extension(std::filesystem::path const& p);

}

#endif
