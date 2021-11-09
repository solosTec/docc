/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_READER_H
#define DOCC_SCRIPT_READER_H

#include <docc/context.h>
#include <docc/sanitizer.h>
#include <docc/tokenizer.h>

namespace docscript {

	class reader {

	public:
		reader(context&);
		void read(std::filesystem::path);

	private:
		void next_symbol(symbol&& sym);
	private:
		context& ctx_;
		token prev_;
		tokenizer tokenizer_;
		sanitizer sanitizer_;
	};
}

#endif