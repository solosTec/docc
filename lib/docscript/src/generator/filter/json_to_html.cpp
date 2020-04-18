/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "json_to_html.h"

#include <cyng/dom/tree_walker.h>
#include <cyng/json.h>
#include <cyng/dom/tree_walker.h>
#include <cyng/object.h>
#include <cyng/core/class_interface.h>
#include <cyng/intrinsics/traits/tag.hpp>
#include <cyng/set_cast.h>

#include <iostream>
#include <iomanip>

#include <boost/regex/pending/unicode_iterator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace docscript	
{
	class json_walker : public cyng::tree_walker
	{
	public:
		json_walker(std::ostream& os, bool linenumbers, boost::uuids::uuid tag)
			: os_(os)
			, linenumbers_(linenumbers)
			, tag_(tag)
			, line_(0)
		{
			if (linenumbers_) {
				os_ << nl();
			}
		}

		/**
		 * ToDo: return size of container to optimize formatting (depended from size of the container)
		 */
		virtual bool enter_node(std::size_t depth, cyng::object const& obj, std::size_t idx, std::size_t total, cyng::object parent) override
		{

			//std::cout << "enter " << depth << " - " << idx << '/' << total << " - " << obj.get_class().type_name() << " - " << cyng::json::to_string(obj) << std::endl;

			switch (obj.get_class().tag()) {
			case cyng::TC_TUPLE:
				//
				//	If the parent element was a also a container insert a NL.
				//	
				if (parent.get_class().tag() == cyng::TC_TUPLE || parent.get_class().tag() == cyng::TC_VECTOR) {
					os_
						<< nl()
						<< indentation(depth)
						;
				}

				os_ << '{';

				if (cyng::to_tuple(obj).empty()) {
					os_ << '}';
				}
				break;
			case cyng::TC_VECTOR:
				os_ << '[';
				if (cyng::to_vector(obj).empty()) {
					os_ << ']';
				}
				break;
			case cyng::TC_PARAM:
			{
				using type = typename std::tuple_element<cyng::type_code::TC_PARAM, cyng::traits::tag_t>::type;
				const type* ptr = cyng::object_cast<type>(obj);
				BOOST_ASSERT_MSG(ptr != nullptr, "invalid type info");
				os_
					<< nl()
					<< indentation(depth)
					<< "<span style = \"color: blue\">"
					<< "&quot;"
					<< ptr->first
					<< "&quot;"
					<< "</span>"
					<< ':'
					<< ' '
					;
			}
			break;

			case cyng::TC_STRING:
				os_
					<< "<span style = \"color: brown\">"
					<< cyng::json::to_string(obj)
					<< "</span>"
					;
				break;
			default:
				os_ << cyng::json::to_string(obj);
				break;
			}
			return true;	//	continue
		}

		virtual void leave_node(std::size_t depth, cyng::object const& obj, std::size_t idx, std::size_t total, cyng::object previous) override
		{
			//std::cout << "leave " << depth << " - " << idx << '/' << total << " - "  << obj.get_class().type_name() << " - " << cyng::json::to_string(obj) << std::endl;

			switch (obj.get_class().tag()) {
			case cyng::TC_TUPLE:
				os_
					<< nl()
					<< indentation(depth)
					<< '}'
					;
				if ((idx != 1) && (depth != 0))	os_ << ',';
				//if ((idx != 1) && total != 1) {
				//	os_
				//		<< std::endl
				//		<< indentation(depth)
				//		;
				//}
				break;
			case cyng::TC_VECTOR:
				//
				//	If the previous element was a also a container insert a NL.
				//	
				if (previous.get_class().tag() == cyng::TC_TUPLE || previous.get_class().tag() == cyng::TC_VECTOR) {
					os_
						<< nl()
						<< indentation(depth)
						;
				}
				os_ << ']';
				if (idx != 1)	os_ << ',';
				break;
			case cyng::TC_PARAM:
			case cyng::TC_ATTR:
				break;
			default:
				if (idx != 1)	os_ << ',';
				break;
			}
		}

	private:
		std::string indentation(std::size_t depth)
		{
			return (depth == 0)
				? ""
				: "  " + indentation(depth - 1)
				;
		}
		std::string nl()
		{
			std::stringstream ss;
			if (line_ != 0u) {
				ss << "</code>";
			}
			ss
				<< std::endl
				<< "<code>"
				;
			++line_;
			if (linenumbers_)
			{
				ss
					<< "<span style = \"color: DarkCyan; font-size: smaller;\" id=\""
					<< tag_
					<< '-'
					<< line_
					<< "\">"
					<< std::setw(4)
					<< std::setfill(' ')
					<< line_
					<< "</span> "
					;
			}
			return ss.str();
		}

	private:
		std::ostream& os_;
		bool const linenumbers_;
		boost::uuids::uuid const tag_;
		std::size_t line_;
	};


	json_to_html::json_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}


	void json_to_html::convert(std::ostream& os, std::string const& inp)
	{
		//
		//	parse into an object hierarchy
		//
		auto obj = cyng::json::read(inp);

		//
		//	walk over object tree and generate formatted JSON output
		//
		json_walker walker(os, linenumbers_, tag_);
		//os << "<code>";
		traverse(obj, walker, 0);
		os << "</code>";
	}


}


