#include <asm/context.h>
#include <asm/sanitizer.h>
#include <asm/symbol.h>

#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>

#include <iostream>
#include <string>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>

namespace docasm {

	context::context(std::filesystem::path out
		, std::vector<std::filesystem::path> const& inc
		, int verbose)
	: out_file_(out)
		, ostream_(out.string(), std::ios::trunc | std::ios::binary)
		, inc_(inc)
		, verbose_(verbose)
		, position_()
		, parser_(*this)
	{
		BOOST_ASSERT(ostream_.is_open());
		if (!ostream_.is_open()) {
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"***info : could not open output file [{}]\n", out.string());

		}
		else {
			if (get_verbosity(3)) {
				fmt::print(
					stdout,
					fg(fmt::color::gray) | fmt::emphasis::bold,
					"***info : output file [{}] is open\n", out.string());

			}
		}
	}

	std::filesystem::path const& context::get_output_path() const {
		return out_file_;
	}

	bool context::push(std::filesystem::path const& p)
	{
		position_.push({ p, 0u });
		position_.top().stream_.open(p.string(), std::ios::in);
		if (position_.top().stream_.is_open()) {
			position_.top().stream_.unsetf(std::ios::skipws);
			return true;
		}
		position_.pop();
		return false;
	}

	void context::pop(sanitizer& san)
	{
		if (position_.size() == 1) {
			fmt::print(
				stdout,
				fg(fmt::color::green_yellow) | fmt::emphasis::bold,
				"***info {}: EOD -> {}\n", get_position(), temp_.string());
			san.eof();
			ostream_.flush();
			ostream_.close();
		}
		position_.pop();
	}

	void context::nl(std::size_t line)
	{
		BOOST_ASSERT(!position_.empty());
		position_.top().line_ = line;
	}

	bool context::get_verbosity(int level) const {
		return verbose_ > level;
	}

	std::pair<std::filesystem::path, bool> context::lookup(std::filesystem::path const& inp) {
		auto const p = verify_extension(inp, "docs");
		return resolve_path(inc_, p);
	}

	std::istream_iterator<char> context::get_stream_iterator() {
		return (position_.empty())
			? std::istream_iterator<char>()
			: std::istream_iterator<char>(position_.top().stream_)
			;
	}

	void context::put(symbol const& sym) {
		std::size_t counter{ 0 };
		while (!parser_.put(sym)) {
			if (counter > 12) {
				fmt::print(
					stdout,
					fg(fmt::color::orange) | fmt::emphasis::bold,
					"***warn : Parser has entered an infinite loop. Stopped after [{}] iterations\n", counter);
				break;
			}
			++counter;
		};
	}

	std::string context::get_position() const {
		if (!position_.empty()) {
			std::stringstream ss;
			ss
				<< position_.top().file_
				<< '('
				<< position_.top().line_
				<< ')'
				;
			return ss.str();
		}
		return "eof";
	}

	void context::emit(cyng::object&& obj) {
		auto const bytes = cyng::io::serialize_binary(ostream_, obj);
#ifdef _DEBUG
		//std::cout << "asm.emit " << bytes << " bytes: " << cyng::io::to_typed(obj) << std::endl;
		ostream_.flush();
#endif
	}

	std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext)
	{
		if (!p.has_extension())
		{
			p.replace_extension(ext);
		}
		return p;
	}

	std::pair<std::filesystem::path, bool> resolve_path(std::vector< std::filesystem::path >const& inc, std::filesystem::path p)
	{
		//
		//	search all include paths for the specified file path
		//
		for (auto const& dir : inc)
		{
			if (std::filesystem::exists(dir / p))	return std::make_pair((dir / p), true);
		}

		//
		//	not found - try harder
		//	search all include paths for the specified file name
		//
		for (auto const& dir : inc)
		{
			//	ignore path
			if (std::filesystem::exists(dir / p.filename()))	return std::make_pair((dir / p.filename()), true);
		}

		return std::make_pair(p, false);

	}

}
