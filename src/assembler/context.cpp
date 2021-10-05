#include <context.h>
#include <sanitizer.h>
#include <symbol.h>

//#include <utils.h>
#include <iostream>
#include <string>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>

namespace docscript {
	context::context(std::filesystem::path const& temp
		, std::filesystem::path out
		, std::vector<std::filesystem::path> const& inc
		, int verbose)
	: temp_(temp)
		, ostream_(temp.string(), std::ios::trunc)
		, out_file_(out)
		, inc_(inc)
		, verbose_(verbose)
		, position_()
		//, method_table_()
		//, parser_(*this)
	{
		BOOST_ASSERT(ostream_.is_open());
		if (!ostream_.is_open()) {
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"***info : could not open temporary output file [{}]\n", temp_.string());

		}
		else {
			if (get_verbosity(3)) {
				fmt::print(
					stdout,
					fg(fmt::color::gray) | fmt::emphasis::bold,
					"***info : temporary output file [{}] is open\n", temp_.string());

			}

			write_bom(ostream_);
			//init_method_table(method_table_);

		}
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
			san.eof();
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
		//while (!parser_.put(sym)) {
		//	if (counter > 12) {
		//		fmt::print(
		//			stdout,
		//			fg(fmt::color::orange) | fmt::emphasis::bold,
		//			"***warn : Parser has entered an infinite loop. Stopped after [{}] iterations\n", counter);
		//		break;
		//	}
		//	++counter;
		//};
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

	void context::emit(std::string const& s) {
		ostream_ << s 
#ifdef _DEBUG
			<< std::flush
#endif
			;
	}

	void write_bom(std::ostream& os) {
		//
		//	write BOM: 0xEF, 0xBB, 0xBF
		//
		os.put(static_cast<char>(0xEF));
		os.put(static_cast<char>(0xBB));
		os.put(static_cast<char>(0xBF));

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
