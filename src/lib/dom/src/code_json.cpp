
#include <html/code_json.h>
#include <html/formatting.h>

#include <cyng/parse/json/json_parser.h>
#include <cyng/io/serializer/json_walker.h>
#include <cyng/io/serialize.h>
#include <cyng/obj/algorithm/dom_walker.h>

#include <sstream>
#include <iomanip>

#include <boost/algorithm/string.hpp>

namespace dom
{
	class walker : public cyng::dom_walker {
	public:
		walker(std::ostream& os, bool line_numbers)
			: os_(os)
			, line_numbers_(line_numbers)
			, nl_(true)
			, line_(0)
		{
			open_row();
		}

		virtual ~walker() {
			close_row();
		}

		virtual void visit(cyng::object const& obj, cyng::type_code, std::size_t depth, cyng::walker_state state, bool is_vector) {
			if (is_vector)	os_ << std::string(depth * 2, '\t');

			switch (obj.rtti().tag()) {
			case cyng::TC_STRING:
				os_	<< "<span class=\"docc-string\">";
				break;
			default:
				break;
			}

			cyng::io::serialize_json(os_, obj);

			switch (obj.rtti().tag()) {
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
		virtual void open(cyng::type_code tag, std::size_t depth, std::size_t size) {
			switch (tag) {
			case cyng::TC_TUPLE:
			case cyng::TC_DEQUE:
			case cyng::TC_PARAM_MAP:
			case cyng::TC_ATTR_MAP:
				if (nl_)	os_ << std::string(depth * 2, '\t');

				os_ << "{";
				if (size > 1) {
					nl();
				}
				else {
					nl_ = false;
				}
				break;
			case cyng::TC_VECTOR:
				if (nl_)	os_ << std::string(depth * 2, '\t');

				os_ << "[ ";
				if (size > 1) {
					nl();
				}
				else {
					nl_ = false;
				}
				break;
			default:
				break;
			}
		}
		virtual void close(cyng::type_code tag, std::size_t depth, cyng::walker_state state) {
			switch (tag) {
			case cyng::TC_TUPLE:
			case cyng::TC_DEQUE:
			case cyng::TC_PARAM_MAP:
			case cyng::TC_ATTR_MAP:
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

		virtual void pair(std::size_t n, std::size_t depth) {
			if (nl_)	os_ << std::string(depth * 2, '\t');
			else os_ << ' ';

			os_
				<< "\""
				<< n
				<< "\": "
				;
			nl_ = false;
		}
		virtual void pair(std::string name, std::size_t depth) {
			if (nl_)	os_ << std::string(depth * 2, '\t');
			else os_ << ' ';

			os_
				<< "<span class=\"docc-key\">"
				<< "\""
				<< name
				<< "\""
				<< "</span>"
				<< ": "
				;
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
			os_ << "</td>" << "</tr>" << std::endl;
		}

	private:
		std::ostream& os_;
		bool const line_numbers_;
		bool nl_;	//	last out was an NL
		std::size_t line_;
	};

	void json_to_html(std::ostream& os, std::istream_iterator<char> start, std::istream_iterator<char> end, bool numbers) {
		cyng::json::parser jp([&](cyng::object&& obj) {
			//std::cout << obj << std::endl;
			walker w(os, numbers);
			cyng::traverse(obj, w);
			});

		os << "<table style=\"tab-size: 2;\">" << std::endl << "<tbody style=\"white-space: pre;\">" << std::endl;
		jp.read(start, end);
		os << "</tbody>" << std::endl << "</table>" << std::endl;

	}


}
