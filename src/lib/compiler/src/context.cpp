#include <docc/context.h>
#include <docc/sanitizer.h>

#include <docc/utils.h>
#include <iostream>
#include <string>

#include <fmt/color.h>
#include <fmt/core.h>

#include <boost/assert.hpp>

namespace docscript {
    context::context(std::filesystem::path out, std::vector<std::filesystem::path> inc, int verbose)
        : out_file_(out)
        , ostream_(out_file_.string(), std::ios::trunc)
        , inc_(inc)
        , verbose_(verbose)
        , position_()
        , method_table_()
        , parser_(*this)
        , stats_() {
        BOOST_ASSERT(ostream_.is_open());
        if (!ostream_.is_open()) {
            fmt::print(
                stdout,
                fg(fmt::color::crimson) | fmt::emphasis::bold,
                "***info : could not open output file [{}]\n",
                out_file_.string());

        } else {
            if (get_verbosity(3)) {
                fmt::print(
                    stdout, fg(fmt::color::gray) | fmt::emphasis::bold, "***info : output file [{}] is open\n", out_file_.string());
            }

            write_bom(ostream_);

            const std::time_t t_c = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
            ostream_ << "; created at " << std::put_time(std::localtime(&t_c), "@%FT%T\n") << std::endl;

            init_method_table(method_table_);
        }
    }

    bool context::push(std::filesystem::path const &p) {
        position_.push({p, 0u});
        position_.top().stream_.open(p.string(), std::ios::in);
        if (position_.top().stream_.is_open()) {
            position_.top().stream_.unsetf(std::ios::skipws);
            return true;
        }
        position_.pop();
        return false;
    }

    void context::pop(sanitizer &san) {
        if (position_.size() == 1) {
            san.eof();
        }
        position_.pop();
    }

    void context::nl(std::size_t line) {
        BOOST_ASSERT(!position_.empty());
        position_.top().line_ = line;
    }

    bool context::get_verbosity(int level) const { return verbose_ > level; }

    std::pair<std::filesystem::path, bool> context::lookup(std::filesystem::path const &inp, std::string ext) const {
        //	ext = "docscript"
        auto const p = verify_extension(inp, ext);
        return resolve_path(inc_, p);
    }

    std::pair<std::istream_iterator<char>, std::istream_iterator<char>> context::get_stream_range() {
        return (position_.empty())
                   ? std::make_pair(std::istream_iterator<char>(), std::istream_iterator<char>())
                   : std::make_pair(std::istream_iterator<char>(position_.top().stream_), std::istream_iterator<char>());
    }

    void context::put(symbol const &sym) {
        std::size_t counter{0};
        while (!parser_.put(sym)) {
            if (counter > 12) {
                fmt::print(
                    stdout,
                    fg(fmt::color::orange) | fmt::emphasis::bold,
                    "***warn : Parser has entered an infinite loop. Stopped after [{}] iterations\n",
                    counter);
                break;
            }
            //
            //  statistics
            //
            auto pos = stats_.find(sym.type_);
            if (pos == stats_.end()) {
                stats_.emplace(sym.type_, 0u);
            } else {
                ++pos->second;
            }
            ++counter;
        };
    }

    std::string context::get_position() const {
        if (!position_.empty()) {
            std::stringstream ss;
            ss << position_.top().file_ << '(' << position_.top().line_ << ')';
            return ss.str();
        }
        return "eof";
    }

    void context::emit(std::string const &s) {
        ostream_ << s
#ifdef _DEBUG
                 << std::flush
#endif
            ;
    }

    std::optional<method> context::lookup_method(std::string const &name) const {
        auto const pos = method_table_.find(name);
        return (pos != method_table_.end()) ? pos->second : std::optional<method>{};
    }

    std::filesystem::path const &context::get_output_path() const { return out_file_; }

    std::map<symbol_type, std::size_t> const &context::get_stats() const { return stats_; }

    void write_bom(std::ostream &os) {
        //
        //	write BOM: 0xEF, 0xBB, 0xBF
        //
        os.put(static_cast<char>(0xEF));
        os.put(static_cast<char>(0xBB));
        os.put(static_cast<char>(0xBF));
    }
    void init_method_table(std::map<std::string, method> &table) {

        //
        //	predefined methods
        //
        insert_method(table, method("header", parameter_type::MAP, false, {"title", "level", "tag"}));
        insert_method(table, method("h1", parameter_type::VECTOR, false));
        insert_method(table, method("h2", parameter_type::VECTOR, false));
        insert_method(table, method("h3", parameter_type::VECTOR, false));
        insert_method(table, method("h4", parameter_type::VECTOR, false));
        insert_method(table, method("h5", parameter_type::VECTOR, false));
        insert_method(table, method("h6", parameter_type::VECTOR, false));

        insert_method(table, method("figure", parameter_type::MAP, false, {"caption", "source"}));
        insert_method(table, method("toc", parameter_type::MAP, false, {}));

        //	 pilgrow (¶) = paragraph
        insert_method(table, method(std::string("\xc2\xb6"), parameter_type::VECTOR, false));

        insert_method(table, method("label", parameter_type::VECTOR, true));
        insert_method(table, method("ref", parameter_type::VECTOR, true));

        insert_method(table, method("get", parameter_type::VECTOR, true));
        insert_method(table, method("set", parameter_type::MAP, true));  //	key, value
        insert_method(table, method("meta", parameter_type::MAP, true)); //	key, value

        insert_method(table, method("resource", parameter_type::MAP, true, {"name", "mime", "cache", "url"}));
        insert_method(table, method("i", parameter_type::VECTOR, true));
        insert_method(table, method("b", parameter_type::VECTOR, true));
        insert_method(table, method("tt", parameter_type::VECTOR, true)); //	mono font
        // insert_method(table, method("number", parameter_type::VECTOR, true));	//	<number>

        insert_method(table, method("esc", parameter_type::VECTOR, true));   //	represent certain special characters in the
                                                                             // target language (HTML, LaTeX, ...)
        insert_method(table, method("quote", parameter_type::VECTOR, true)); //	same as "..."
        insert_method(table, method("range", parameter_type::VECTOR, true)); //	collect a vector
        insert_method(table, method("fuse", parameter_type::VECTOR, true));  //	concatenate without spaces
        insert_method(table, method("cat", parameter_type::MAP, true, {"sep", "text"})); //	concatenate with separator

        //	calculate return value count requires to determine "count" value at compile time
        insert_method(table, method("repeat", parameter_type::MAP, true, {"count", "value", "sep"}));
        insert_method(table, method("currency", parameter_type::MAP, true, {"name", "value"}));

        // insert_method(table, method("rgb", parameter_type::MAP, true, { "r", "g", "b" }));
        // insert_method(table, method("rgba", parameter_type::MAP, true, { "r", "g", "b", "a"}));

        insert_method(table, method("now", parameter_type::MAP, true));
        insert_method(table, method("uuid", parameter_type::MAP, true));

        //	linenumber is optional (block)
        insert_method(table, method("code", parameter_type::MAP, false, {"language", "source"}));
        insert_method(table, method("table", parameter_type::MAP, false, {"source", "title"}));
        insert_method(table, method("tree", parameter_type::MAP, false, {"root"}));
    }

    bool insert_method(std::map<std::string, method> &table, method &&m) { return table.emplace(m.get_name(), m).second; }
} // namespace docscript
