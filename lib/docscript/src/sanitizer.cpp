/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include <docscript/sanitizer.h>

namespace docscript
{
	sanitizer::sanitizer(emit_token_f f, std::function<void(cyng::logging::severity, std::string)> err)
		: emit_(f)
		, err_(err)
		, last_char_('\n')
		, counter_(0)
	{}

	void sanitizer::read(boost::u8_to_u32_iterator<std::string::const_iterator> first, boost::u8_to_u32_iterator<std::string::const_iterator> last)
	{
		//
		//	walk over range
		//
		while (first != last)
		{
			//
			// skip windows artifacts in linux
			//
			if ('\r' == *first) {
				++first;
			}
			else {

				//			std::cout << "--- " << char(*first) << '[' << *first << ']' << " --- " << char(last_char_) << std::endl;
				if (*first != last_char_)
				{
					//
					//	process n * characters
					//
					next(last_char_);

					last_char_ = *first++;
					counter_ = 1;
				}
				else
				{
					++counter_;
					last_char_ = *first++;
				}
			}
		}
	}

	void sanitizer::flush(bool eof)
	{
		next(last_char_);
		if (eof)	emit_(make_eof());
	}

	void sanitizer::emit(std::uint32_t c) const
	{
		emit_(make_token(c, counter_));
	}

	void sanitizer::emit(std::uint32_t c, std::size_t count) const
	{
		emit_(make_token(c, count));
	}

	void sanitizer::next(std::uint32_t c)
	{
		emit_(make_token(c, counter_));
	}

}
