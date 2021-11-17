
#include "controller.h"
#include "generator.h"

#include <docc/reader.h>
#include <docc/utils.h>
#include <html/formatting.h>
#include <rt/currency.h>

#include <cyng/io/ostream.h>
//#include <cyng/io/parser/parser.h>
#include <cyng/io/serialize.h>
//#include <cyng/obj/algorithm/reader.hpp>
#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/numeric_cast.hpp>
#include <cyng/task/controller.h>
#include <cyng/task/scheduler.h>
#include <cyng/vm/mesh.h>
#include <cyng/vm/vm.h>

#include <fmt/color.h>
#include <fmt/core.h>

#include <fstream>
#include <functional>
#include <chrono>
#include <iostream>


namespace docruntime {


	controller::controller(std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, std::filesystem::path const& tmp_asm
		, std::filesystem::path const& tmp_html, int verbose)
	: ofs_(out.string(), std::ios::trunc)
		, tmp_html_(tmp_html.string(), std::ios::trunc)
		, tmp_html_path_(tmp_html)
		, ctx_(docscript::verify_extension(tmp_asm, "docs"), inc, verbose)
		, assembler_(std::filesystem::path(ctx_.get_output_path()).replace_extension("cyng")
			, inc
			, verbose) 
	{
	}

	int controller::run(std::filesystem::path&& inp
		, std::size_t pool_size
		, boost::uuids::uuid tag
		, bool generate_body_only
		, bool generate_meta
		, bool generate_index
		, std::string type) {

		//
		//	check output file
		//
		if (!ofs_.is_open() || !tmp_html_.is_open()) {
			fmt::print(stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : cannot open output file\n");
			return EXIT_FAILURE;
		}

		auto const now = std::chrono::high_resolution_clock::now();

		//
		//	check input file
		//
		auto const r = ctx_.lookup(inp);
		if (!r.second)
			return EXIT_FAILURE;

		//
		//	start compiler and generate an assembler file
		//
		docscript::reader compiler(ctx_);
		compiler.read(r.first);

		if (ctx_.get_verbosity(2)) {

			fmt::print(stdout, fg(fmt::color::forest_green),
				"***info : intermediate file {} complete\n",
				ctx_.get_output_path());
		}

		//
		//	generate program from assembler
		//
		assembler_.read(std::filesystem::path(ctx_.get_output_path()));

		if (ctx_.get_verbosity(2)) {

			fmt::print(stdout, fg(fmt::color::forest_green),
				"***info : program {} complete\n",
				assembler_.get_output_path().string());
		}

		//
		//	load and execute program
		//
		std::ifstream ifs(assembler_.get_output_path().string(),
			std::ios::binary);
		if (ifs.is_open()) {
			ifs.unsetf(std::ios::skipws);

			//
			//	Create an scheduler with specified size
			//	of the thread pool.
			//
			cyng::controller ctl(pool_size);
			cyng::mesh fabric(ctl);

			generator gen(ifs, tmp_html_, fabric, tag, ctx_);
			gen.run();

			//
			//	wait for pending requests
			//
			//std::this_thread::sleep_for(std::chrono::milliseconds(10));
			//vm.stop();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ctl.cancel();
			ctl.stop();
			// ctl.shutdown();

			tmp_html_.close();

			if (!generate_body_only) {
				ofs_ << "<html>" << std::endl;
				
				emit_header(gen.get_meta());
				ofs_ << "<body>" << std::endl;

				tmp_html_.open(tmp_html_path_.string(),
					std::ios::in | std::ios::binary);
				ofs_ << tmp_html_.rdbuf();
				tmp_html_.close();

				ofs_ << "</body>" << std::endl << "</html>" << std::endl;
			}
			else {
				tmp_html_.open(tmp_html_path_.string(),
					std::ios::in | std::ios::binary);
				ofs_ << tmp_html_.rdbuf();
				tmp_html_.close();
			}

			if (ctx_.get_verbosity(4)) {

				fmt::print(stdout, fg(fmt::color::forest_green),
					"***info : TOC:\n");

				std::cout << gen.get_toc() << std::endl;
			}

			//	JSON
			if (generate_index) {
				auto const vec = to_vector(gen.get_toc());
				std::cout << cyng::io::to_json_pretty(cyng::make_object(vec))
					<< std::endl;
			}

		}
		else {
			fmt::print(stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : input file [{}] not found\n", inp.string());
			return EXIT_FAILURE;
		}


		auto const delta =
			std::chrono::duration_cast<std::chrono::milliseconds>(
				std::chrono::high_resolution_clock::now() - now);

		fmt::print(stdout, fg(fmt::color::forest_green),
			"***info : complete after {} milliseconds\n", delta.count());

		return EXIT_SUCCESS;
	}


	void controller::emit_header(cyng::param_map_t& meta) {
		ofs_ << "<head>" << std::endl;
		ofs_ << "\t<meta charset = \"utf-8\"/> " << std::endl;
		ofs_ << "\t<meta name=\"viewport\"content=\"width=device-width, "
			"initial-scale=1\"/>"
			<< std::endl;

		//
		//	title first
		//
		auto const pos = meta.find("title");
		if (pos != meta.end()) {
			ofs_ << "\t<title>" << pos->second << "</title>" << std::endl;
		}

		for (auto const& param : meta) {
			if (boost::algorithm::equals(param.first, "title")) {
				ofs_ << "\t<meta name=\"og:title\" content=\"" << pos->second
					<< "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "description")) {
				ofs_ << "\t<meta name=\"og:description\" content=\""
					<< param.second << "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "type")) {
				ofs_ << "\t<meta name=\"og:type\" content=\"" << param.second
					<< "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "url")) {
				ofs_ << "\t<meta name=\"og:url\" content=\"" << param.second
					<< "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "site_name")) {
				ofs_ << "\t<meta name=\"og:site_name\" content=\""
					<< param.second << "\" />" << std::endl;
			}
			else {
				ofs_ << "\t<meta name=\"" << param.first << "\" content=\""
					<< param.second << "\" />" << std::endl;
			}
		}
		emit_styles(1, ofs_);
		ofs_ << "</head>" << std::endl;
	}

	std::filesystem::path verify_extension(std::filesystem::path p,
		std::string const& ext) {
		if (!p.has_extension()) {
			p.replace_extension(ext);
		}
		return p;
	}

	void emit_styles(std::size_t depth, std::ostream& ofs) {

		ofs << std::string(depth, '\t') << "<style>" << std::endl
			<< std::string(depth + 1, '\t') << "body { "
			<< std::endl
			//	Georgia,Cambria,serif;
			<< std::string(depth + 2, '\t')
			<< "font-family:'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"
			<< std::endl
			//	https://jrl.ninja/etc/1/
			//<< "\t\t\tmax-width: 52rem; "
			<< std::string(depth + 2, '\t') << "max-width: 62%; " << std::endl
			<< std::string(depth + 2, '\t') << "padding: 2rem; " << std::endl
			<< std::string(depth + 2, '\t') << "margin: auto; " << std::endl
			<< std::string(depth + 2, '\t') << "font-size: 1.1em; " << std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//
			//	styling header h1 and h2
			//
			<< std::string(depth + 1, '\t') << "h1, h2 {" << std::endl
			<< std::string(depth + 2, '\t') << "padding-bottom: .3em;"
			<< std::endl
			<< std::string(depth + 2, '\t')
			<< "border-bottom: 1px solid #eaecef;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "code, kbd, pre, samp {"
			<< std::endl
			<< std::string(depth + 1, '\t') << "\tfont-family: monospace;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			// oction

			<< std::string(depth + 1, '\t') << "a.oction {" << std::endl
			<< std::string(depth + 2, '\t') << "margin-left: 6px;" << std::endl
			<< std::string(depth + 2, '\t') << "opacity: 0;" << std::endl
			<< std::string(depth + 2, '\t') << "transition: opacity 0.2s;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "cursor: pointer;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "a.oction:hover{" << std::endl
			<< std::string(depth + 2, '\t') << "opacity: 1;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "p > a {" << std::endl
			<< std::string(depth + 2, '\t') << "text-decoration: none;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "color: blue;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t')
			<< "blockquote > p { margin-bottom: 1px; }" << std::endl
			<< std::string(depth + 1, '\t')
			<< "pre { background-color: #fafafa; }" << std::endl

			<< std::string(depth + 1, '\t') << "pre > code:hover {" << std::endl
			<< std::string(depth + 2, '\t') << "background-color: orange;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "blockquote {" << std::endl
			<< std::string(depth + 2, '\t') << "border-left: 4px solid #eee;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding-left: 10px;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "color: #777;" << std::endl
			<< std::string(depth + 2, '\t') << "margin: 16px 20px;" << std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//
			//	figure
			//
			<< std::string(depth + 1, '\t') << "figure {" << std::endl
			<< std::string(depth + 2, '\t') << "margin: 2%;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "figure > figcaption {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: #ddd;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "font-style: italic;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "img {"
			<< std::endl
			//<< std::string(depth + 1, '\t') << "\tmax-width: 95%;"
			//<< std::endl
			<< std::string(depth + 2, '\t') << "border: 2px solid #777;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "img:hover {" << std::endl
			<< std::string(depth + 2, '\t') << "box-shadow: 0 0 10px #ccc;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//
			//	gallery
			//
			<< std::string(depth + 1, '\t') << "div.gallery {" << std::endl
			<< std::string(depth + 2, '\t') << "display: grid;" << std::endl
			<< std::string(depth + 2, '\t') << "grid-gap: 12px;" << std::endl
			<< std::string(depth + 2, '\t') << "background-color: #eee;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "div.smf-svg:hover {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "box-shadow: 0 0 10px #ccc;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//
			//	gallery
			//
			<< std::string(depth + 1, '\t') << "div.gallery img {" << std::endl
			<< std::string(depth + 2, '\t') << "width: 100%;" << std::endl
			<< std::string(depth + 2, '\t') << "height: auto;" << std::endl
			<< std::string(depth + 2, '\t') << "object-fit: cover;" << std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//
			//	definition lists with flexbox
			//
			<< std::string(depth + 1, '\t') << "dl {" << std::endl
			<< std::string(depth + 2, '\t') << "display: flex;" << std::endl
			<< std::string(depth + 2, '\t') << "flex-flow: row wrap;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "dt {" << std::endl
			<< std::string(depth + 2, '\t') << "font-weight: bold;" << std::endl
			<< std::string(depth + 2, '\t') << "flex-basis: 20% ;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "dt::after {" << std::endl
			<< std::string(depth + 2, '\t') << "content: \":\";"
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "dd {" << std::endl
			<< std::string(depth + 2, '\t') << "flex-basis: 70%;" << std::endl
			<< std::string(depth + 2, '\t') << "flex-grow: 1;" << std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//
			//	alertbox with flexgrid
			//
			<< std::string(depth + 1, '\t') << "ul.alert {" << std::endl
			<< std::string(depth + 2, '\t') << "display: flex;" << std::endl
			<< std::string(depth + 2, '\t') << "list-style: none;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "ul.alert > li {" << std::endl
			<< std::string(depth + 2, '\t') << "padding: 7px;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "ul.alert > li:nth-child(2) {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: #ddd;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-radius: 5px;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//	formatting sub and supscript
			<< std::string(depth + 1, '\t') << "sup { top: -.5em; }"
			<< std::endl
			<< std::string(depth + 1, '\t') << "sub { bottom: -.25em; }"
			<< std::endl
			<< std::string(depth + 1, '\t') << "sub, sup {" << std::endl
			<< std::string(depth + 2, '\t') << "font-size: 75%;" << std::endl
			<< std::string(depth + 2, '\t') << "line-height: 0;" << std::endl
			<< std::string(depth + 2, '\t') << "position: relative;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "vertical-align: baseline;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//	There is an alternative styling for aside tag that works
			//	for bootstrap 4 too: float: right;
			<< std::string(depth + 1, '\t') << "aside {" << std::endl
			<< std::string(depth + 2, '\t') << "position: absolute;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "right: 2em;" << std::endl
			<< std::string(depth + 2, '\t') << "width: 20%;" << std::endl
			<< std::string(depth + 2, '\t') << "border: 1px #D5DBDB solid;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "color: #9C640C;" << std::endl
			<< std::string(depth + 2, '\t') << "padding: 0.5em;" << std::endl
			<< std::string(depth + 2, '\t') << "z-index: -1;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "table {" << std::endl
			<< std::string(depth + 2, '\t') << "border-collapse: collapse;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-spacing: 0px;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "table, th, td {" << std::endl
			<< std::string(depth + 2, '\t') << "padding: 5px;" << std::endl
			<< std::string(depth + 2, '\t') << "border: 1px solid black;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "tr:nth-child(even) {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: #f2f2f2;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "caption {" << std::endl
			<< std::string(depth + 2, '\t') << "font-weight: bold;" << std::endl
			<< std::string(depth + 2, '\t') << "color: white;" << std::endl
			<< std::string(depth + 2, '\t') << "background-color: DimGray;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding: 5px;" << std::endl
			<< std::string(depth + 2, '\t') << "text-align: left;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "kbd {" << std::endl
			<< std::string(depth + 2, '\t') << "padding: 2px 4px;" << std::endl
			<< std::string(depth + 2, '\t') << "font-size: 90%;" << std::endl
			<< std::string(depth + 2, '\t') << "color: #ffffff;" << std::endl
			<< std::string(depth + 2, '\t') << "background-color: #333333;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-radius: 3px;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//
			//	ToC
			//
			<< std::string(depth + 1, '\t') << "details > ul * {" << std::endl
			<< std::string(depth + 2, '\t') << "list-style-type: none;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "text-decoration: none;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "margin-left: 0em;" << std::endl
			<< std::string(depth + 2, '\t') << "padding-left: 0.7em;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//	math class
			<< std::string(depth + 2, '\t') << ".math {"
			<< std::string(depth + 2, '\t')
			<< "font-family: 'Latin Modern Math', Palatino, 'Asana Math';"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth, '\t') << "</style>" << std::endl;
	}
} // namespace docruntime
