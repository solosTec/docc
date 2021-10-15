#include <context.h>
#include <sanitizer.h>

#include <utils.h>
#include <iostream>
#include <string>

#include <fmt/core.h>
#include <fmt/color.h>

#include <boost/assert.hpp>

namespace docscript {
	context::context(std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, int verbose)
	: out_file_(out)
		, ostream_(out_file_.string(), std::ios::trunc)
		, inc_(inc)
		, verbose_(verbose)
		, position_()
		, method_table_()
		, parser_(*this)
	{
		BOOST_ASSERT(ostream_.is_open());
		if (!ostream_.is_open()) {
			fmt::print(
				stdout,
				fg(fmt::color::crimson) | fmt::emphasis::bold,
				"***info : could not open output file [{}]\n", out_file_.string());

		}
		else {
			if (get_verbosity(3)) {
				fmt::print(
					stdout,
					fg(fmt::color::gray) | fmt::emphasis::bold,
					"***info : output file [{}] is open\n", out_file_.string());

			}

			write_bom(ostream_);
			init_method_table(method_table_);

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

	void context::emit(std::string const& s) {
		ostream_ << s 
#ifdef _DEBUG
			<< std::flush
#endif
			;
	}

	std::optional<method> context::lookup_method(std::string const& name) const {
		auto const pos = method_table_.find(name);
		return (pos != method_table_.end())
			? pos->second
			: std::optional<method>{}
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
	void init_method_table(std::map<std::string, method>& table) {

		//
		//	predefined methods
		//
		insert_method(table, method("header", return_type::STRING, parameter_type::MAP, false, { "title", "level", "tag" }));
		insert_method(table, method("h1", return_type::STRING, parameter_type::VECTOR, false));
		insert_method(table, method("h2", return_type::STRING, parameter_type::VECTOR, false));
		insert_method(table, method("h3", return_type::STRING, parameter_type::VECTOR, false));
		insert_method(table, method("h4", return_type::STRING, parameter_type::VECTOR, false));
		insert_method(table, method("h5", return_type::STRING, parameter_type::VECTOR, false));
		insert_method(table, method("h6", return_type::STRING, parameter_type::VECTOR, false));

		//	 pilgrow (�) = paragraph
		insert_method(table, method(std::string("\xc2\xb6"), return_type::STRING, parameter_type::VECTOR, false));

		insert_method(table, method("label", return_type::STRING, parameter_type::VECTOR, true));
		insert_method(table, method("ref", return_type::STRING, parameter_type::VECTOR, true));

		insert_method(table, method("get", return_type::STRING, parameter_type::VECTOR, true));
		insert_method(table, method("set", return_type::STRING, parameter_type::MAP, true));	//	key, value
		insert_method(table, method("resource", return_type::STRING, parameter_type::MAP, true, {"name", "mime", "cache", "url"}));
		insert_method(table, method("i", return_type::STRING, parameter_type::VECTOR, true));
		insert_method(table, method("b", return_type::STRING, parameter_type::VECTOR, true));
		insert_method(table, method("quote", return_type::STRING, parameter_type::VECTOR, true));	//	same as "..."

		//	calculate return value count requires to determine "count" value at compile time
		insert_method(table, method("repeat", return_type::STRING, parameter_type::MAP, true, { "count", "value", "sep"}));

		//insert_method(table, method("rgb", return_type::U32, parameter_type::VECTOR, true, { "r", "g", "b" }));
		//insert_method(table, method("rgba", return_type::U32, parameter_type::VECTOR, true, { "r", "g", "b", "a"}));

		insert_method(table, method("now", return_type::TIMESTAMP, parameter_type::VECTOR, true));

	}

	bool insert_method(std::map<std::string, method>& table, method&& m) {
		return table.emplace(m.get_name(), m).second;
	}


}
