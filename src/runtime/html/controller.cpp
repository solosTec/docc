
#include "controller.h"
#include <docc/utils.h>
#include <docc/reader.h>
#include <rt/currency.h>

#include <cyng/task/controller.h>
#include <cyng/task/scheduler.h>
#include <cyng/vm/vm.h>
#include <cyng/vm/mesh.h>
#include <cyng/io/parser/parser.h>
#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>
#include <cyng/obj/algorithm/reader.hpp>
#include <cyng/obj/numeric_cast.hpp>
#include <cyng/obj/container_cast.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>

namespace docruntime {

	void show(std::string str) {
		std::cout << str << std::endl;
	}


	controller::controller(std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, std::filesystem::path const& tmp_asm
		, std::filesystem::path const& tmp_html
		, int verbose)
	: ofs_(out.string(), std::ios::trunc)
		, tmp_html_(tmp_html.string(), std::ios::trunc)
		, tmp_html_path_(tmp_html)
		, vars_()
		, meta_()
		, toc_()
		, uuid_gen_()
		, ctx_(docscript::verify_extension(tmp_asm, "docs"), inc, verbose)
		, assembler_(std::filesystem::path(ctx_.get_output_path()).replace_extension("cyng"), inc, verbose)
	{
		meta_.emplace("build", cyng::make_object(std::chrono::system_clock::now()));
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
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : cannot open output file\n");
			return EXIT_FAILURE;
		}

		auto const now = std::chrono::high_resolution_clock::now();


		auto const r = ctx_.lookup(inp);
		if (!r.second) return EXIT_FAILURE;

		docscript::reader compiler(ctx_);
		compiler.read(r.first);

		if (ctx_.get_verbosity(2)) {

			fmt::print(
				stdout,
				fg(fmt::color::forest_green),
				"***info : intermediate file {} complete\n", ctx_.get_output_path());
		}

		//
		//	generate program from assembler
		//
		assembler_.read(std::filesystem::path(ctx_.get_output_path()));

		if (ctx_.get_verbosity(2)) {

			fmt::print(
				stdout,
				fg(fmt::color::forest_green),
				"***info : program {} complete\n", assembler_.get_output_path().string());
		}

		//
		//	load program
		//
		std::ifstream ifs(assembler_.get_output_path().string(), std::ios::binary);
		if (ifs.is_open()) {
			ifs.unsetf(std::ios::skipws);

			cyng::buffer_t buffer;
			
			ifs.seekg(0, std::ios::end);
			buffer.reserve(ifs.tellg());
			ifs.seekg(0, std::ios::beg);


			auto pos = std::istream_iterator<char>(ifs);
			auto end = std::istream_iterator<char>();

			buffer.insert(buffer.begin(), pos, end);

			//
			//	Create an scheduler with specified size
			//	of the thread pool.
			//
			cyng::controller ctl(pool_size);
			cyng::mesh fabric(ctl);

			//
			//	Create VM
			//
			auto vm = fabric.make_proxy(tag
				, cyng::make_description("quote", f_quote())
				, cyng::make_description("set", f_set())
				, cyng::make_description("get", f_get())
				, cyng::make_description("meta", f_meta())
				, cyng::make_description(std::string("\xc2\xb6"), f_paragraph())
				, cyng::make_description("i", f_italic())
				, cyng::make_description("b", f_bold())
				, cyng::make_description("tt", f_typewriter())
				, cyng::make_description("label", f_label())
				, cyng::make_description("ref", f_ref())
				, cyng::make_description("h1", f_h1())
				, cyng::make_description("h2", f_h2())
				, cyng::make_description("h3", f_h3())
				, cyng::make_description("h4", f_h4())
				, cyng::make_description("h5", f_h5())
				, cyng::make_description("h6", f_h6())
				, cyng::make_description("header", f_header())
				, cyng::make_description("figure", f_figure())
				, cyng::make_description("resource", f_resource())
				, cyng::make_description("now", f_now())
				, cyng::make_description("uuid", f_uuid())
				, cyng::make_description("range", f_range())
				, cyng::make_description("cat", f_cat())
				, cyng::make_description("repeat", f_repeat())
				, cyng::make_description("currency", f_currency())
				, cyng::make_description("show", f_show())
			);

			cyng::deque_t deq;
			cyng::io::parser p([&](cyng::object&& obj) -> void {
				//std::cout << cyng::io::to_typed(obj) << std::endl;
				deq.push_back(std::move(obj));
				});
			p.read(std::begin(buffer), std::end(buffer));

			if (ctx_.get_verbosity(4)) {

				fmt::print(
					stdout,
					fg(fmt::color::forest_green),
					"***info : load program of {} bytes with {} instructions\n", buffer.size(), deq.size());
			}

			//
			//	execute program
			// 
			vm.load(std::move(deq));
			vm.run();

			//
			//	wait for pending requests
			//
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			vm.stop();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ctl.cancel();
			ctl.stop();
			//ctl.shutdown();

			tmp_html_.close();

			if (!generate_body_only) {
				ofs_ << "<html>" << std::endl;
				emit_header();
				ofs_ 
					<< "<body>" << std::endl
					;

				tmp_html_.open(tmp_html_path_.string(), std::ios::in | std::ios::binary);
				ofs_ << tmp_html_.rdbuf();
				tmp_html_.close();

				ofs_ 
					<< "</body>" << std::endl
					<< "</html>" << std::endl
					;
			}
			else {
				tmp_html_.open(tmp_html_path_.string(), std::ios::in | std::ios::binary);
				ofs_ << tmp_html_.rdbuf();
				tmp_html_.close();

			}

		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : input file [{}] not found\n", inp.string());
			return EXIT_FAILURE;
		}

		if (ctx_.get_verbosity(4)) {

			fmt::print(
				stdout,
				fg(fmt::color::forest_green),
				"***info : TOC:\n");

			std::cout << toc_ << std::endl;
		}

		//	JSON
		if (generate_index) {
			auto const vec = to_vector(toc_);
			std::cout << cyng::io::to_json_pretty(cyng::make_object(vec)) << std::endl;
		}

		auto const delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now);

		fmt::print(
			stdout,
			fg(fmt::color::forest_green),
			"***info : complete after {} milliseconds\n", delta.count());

		return EXIT_SUCCESS;
	}

	std::string controller::quote(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		//
		//	dependend from language (see meta data)
		//	https://dbaron.org/www/quotes
		//
		ss << "&bdquo;";
		bool init = false;
		to_html(ss, vec);
		ss << "&ldquo;";
		return ss.str();
	}

	std::string controller::italic(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "<span style=\"font-style: italic;\">";
		to_html(ss, vec);
		ss << "</span>";
		return ss.str();
	}

	std::string controller::bold(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "<span style=\"font-weight: bold;\">";
		to_html(ss, vec);
		ss << "</span>";
		return ss.str();
	}

	std::string controller::typewriter(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "<span style=\"font-family: monospace;\">";
		to_html(ss, vec);
		ss << "</span>";
		return ss.str();
	}

	void controller::set(cyng::param_map_t pm) {
		//std::cout << "SET(" << pm << ")" << std::endl;
		vars_.insert(pm.begin(), pm.end());
	}
	void controller::meta(cyng::param_map_t pm) {
		std::cout << "META(" << pm << ")" << std::endl;
		meta_.insert(pm.begin(), pm.end());
	}

	cyng::vector_t controller::get(cyng::vector_t vec) {
		cyng::vector_t res;
		for (auto const& v : vec) {
			auto const pos = vars_.find(cyng::io::to_plain(v));
			if (pos != vars_.end()) {
				res.push_back(pos->second);
			}
		}
		return res;
	}

	std::string controller::paragraph(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		//std::stringstream ss;
		//ss << "PARAGRAPH*" << vec.size() << "(";
		tmp_html_ << "<p>";
		to_html(tmp_html_, vec);
		tmp_html_ << "</p>" << std::endl;
		//std::cout << ss.str() << std::endl;
		return "";
	}

	void controller::label(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "LABEL(" << vec << ")";
		std::cout << ss.str() << std::endl;
	}

	std::string controller::ref(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "REF(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	std::string controller::h1(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		tmp_html_ << "<h1>";
		to_html(tmp_html_, vec);
		tmp_html_ << "</h1>" << std::endl;
		//std::cout << ss.str() << std::endl;
		toc_.add(0, uuid_gen_(), cyng::to_string(vec));
		return "";
	}
	std::string controller::h2(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		tmp_html_ << "<h2>";
		to_html(tmp_html_, vec);
		tmp_html_ << "</h2>" << std::endl;
		//std::cout << ss.str() << std::endl;
		toc_.add(1, uuid_gen_(), cyng::to_string(vec));
		return "";
	}
	std::string controller::h3(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		tmp_html_ << "<h3>";
		to_html(tmp_html_, vec);
		tmp_html_ << "</h3>" << std::endl;
		toc_.add(2, uuid_gen_(), cyng::to_string(vec));
		return "";
	}
	std::string controller::h4(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		tmp_html_ << "<h4>";
		to_html(tmp_html_, vec);
		tmp_html_ << "</h4>" << std::endl;
		toc_.add(3, uuid_gen_(), cyng::to_string(vec));
		return "";
	}
	std::string controller::h5(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		tmp_html_ << "<h5>";
		to_html(tmp_html_, vec);
		tmp_html_ << "</h5>" << std::endl;
		toc_.add(4, uuid_gen_(), cyng::to_string(vec));
		return "";
	}
	std::string controller::h6(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		tmp_html_ << "<h6>";
		to_html(tmp_html_, vec);
		tmp_html_ << "</h6>" << std::endl;
		toc_.add(5, uuid_gen_(), cyng::to_string(vec));
		return "";
	}
	std::string controller::header(cyng::param_map_t pm) {
		std::stringstream ss;
		ss << "HEADER(" << pm << ")";
		std::cout << ss.str() << std::endl;

		//	"level":0000000000000001),("tag":<uuid>'79bf3ba0-2362-4ea5-bcb5-ed93844ac59a'),("title":[Basics]))
		auto const reader = cyng::make_reader(pm);
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0);
		auto const tag = cyng::value_cast(reader.get("tag"), uuid_gen_());
		auto const title = cyng::io::to_plain(reader.get("title"));

		toc_.add(level, tag, title);
		return ss.str();
	}

	std::string controller::figure(cyng::param_map_t pm) {
		std::stringstream ss;
		ss << "FIGURE(" << pm << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}

	void controller::resource(cyng::param_map_t pm) {
		std::cout << "RESOURCE(" << pm << ")" << std::endl;
	}
	std::chrono::system_clock::time_point controller::now(cyng::param_map_t pm) {
		//std::stringstream ss;
		//std::cout << "NOW(" << pm << ")" << std::endl;
		return std::chrono::system_clock::now();
	}
	boost::uuids::uuid controller::uuid(cyng::param_map_t) {
		return uuid_gen_();
	}

	cyng::vector_t controller::range(cyng::vector_t vec) {
		return vec;
	}
	std::string controller::cat(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		//ss << "CAT*" << vec.size() << "(";
		for (auto const& v : vec) {
			ss << v;
		}
		//ss << ")";
		//std::cout << ss.str() << std::endl;
		return ss.str();
	}

	//		insert_method(table, method("repeat", parameter_type::MAP, true, { "count", "value", "sep"}));
	std::string controller::repeat(cyng::param_map_t pm) {
		return "";
	}

	std::string controller::currency(cyng::param_map_t pm) {
		//	https://www.fileformat.info/info/unicode/category/Sc/list.htm
		//	https://www.w3schools.com/charsets/ref_utf_currency.asp
		//return std::string("\xE2\x82\xB9");	//	indian rupee

		auto const reader = cyng::make_reader(pm);
		auto const value = cyng::numeric_cast<std::size_t>(reader.get("value"), 0);
		auto const name = cyng::value_cast<std::string>(reader.get("name"), "euro");

		return docruntime::currency_html(value, name);
	}

	void controller::emit_header() {
		ofs_ << "<head>" << std::endl;
		ofs_ << "\t<meta charset = \"utf-8\"/> " << std::endl;
		ofs_ << "\t<meta name=\"viewport\"content=\"width=device-width, initial-scale=1\"/>" << std::endl;

		//
		//	title first
		//
		auto const pos = meta_.find("title");
		if (pos != meta_.end()) {
			ofs_ << "\t<title>" << pos->second << "</title>" << std::endl;
		}

		for (auto const& param : meta_) {
			if (boost::algorithm::equals(param.first, "title")) {
				ofs_ << "\t<meta name=\"og:title\" content=\"" << pos->second << "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "description")) {
				ofs_ << "\t<meta name=\"og:description\" content=\"" << param.second << "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "type")) {
				ofs_ << "\t<meta name=\"og:type\" content=\"" << param.second << "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "url")) {
				ofs_ << "\t<meta name=\"og:url\" content=\"" << param.second << "\" />" << std::endl;
			}
			else if (boost::algorithm::equals(param.first, "site_name")) {
				ofs_ << "\t<meta name=\"og:site_name\" content=\"" << param.second << "\" />" << std::endl;
			}
			else {
				ofs_ << "\t<meta name=\"" << param.first << "\" content=\"" << param.second << "\" />" << std::endl;
			}
		}
		emit_styles(1, ofs_);
		ofs_ << "</head>" << std::endl;
	}

	std::function<std::string(cyng::vector_t)> controller::f_quote() {
		return std::bind(&controller::quote, this, std::placeholders::_1);
	}

	std::function<void(cyng::param_map_t)> controller::f_set() {
		return std::bind(&controller::set, this, std::placeholders::_1);
	}
	std::function<cyng::vector_t(cyng::vector_t)> controller::f_get() {
		return std::bind(&controller::get, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> controller::f_meta() {
		return std::bind(&controller::meta, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::vector_t)> controller::f_paragraph() {
		return std::bind(&controller::paragraph, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_italic() {
		return std::bind(&controller::italic, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_bold() {
		return std::bind(&controller::bold, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_typewriter() {
		return std::bind(&controller::typewriter, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> controller::f_label() {
		return std::bind(&controller::label, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_ref() {
		return std::bind(&controller::ref, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h1() {
		return std::bind(&controller::h1, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h2() {
		return std::bind(&controller::h2, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h3() {
		return std::bind(&controller::h3, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h4() {
		return std::bind(&controller::h4, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h5() {
		return std::bind(&controller::h5, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h6() {
		return std::bind(&controller::h6, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::param_map_t)> controller::f_header() {
		return std::bind(&controller::header, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::param_map_t)> controller::f_figure() {
		return std::bind(&controller::figure, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> controller::f_resource() {
		return std::bind(&controller::resource, this, std::placeholders::_1);
	}
	std::function<std::chrono::system_clock::time_point(cyng::param_map_t)> controller::f_now() {
		return std::bind(&controller::now, this, std::placeholders::_1);
	}
	std::function<boost::uuids::uuid(cyng::param_map_t)> controller::f_uuid() {
		return std::bind(&controller::uuid, this, std::placeholders::_1);
	}
	std::function<cyng::vector_t(cyng::vector_t)> controller::f_range() {
		return std::bind(&controller::range, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_cat() {
		return std::bind(&controller::cat, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::param_map_t pm)> controller::f_repeat() {
		return std::bind(&controller::repeat, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::param_map_t)> controller::f_currency() {
		return std::bind(&controller::currency, this, std::placeholders::_1);
	}

	std::function<void(std::string)> controller::f_show() {
		return std::bind(&show, std::placeholders::_1);
	}

	std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext)
	{
		if (!p.has_extension())
		{
			p.replace_extension(ext);
		}
		return p;
	}

	void to_html(std::ostream& os, cyng::vector_t const& vec) {
		bool init = false;
		for (auto const& obj : vec) {
			if (init) {
				os << ' ';
			}
			else {
				init = true;
			}

			switch (obj.rtti().tag()) {
			case cyng::TC_VECTOR:
				to_html(os, cyng::container_cast<cyng::vector_t>(obj));
				break;
			default:
				os << obj;
				break;
			}
		}

	}

	void emit_styles(std::size_t depth, std::ostream& ofs)
	{
		
		ofs
			<< std::string(depth, '\t') << "<style>"	<< std::endl
			<< std::string(depth + 1, '\t') << "body { " << std::endl
			//	Georgia,Cambria,serif;
			<< std::string(depth + 2, '\t') << "font-family:'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;" << std::endl
			//	https://jrl.ninja/etc/1/
			//<< "\t\t\tmax-width: 52rem; "
			<< std::string(depth + 2, '\t') << "max-width: 62%; " << std::endl
			<< std::string(depth + 2, '\t') << "padding: 2rem; " << std::endl
			<< std::string(depth + 2, '\t') << "margin: auto; "	<< std::endl
			<< std::string(depth + 2, '\t') << "font-size: 1.1em; " << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//
			//	styling header h1 and h2
			//
			<< std::string(depth + 1, '\t') << "h1, h2 {" << std::endl
			<< std::string(depth + 2, '\t') << "padding-bottom: .3em;" << std::endl
			<< std::string(depth + 2, '\t') << "border-bottom: 1px solid #eaecef;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "code, kbd, pre, samp {"
			<< std::endl
			<< std::string(depth + 1, '\t') << "\tfont-family: monospace;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//oction

			<< std::string(depth + 1, '\t') << "a.oction {"	<< std::endl
			<< std::string(depth + 2, '\t') << "opacity: 0;" << std::endl
			<< std::string(depth + 2, '\t') << "transition: opacity 0.2s;" << std::endl
			<< std::string(depth + 2, '\t') << "cursor: pointer;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "a.oction:hover{" << std::endl
			<< std::string(depth + 2, '\t') << "opacity: 1;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "p > a {" << std::endl
			<< std::string(depth + 2, '\t') << "text-decoration: none;" << std::endl
			<< std::string(depth + 2, '\t') << "color: blue;" << std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "blockquote > p { margin-bottom: 1px; }"
			<< std::endl
			<< std::string(depth + 1, '\t') << "pre { background-color: #fafafa; }"
			<< std::endl

			<< std::string(depth + 1, '\t') << "pre > code:hover {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: orange;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "blockquote {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-left: 4px solid #eee;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding-left: 10px;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "color: #777;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "margin: 16px 20px;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//
			//	figure
			//
			<< std::string(depth + 1, '\t') << "figure {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "margin: 2%;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl
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
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl
			<< std::string(depth + 1, '\t') << "img:hover {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "box-shadow: 0 0 10px #ccc;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//
			//	gallery
			//
			<< std::string(depth + 1, '\t') << "div.gallery {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "display: grid;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "grid-gap: 12px;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: #eee;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "div.smf-svg:hover {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "box-shadow: 0 0 10px #ccc;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//
			//	gallery
			//
			<< std::string(depth + 1, '\t') << "div.gallery img {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "width: 100%;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "height: auto;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "object-fit: cover;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//
			//	definition lists with flexbox
			//
			<< std::string(depth + 1, '\t') << "dl {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "display: flex;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "flex-flow: row wrap;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "dt {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "font-weight: bold;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "flex-basis: 20% ;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "dt::after {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "content: \":\";"
			<< std::string(depth + 1, '\t') << "}" << std::endl
			<< std::string(depth + 1, '\t') << "dd {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "flex-basis: 70%;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "flex-grow: 1;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//
			//	alertbox with flexgrid
			//
			<< std::string(depth + 1, '\t') << "ul.alert {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "display: flex;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "list-style: none;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "ul.alert > li {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding: 7px;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "ul.alert > li:nth-child(2) {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: #ddd;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-radius: 5px;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			//	formatting sub and supscript
			<< std::string(depth + 1, '\t') << "sup { top: -.5em; }"
			<< std::endl
			<< std::string(depth + 1, '\t') << "sub { bottom: -.25em; }"
			<< std::endl
			<< std::string(depth + 1, '\t') << "sub, sup {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "font-size: 75%;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "line-height: 0;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "position: relative;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "vertical-align: baseline;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl

			//	There is an alternative styling for aside tag that works
			//	for bootstrap 4 too: float: right;
			<< std::string(depth + 1, '\t') << "aside {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "position: absolute;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "right: 2em;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "width: 20%;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border: 1px #D5DBDB solid;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "color: #9C640C;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding: 0.5em;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "z-index: -1;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "table {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-collapse: collapse;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-spacing: 0px;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}"
			<< std::endl
			<< std::string(depth + 1, '\t') << "table, th, td {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding: 5px;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border: 1px solid black;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "tr:nth-child(even) {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: #f2f2f2;"
			<< std::endl
			<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "caption {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "font-weight: bold;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "color: white;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: DimGray;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding: 5px;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "text-align: left;"
			<< std::endl
				<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth + 1, '\t') << "kbd {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding: 2px 4px;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "font-size: 90%;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "color: #ffffff;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "background-color: #333333;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "border-radius: 3px;"
			<< std::endl
				<< std::string(depth + 1, '\t') << "}" << std::endl

			//
			//	ToC
			//
			<< std::string(depth + 1, '\t') << "details > ul * {"
			<< std::endl
			<< std::string(depth + 2, '\t') << "list-style-type: none;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "text-decoration: none;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "margin-left: 0em;"
			<< std::endl
			<< std::string(depth + 2, '\t') << "padding-left: 0.7em;"
			<< std::endl
				<< std::string(depth + 1, '\t') << "}" << std::endl

			<< std::string(depth, '\t') << "</style>" << std::endl

			;
	}
}
