

#include <html/code_json.h>
#include <html/formatting.h>

#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>
#include <cyng/io/serializer/json_walker.h>
#include <cyng/obj/algorithm/dom_walker.h>
#include <cyng/parse/json/json_parser.h>

#include <boost/algorithm/string.hpp>

#include <iomanip>
#include <sstream>

namespace dom {
    class walker : public cyng::dom_walker {
      public:
        walker(std::ostream &os, bool line_numbers)
            : os_(os)
            , line_numbers_(line_numbers)
            , nl_(true)
            , line_(1) {
            open_row();
        }

        virtual ~walker() { close_row(); }

        virtual void visit(cyng::object const &obj, cyng::type_code, std::size_t depth, cyng::walker_state state) override {
            // if (is_vector)	os_ << std::string(depth * 2, '\t');

            switch (obj.tag()) {
            case cyng::TC_STRING:
                os_ << "<span class=\"docc-string\">";
                break;
            default:
                break;
            }

            cyng::io::serialize_json(os_, obj);

            switch (obj.tag()) {
            case cyng::TC_STRING:
                os_ << "</span>";
                break;
            default:
                break;
            }

            if (state != cyng::walker_state::LAST) {
                os_ << ", ";
            }
            nl();
        }
        virtual void open(cyng::type_code tag, std::size_t depth, std::size_t size) override {
            switch (tag) {
            case cyng::TC_TUPLE:
            case cyng::TC_DEQUE:
            case cyng::TC_PARAM_MAP:
            case cyng::TC_ATTR_MAP:
            case cyng::TC_PROP_MAP:
                if (nl_)
                    os_ << std::string(depth * 2, '\t');

                os_ << "{";
                if (size > 1) {
                    nl();
                } else {
                    nl_ = false;
                }
                break;
            case cyng::TC_VECTOR:
                if (nl_)
                    os_ << std::string(depth * 2, '\t');

                os_ << "[ ";
                if (size > 1) {
                    nl();
                } else {
                    nl_ = false;
                }
                break;
            default:
                break;
            }
        }
        virtual void close(cyng::type_code tag, std::size_t depth, cyng::walker_state state, cyng::type_code parent_type) override {
            switch (tag) {
            case cyng::TC_TUPLE:
            case cyng::TC_DEQUE:
            case cyng::TC_PARAM_MAP:
            case cyng::TC_ATTR_MAP:
            case cyng::TC_PROP_MAP:
                os_ << std::string(depth * 2, '\t') << "}";
                if (state != cyng::walker_state::LAST) {
                    os_ << ", ";
                }
                nl();
                break;
            case cyng::TC_VECTOR:
                os_ << std::string(depth * 2, '\t') << "]";
                if (state != cyng::walker_state::LAST) {
                    os_ << ", ";
                }
                nl();
                break;
            default:
                break;
            }
        }

        virtual void pair(std::size_t n, std::size_t depth) override {
            if (nl_)
                os_ << std::string(depth * 2, '\t');
            else
                os_ << ' ';

            os_ << "\"" << n << "\": ";
            nl_ = false;
        }
        virtual void pair(std::string const &name, std::size_t depth) override {
            if (nl_)
                os_ << std::string(depth * 2, '\t');
            else
                os_ << ' ';

            os_ << "<span class=\"docc-keyword\">"
                << "\"" << name << "\""
                << "</span>"
                << ": ";
            nl_ = false;
        }
        virtual void pair(cyng::obis const &code, std::size_t depth) override {
            if (nl_)
                os_ << std::string(depth * 2, '\t');
            else
                os_ << ' ';

            os_ << "<span class=\"docc-keyword\">"
                << "\"" << cyng::to_str(code) << "\""
                << "</span>"
                << ": ";
            nl_ = false;
        }

      private:
        void nl() {
            nl_ = true;
            ++line_;
            close_row();
            open_row();
        }
        void open_row() {
            os_ << "<tr>";
            if (line_numbers_) {
                os_ << "<td class=\"docc-num\" data-line-number=\"" << line_ << "\"></td>";
            }
            os_ << "<td class=\"docc-code\">";
        }
        void close_row() {
            os_ << "</td>"
                << "</tr>" << std::endl;
        }
        void simple(cyng::attr_t const &attr, std::size_t depth, cyng::walker_state) {
            if (nl_)
                os_ << std::string(depth * 2, '\t');
            else
                os_ << ' ';

            // os_ << attr.first << '#' << attr.second << std::endl;
            os_ << "\"" << attr.first << "\": ";
            cyng::io::serialize_json(os_, attr.second);
            nl();
        }
        void simple(cyng::param_t const &param, std::size_t depth, cyng::walker_state) {
            if (nl_)
                os_ << std::string(depth * 2, '\t');
            else
                os_ << ' ';

            os_ << "<span class=\"docc-keyword\">"
                << "\"" << param.first << "\""
                << "</span>"
                << ": ";

            switch (param.second.tag()) {
            case cyng::TC_STRING:
                // docc-string
                os_ << "<span class=\"docc-string\">"
                    << "\"" << param.second << "\""
                    << "</span>"
                    << ": ";
                break;
            default:
                cyng::io::serialize_json(os_, param.second);
                break;
            }
            nl();
        }
        void simple(cyng::prop_t const &, std::size_t depth, cyng::walker_state) {}

      private:
        std::ostream &os_;
        bool const line_numbers_;
        bool nl_; //	last out was an NL
        std::size_t line_;
    };

    void json_to_html(std::ostream &os, std::istream_iterator<char> start, std::istream_iterator<char> end, bool numbers) {
        cyng::json::parser jp([&](cyng::object &&obj) {
            // std::cout << obj << std::endl;
            walker w(os, numbers);
            cyng::traverse(obj, w);
        });

        os << "<table style=\"tab-size: 2;\" class=\"docc-code\" >" << std::endl
           << "<tbody style=\"white-space: pre;\">" << std::endl;
        jp.read(start, end);
        os << "</tbody>" << std::endl << "</table>" << std::endl;
    }

} // namespace dom
