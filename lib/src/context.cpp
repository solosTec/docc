#include <context.h>

#include <utils.h>
#include <iostream>
#include <string>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>

namespace docscript {
	context::context(std::filesystem::path const& temp
		, std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, int verbose)
		: temp_(temp)
		, out_file_(out)
		, inc_(inc)
		, verbose_(verbose)
		, position_()
		, parser_(*this)
	{}

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

	bool context::pop()
	{
		position_.pop();
		return position_.empty();
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
		auto const p = verify_extension(inp, "docscript");
		return resolve_path(inc_, p);
	}

	std::istream_iterator<char> context::get_stream_iterator() {
		return (position_.empty())
			? std::istream_iterator<char>()
			: std::istream_iterator<char>(position_.top().stream_)
			;
	}

	void context::put(symbol const& sym) {
		parser_.put(sym);
	}

	std::string context::get_position() const {
		if (!position_.empty()) {
			std::stringstream ss;
			ss
				<< '['
				<< position_.top().file_
				<< ']'
				<< ' '
				<< position_.top().line_
				;
			return ss.str();
		}
		return "";
	}

}
