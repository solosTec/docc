/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef DOCSCRIPT_SANITIZER_H
#define DOCSCRIPT_SANITIZER_H

#include <docscript/token.h>
#include <cyng/log/severity.h>

#include <boost/regex/pending/unicode_iterator.hpp>

namespace docscript
{
	class sanitizer
	{
	public:
		sanitizer(emit_token_f, std::function<void(cyng::logging::severity, std::string)>);


		/**
		 * Read the specified range and emit tokens.
		 * Comments are removed from data stream.
		 */
		void read(boost::u8_to_u32_iterator<std::string::const_iterator> first, boost::u8_to_u32_iterator<std::string::const_iterator> last);

		/**
		 * After reading the input file, the last pending
		 * character in the input buffer have to be emitted.
		 */
		void flush(bool eof);

	private:
		void next(std::uint32_t);
		void emit(std::uint32_t) const;
		void emit(std::uint32_t, std::size_t) const;

	private:
		/**
		 * callback for complete tokens
		 */
		emit_token_f	emit_;
		std::function<void(cyng::logging::severity, std::string)>	err_;

		std::uint32_t	last_char_;	//!<	previous character
		std::size_t		counter_;	//!<	counter of successive equal characters
	};
}

#endif	
