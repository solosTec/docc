/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2018 Sylko Olzscher 
 * 
 */ 


#include "batch.h"
#include "../../src/driver.h"

#include <cyng/json.h>
#include <cyng/dom/reader.h>
#include <cyng/io/serializer.h>
#include <cyng/io/io_chrono.hpp>
#include <cyng/factory.h>

#include <boost/range/iterator_range.hpp>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/algorithm/string.hpp>
#include <iostream>

namespace docscript
{

	batch::batch(std::vector< std::string >const& inc
		, int verbose)
	: includes_(inc.begin(), inc.end())
		, verbose_(verbose)
		, index_()
	{}

	batch::~batch()
	{}

	int batch::run(boost::filesystem::path const& inp
		, boost::filesystem::path const& out
		, bool gen_robot
		, bool gen_sitemap)
	{
		if (boost::filesystem::is_directory(inp)) {

			for (auto const& entry : boost::make_iterator_range(boost::filesystem::directory_iterator(inp), {})) {

				process_file(entry, out);
			}

			//
			//	generate index file
			//
			generate_index(out, gen_robot, gen_sitemap);

			return EXIT_SUCCESS;
		}

		//
		//	not a directory
		//
		std::cerr << "***error: " << inp << " is not a directory" << std::endl;
		return EXIT_FAILURE;
	}

	void batch::process_file(boost::filesystem::path const& inp
		, boost::filesystem::path const& out)
	{
		//
		//	exclude all files without the extension ".docscript"
		//
		if (inp.has_extension()) {

			if (boost::algorithm::equals(".docscript", inp.extension().string())) {

				//
				//	generate some temporary file names for intermediate files
				//
				boost::filesystem::path tmp = boost::filesystem::temp_directory_path() / (boost::filesystem::path(inp).filename().string() + ".bin");

				//
				//	Construct driver instance
				//
				driver d(includes_, verbose_);

				//
				//	output file
				//
				boost::filesystem::path out_file = out / inp.filename();
				out_file.replace_extension(".html");

				if (verbose_ > 0) {
					std::cout << "***info: processing file "
						<< inp
						<< " ==> "
						<< out_file
						<< std::endl;
				}

				//
				//	Start driver with the main/input file
				//
				d.run(boost::filesystem::path(inp).filename()
					, tmp
					, out_file
					, true	//	only HTML body
					, false	//	generate meta data
					, false	//	index
					, "article");

				index_.emplace(out_file.string(), cyng::make_object(d.get_meta()));
			}
		}
	}

	void batch::generate_index(boost::filesystem::path const& out
		, bool gen_robot
		, bool gen_sitemap)
	{
		//
		//	chronological order
		//
		auto const chrono_index = get_sorted();

		//
		//	generate index page
		//
		generate_index_page(out, chrono_index);

		//
		//	generate index map
		//
		generate_index_map(out, chrono_index);

		if (gen_robot) {

			//
			//	generate robots.txt
			//
			generate_robots_txt(out, chrono_index, gen_sitemap);
		}

		if (gen_sitemap) {

			//
			//	generate sitemap
			//
			generate_sitemap(out, chrono_index);
		}

	}

	void batch::generate_index_page(boost::filesystem::path const& out
		, chrono_idx_t const& idx)
	{
		boost::filesystem::path p = out / "index.html";

		std::ofstream ofs(p.string(), std::ios::out | std::ios::trunc);
		if (!ofs.is_open())
		{
			std::cerr
				<< "***error: cannot open index page "
				<< p
				<< std::endl;

		}
		else
		{
			ofs
				<< "<div class=\"docscript\">"
				<< std::endl
				;

			//
			//	reverse iterate over all meta data
			//
			for (auto const& meta : boost::adaptors::reverse(idx)) {

				auto const reader = cyng::make_reader(meta.second);

				auto const title = cyng::value_cast<std::string>(reader.get("title"), "title");
				auto const file_name = cyng::value_cast<std::string>(reader.get("file-name"), "file-name");
				auto const slug = cyng::value_cast<std::string>(reader.get("slug"), "slug");
				auto const entropy = reader.get("text-entropy");	// double
				auto const symbols = reader.get("input-symbols");	//	size_t
				auto const released = cyng::value_cast(reader.get("released"), std::chrono::system_clock::now());

					//
					//	assume that the path has depth of one
					//
				auto const blog_path = out.filename().string();

				//	<a href="#" title="4860 characters" onclick="load_page(&quot;test.html&quot;);">docScript Test</a>

				ofs
					<< "\t<div>"
					<< std::endl
					<< "\t\t<a href=\"/"
					<< blog_path
					<< "/posts?slug="
					<< slug
					<< "\" onclick=\"load_page('"
					<< file_name
					//	don't follow href
					<< "'); return false;\" title=\""
					<< title
					<< ' '
					<< "input symbols with an entropy of "
					<< cyng::io::to_str(entropy)
					<< "\">"
					<< title
					<< "</a>"
					<< std::endl
					<< "\t\t<p class=\"docscript-timestamp\">"
					<< cyng::date_to_str(released)
					<< "</p>"
					<< std::endl
					<< "\t</div>"
					<< std::endl
					;
			}

			ofs
				<< "</div>"
				<< std::endl
				;

		}
	}

	void batch::generate_index_map(boost::filesystem::path const& out, chrono_idx_t const& idx)
	{
		boost::filesystem::path p = out / "index.json";

		std::ofstream ofs(p.string(), std::ios::out | std::ios::trunc);
		if (!ofs.is_open())
		{
			std::cerr
				<< "***error: cannot open index map "
				<< p
				<< std::endl;

		}
		else
		{
			//
			//	data vector of all available posts
			//
			cyng::vector_t posts;

			//
			//	reverse iterate over all meta data
			//
			std::size_t index{ 0 };
			for (auto const& meta : boost::adaptors::reverse(idx)) {

				auto const reader = cyng::make_reader(meta.second);

				auto const title = cyng::value_cast<std::string>(reader.get("title"), "title");
				auto const file_name = cyng::value_cast<std::string>(reader.get("file-name"), "file-name");
				auto const slug = cyng::value_cast<std::string>(reader.get("slug"), "slug");
				auto const entropy = reader.get("text-entropy");	// double
				auto const symbols = reader.get("token-count");	//	size_t
				auto const size = reader.get("total-file-size");	//	size_t
				auto const released = cyng::value_cast(reader.get("released"), std::chrono::system_clock::now());

				auto map = cyng::param_map_factory("title", title)
					("file-name", file_name)
					("slug", slug)
					("token-count", symbols)
					("size", size)
					("index", index)
					("entropy", entropy)
					();

				posts.push_back(map);
				++index;
			}

			//
			//	write as JSON file
			//
			cyng::json::write(ofs, cyng::make_object(posts));

		}
	}

	void batch::generate_robots_txt(boost::filesystem::path const& out, chrono_idx_t const& idx, bool gen_sitemap)
	{
		boost::filesystem::path p = out / "robots.txt";

		std::ofstream ofs(p.string(), std::ios::out | std::ios::trunc);
		if (!ofs.is_open())
		{
			std::cerr
				<< "***error: cannot robot file"
				<< p
				<< std::endl;

		}
		else {
			//
			//	assume that the path has depth of one
			//
			auto const blog_path = out.filename().string();

			ofs
				<< "User-agent: *"
				<< std::endl
				<< "Allow: /"
				<< blog_path
				<< std::endl
				;

			if (gen_sitemap) {

				ofs
					<< "Sitemap: sitemap.xml"
					<< std::endl
					;
			}
		}
	}

	void batch::generate_sitemap(boost::filesystem::path const& out, chrono_idx_t const& idx)
	{
		boost::filesystem::path p = out / "sitemap.xml";

		std::ofstream ofs(p.string(), std::ios::out | std::ios::trunc);
		if (!ofs.is_open())
		{
			std::cerr
				<< "***error: cannot open sitemap "
				<< p
				<< std::endl;

		}
		else {

			//
			//	ToDo: generate sitemap
			//
		}
	}

	batch::chrono_idx_t batch::get_sorted()
	{
		chrono_idx_t idx;
		for (auto const& doc : index_) {
			//cyng::param_map_t pm;
			//pm = cyng::value_cast(doc.second, pm);
			//std::cout 
			//	<< boost::filesystem::path(doc.first).filename()
			//	<< ": "
			//	<< cyng::io::to_str(doc.second)
			//	<< std::endl
			//	<< std::endl
			//	;
			auto const reader = cyng::make_reader(doc.second);
			auto const released = cyng::value_cast(reader.get("released"), std::chrono::system_clock::now());
			idx.emplace(released, doc.second);
		}
		return idx;
	}

}
