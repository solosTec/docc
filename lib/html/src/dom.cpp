/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Sylko Olzscher
 *
 */

#include <html/dom.hpp>
#include <boost/algorithm/string.hpp>

namespace dom
{

	node::node()
	{}

	std::string	node::operator()(std::size_t intend) const
	{
		std::stringstream ss;
		serialize(ss, intend);
		return ss.str();
	}

	text::text()
		: value_()
	{}

	text::text(std::string value)
		: value_(std::move(value))
	{}

	void text::serialize(std::ostream& os) const
	{
		os << value_;
	}

	void text::serialize(std::ostream& os, std::size_t depth) const
	{
		serialize(os);
	}

	node_types text::get_node_type() const
	{
		return node_types::TEXT;
	}

	text::operator bool() const
	{
		return !value_.empty();
	}

	attribute::attribute()
		: name_(std::string("href"))
		, value_()
	{}

	attribute::attribute(std::string name)
		: name_(patch_attribute_name(name))
		, value_()
	{}

	void attribute::serialize(std::ostream& os) const
	{
		os
			<< name_
			;

		if (!value_.empty()) {
			os
				<< '='
				<< '"'
				<< value_
				<< '"'
				;
		}
	}

	void attribute::serialize(std::ostream& os, std::size_t depth) const
	{
		serialize(os);
	}

	node_types attribute::get_node_type() const
	{
		return node_types::ATTRIBUTE;
	}

	element::element()
		: tag_(std::string("div"))
		, attr_()
		, children_()
		, text_()
	{}

	element::element(std::string tag)
		: tag_(std::move(tag))
		, attr_()
		, children_()
		, text_()
	{}

	element::element(std::string tag, text value)
		: tag_(std::move(tag))
		, attr_()
		, children_()
		, text_(std::move(value))
	{}

	node_types element::get_node_type() const
	{
		return node_types::ELEMENT;
	}

	element& element::operator+=(attribute&& attr)
	{
		attr_.push_back(std::move(attr));
		return *this;
	}

	element& element::operator+=(element&& e)
	{
		children_.push_back(std::move(e));
		return *this;
	}

	void element::assign_impl(attribute&& attr)
	{
		attr_.push_back(std::move(attr));
	}

	void element::assign_impl(element&& e)
	{
		children_.push_back(std::move(e));
	}

	void element::assign_impl(text&& t)
	{
		text_ = t;
	}



	void element::serialize(std::ostream& os) const
	{
		os
			<< '<'
			<< tag_.get()
			;

		for (auto const& attr : attr_) {
			os << ' ';
			attr.serialize(os);
		}

		if (children_.empty()) {
			if (text_) {

				os << ">";
				text_.serialize(os);
				os
					<< "</"
					<< tag_.get()
					<< ">"
					;
			}
			else {
				os << ' ' << '/' << '>';
			}
		}
		else {

			//
			//	serialize attributes
			//
			bool initialized{ false };
			for (auto const child : children_) {
				if (initialized) {
					os << ' ';
				}
				else {
					initialized = true;
				}
				child.serialize(os);
			}
		}
	}

	void element::serialize(std::ostream& os, std::size_t depth) const
	{
		if (depth != 0) {
			os
				<< std::endl
				<< std::string(depth, '\t')
				;
		}
		os
			<< '<'
			<< tag_.get()
			;

		for (auto const& attr : attr_) {
			os << ' ';
			attr.serialize(os);
		}

		if (children_.empty()) {
			if (text_) {

				os << ">";
				text_.serialize(os);
				os
					<< "</"
					<< tag_.get()
					<< ">"
					;
			}
			else {
				os << ' ' << '/' << '>';
			}
		}
		else {

			os << ">";

			//
			//	serialize children
			//
			bool initialized{ false };
			for (auto const child : children_) {
				if (initialized) {
					os << ' ';
				}
				else {
					initialized = true;
				}
				child.serialize(os, depth + 1);
			}

			//
			//	check indentation
			//
			auto const tag = tag_.get();
			if (boost::algorithm::equals(tag, "div")
				|| boost::algorithm::equals(tag, "ul")
				|| boost::algorithm::equals(tag, "ol")
				|| boost::algorithm::equals(tag, "li")
				|| boost::algorithm::equals(tag, "nav")
				|| boost::algorithm::equals(tag, "footer")
				|| boost::algorithm::equals(tag, "section")) {

				os
					<< std::endl
					<< std::string(depth, '\t')
					;
			}

			os
				<< text_.value_
				<< "</"
				<< tag_.get()
				<< ">"
				;

		}
	}

	std::string patch_attribute_name(std::string name)
	{
		std::replace(name.begin(), name.end(), '_', '-');
		return name;
	}


}
