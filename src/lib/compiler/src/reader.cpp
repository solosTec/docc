
#include <docc/reader.h>
#include <docc/utils.h>

#include <cyng/io/parser/utf-8.h>
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
			//ctl_.get_registry().dispatch("ruler", "open", std::filesystem::path(p), channel_.lock()->get_id());


			std::istream_iterator<char> a = ctx_.get_stream_iterator();
			std::istream_iterator<char> b;
			BOOST_ASSERT(a != b);

			//  test BOM
			switch (*a & 0xFF) {
			case 0xEF:
				if (((*++a & 0xFF) == 0xBB) && ((*++a & 0xFF) == 0xBF)) {
					++a;
				}
				else {
					//  error
				}
				break;
			case 0xFE:
				//  error: utf-16 BE
				BOOST_ASSERT_MSG(false, "utf-16 BE");
				break;
			case 0xFF:
				//  error: utf-16 LE
				BOOST_ASSERT_MSG(false, "utf-16 LE");
				break;
			default:
				break;
			}

			cyng::utf8::u8_to_u32_iterator pos(a);
			cyng::utf8::u8_to_u32_iterator end(b);

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

			auto const r = ctx_.lookup(sym.value_);
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
