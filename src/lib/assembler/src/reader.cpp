
#include <asm/reader.h>

#include <cyng/io/parser/stream.hpp>

#include <iostream>
#include <fstream>

#include <fmt/core.h>
#include <fmt/color.h>

namespace docasm {

	reader::reader(std::filesystem::path out
		, std::vector<std::filesystem::path> const& inc
		, int verbose)
	: ctx_(verify_extension(out, "cyng"), inc, verbose)
		, prev_('\n')
		, tokenizer_([this](symbol&& sym) {
			this->next_symbol(std::move(sym));
			})
		, sanitizer_([this](token&& tok) {
				//std::cout << tok << std::endl;
				//
				//  repeat if requested
				//
				while (!tokenizer_.put(tok, prev_)) {
					;
				}
				new (&prev_) token(tok);
			})
	{
	}


	void reader::read(std::filesystem::path p) {

		auto const r = ctx_.lookup(p);
		if (r.second && ctx_.push(r.first)) {

			fmt::print(
				stdout,
				fg(fmt::color::green_yellow) | fmt::emphasis::bold,
				"***info : read [{}]\n", r.first.string());

			//
			//  start new file
			//

			std::istream_iterator<char> a = ctx_.get_stream_iterator();
			std::istream_iterator<char> b;
			BOOST_ASSERT(a != b);

			auto [pos, end] = cyng::utf8::get_utf8_range(a, b);

			std::size_t line{ 1 };
			for (; pos != end; ++pos)
			{
				if (sanitizer_.put(*pos)) {
					ctx_.nl(++line);
				}
			}
			
			//
			//	file complete
			//
			ctx_.pop(sanitizer_);
		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : input file [{}] not found\n", p.string());

		}
	}

	std::filesystem::path const& reader::get_output_path() const {
		return ctx_.get_output_path();
	}

	void reader::next_symbol(symbol&& sym) {
		//std::cout << sym << std::endl;
		ctx_.put(sym);
	}
}
