/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 

#ifndef DOCC_PAGE_H
#define DOCC_PAGE_H


#include <cyng/compatibility/file_system.hpp>
#include <string>
#include <boost/uuid/uuid.hpp>

namespace docscript
{
	class page
	{
	public:
		page();
		page(std::string const& name
			, std::string const& title
			, bool enabled
			, std::string const& source
			, boost::uuids::uuid tag
			, cyng::filesystem::path file
			, cyng::filesystem::path fragment
			, std::string const& type
			, std::string const& css_page
			, std::string const& menu
			, cyng::filesystem::path nav
			, std::string const& footer);


		std::string const& get_name() const;
		std::string const& get_title() const;
		bool is_enabled() const;
		std::string const& get_source() const;
		boost::uuids::uuid const get_tag() const;
		cyng::filesystem::path const& get_file() const;
		cyng::filesystem::path const& get_fragment() const;
		std::string const& get_type() const;
		std::string const& get_css() const;
		std::string const& get_menu() const;
		cyng::filesystem::path const& get_nav() const;
		std::string const& get_footer() const;

		bool has_menu() const;
		bool has_footer() const;
		bool has_css() const;

	private:
		std::string const name_;
		std::string const title_;
		bool const enabled_;
		std::string const source_;
		boost::uuids::uuid const tag_;
		cyng::filesystem::path const file_;
		cyng::filesystem::path const fragment_;
		std::string const type_;
		std::string const css_;
		std::string const menu_;
		cyng::filesystem::path const nav_;
		std::string const footer_;

	};

}

#endif
