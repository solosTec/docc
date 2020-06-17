/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 


#include "page.h"

#include <boost/uuid/uuid_generators.hpp>

namespace docscript
{

	page::page()
		: name_()
		, title_()
		, enabled_(false)
		, source_()
		, tag_(boost::uuids::nil_uuid())
		, file_()
		, fragment_()
		, type_()
		, css_()
		, menu_()
		, nav_()
		, footer_()
	{}

	page::page(std::string const& name
		, std::string const& title
		, bool enabled
		, std::string const& source
		, boost::uuids::uuid tag
		, cyng::filesystem::path file
		, cyng::filesystem::path fragment
		, std::string const& type
		, std::string const& css
		, std::string const& menu
		, cyng::filesystem::path nav
		, std::string const& footer)
	: name_(name)
		, title_(title)
		, enabled_(enabled)
		, source_(source)
		, tag_(tag)
		, file_(file)
		, fragment_(fragment)
		, type_(type)
		, css_(css)
		, menu_(menu)
		, nav_(nav)
		, footer_(footer)
	{}

	std::string const& page::get_name() const
	{
		return name_;
	}

	std::string const& page::get_title() const
	{
		return title_;
	}

	bool page::is_enabled() const
	{
		return enabled_;
	}

	std::string const& page::get_source() const
	{
		return source_;
	}

	boost::uuids::uuid const page::get_tag() const
	{
		return tag_;
	}

	cyng::filesystem::path const& page::get_file() const
	{
		return file_;
	}

	cyng::filesystem::path const& page::get_fragment() const
	{
		return fragment_;
	}

	std::string const& page::get_type() const
	{
		return type_;
	}

	std::string const& page::get_css() const
	{
		return css_;
	}

	std::string const& page::get_menu() const
	{
		return menu_;
	}

	cyng::filesystem::path const& page::get_nav() const
	{
		return nav_;
	}

	std::string const& page::get_footer() const
	{
		return footer_;
	}

	bool page::has_menu() const
	{
		return !get_menu().empty();
	}

	bool page::has_footer() const
	{
		return !get_footer().empty();
	}

	bool page::has_css() const
	{
		return !get_css().empty();
	}

}
