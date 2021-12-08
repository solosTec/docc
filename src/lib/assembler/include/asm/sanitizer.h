/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_ASM_SANITIZER_H
#define DOCC_ASM_SANITIZER_H

#include <asm/token.h>

#include <cstdint>
#include <functional>

namespace docasm {

	class sanitizer
	{

	public:
		using callback = std::function<void(token&&)>;

	public:
		sanitizer(callback);
		/**
		 * @brief insert next codepoint
		 *
		 * @param c
		 * @return true if new line (NL)
		 */
		bool put(std::uint32_t c);
		void eof();

	private:
		callback cb_;
	};
}
#endif //   DOCC_SCRIPT_SANITIZER_H