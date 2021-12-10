
#include <docc/reader.h>
#include <docc/utils.h>

//#include <cyng/io/parser/utf-8.hpp>
#include <cyng/io/parser/stream.hpp>
#include <cyng/task/registry.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/algorithm/string.hpp>

namespace docscript {

	reader::reader(	context& ctx)
		: ctx_(ctx)
		, prev_('\n')
		, tokenizer_([this](symbol&& sym) {
			this->next_symbol(std::move(sym));
		})
		, sanitizer_([this](token&& tok) {
			//std::cout << tok << std::endl;
			//
			//  repeat if requested
			//
			while (!tokenizer_.put(tok, prev_))
				;
			new (&prev_) token(tok);
		})
	{}

	void reader::read(std::filesystem::path p) {

		if (ctx_.get_verbosity(1)) {
			fmt::print(
				stdout,
				fg(fmt::color::green_yellow) | fmt::emphasis::bold,
				"***info : read [{}]\n", p.filename().string());
		}

		//
		//  start new file
		//
		if (ctx_.push(p))
		{

			auto [a, b] = ctx_.get_stream_range();
			BOOST_ASSERT(a != b);

			auto [pos, end] = cyng::utf8::get_utf8_range(a, b);

			std::size_t line{ 1 };
			for (; pos != end; ++pos)
			{
				if (sanitizer_.put(*pos)) {
					ctx_.nl(++line);
				}
			}

			ctx_.pop(sanitizer_);

		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : input file [{}] not found\n", p.string());

		}
	}

	void reader::next_symbol(symbol&& sym) {
		if (sym == symbol_type::INC) {

			auto const r = ctx_.lookup(sym.value_, "docscript");
			if (r.second) {

				if (ctx_.get_verbosity(2)) {
					fmt::print(
						stdout,
						fg(fmt::color::green_yellow) | fmt::emphasis::bold,
						"***info : include [{}]\n", r.first.filename().string());
				}

				//
				//  recursive call
				//
				read(r.first);
			}
			else {
				fmt::print(
					stdout,
					fg(fmt::color::crimson) | fmt::emphasis::bold,
					"***info : include file [{}] not found\n", r.first.string());

			}

		}
		else {
			ctx_.put(sym);
		}
	}
}
