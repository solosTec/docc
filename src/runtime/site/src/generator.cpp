
#include <generator.h>

#include <rt/currency.h>
#include <rt/stream.h>
//#include <rt/i18n.h>

#include <html/formatting.h>
#include <html/dom.hpp>
#include <html/code.h>
#include <html/tree.h>
#include <html/table.h>

#include <cyng/vm/vm.h>
#include <cyng/io/parser/parser.h>
#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>
#include <cyng/obj/algorithm/reader.hpp>
#include <cyng/obj/numeric_cast.hpp>
#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/container_factory.hpp>
#include <cyng/xml/node.h>
#include <cyng/parse/json.h>

#include <smfsec/hash/base64.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>

#include <boost/uuid/uuid_io.hpp>

namespace docscript {

	generator::generator(std::istream& is
		, std::ostream& os
		, docscript::context& ctx
		, cyng::mesh& fabric
		, boost::uuids::uuid tag
		, std::string const& name
		, page const& cfg_page
		, navbar const& cfg_navbar
		, footer const& cfg_footer
		, std::string const& locale
		, std::string const& country
		, std::string const& language
		, std::string const& encoding)
	: is_(is)
		, os_(os)
		//, index_file_(index_file)
		, vars_()
		, meta_(cyng::param_map_factory("build", std::chrono::system_clock::now())
			("title", cfg_page.title_)
			("locale", locale)
			("country", country)
			("language", language)
			("encoding", encoding))
		, toc_()
		//, footnotes_()
		//, figures_()
		//, tables_()
		, uuid_gen_()
		, name_gen_(tag)
		, vm_(fabric.make_proxy(tag
			, cyng::make_description("esc", f_esc())
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
			, cyng::make_description("toc", f_toc())
			, cyng::make_description("resource", f_resource())
			, cyng::make_description("now", f_now())
			, cyng::make_description("uuid", f_uuid())
			, cyng::make_description("range", f_range())
			, cyng::make_description("fuse", f_fuse())
			, cyng::make_description("cat", f_cat())
			, cyng::make_description("repeat", f_repeat())
			, cyng::make_description("currency", f_currency())
			//, cyng::make_description("show", f_show())
			, cyng::make_description("code", f_code())
			, cyng::make_description("tree", f_tree())
			, cyng::make_description("table", f_table())
		))
		, ctx_(ctx)
		, page_name_(name)
		, cfg_page_(cfg_page)
		, cfg_navbar_(cfg_navbar)
		, cfg_footer_(cfg_footer)
	{}

	cyng::param_map_t& generator::get_vars() {
		return vars_;
	}
	cyng::param_map_t& generator::get_meta() {
		return meta_;
	}
	//docruntime::toc& generator::get_toc() {
	//	return toc_;
	//}

	int generator::run() {

		//
		//	navbar
		// 
		emit_navbar(os_, cfg_navbar_, cfg_page_);

		//
		//	xlink svgs
		//
		emit_svg(os_);

		//
		//	load program into buffer
		//
		auto [buffer, length] = docruntime::stream_to_buffer(is_);

		cyng::deque_t deq;
		cyng::io::parser p([&](cyng::object&& obj) -> void {
#ifdef _DEBUG
			//std::cout << "load: " << cyng::io::to_typed(obj) << std::endl;
#endif
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
		vm_.load(std::move(deq));
		vm_.run();

		//
		//	wait for pending requests
		//
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		vm_.stop();

		//
		//	footer
		// 
		emit_footer(os_, cfg_footer_);

		return EXIT_SUCCESS;
	}

	std::string generator::quote(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		//
		//	dependend from language (see meta data)
		//	https://dbaron.org/www/quotes
		//
		ss << "&bdquo;";
		dom::to_html(ss, vec, " ");
		ss << "&ldquo;";
		return ss.str();
	}

	std::string generator::esc(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		BOOST_ASSERT_MSG(vec.size() == 1, "single string expected");
		//
		//	only strings allowed
		//
		auto const s = dom::to_html(vec, " ");
		//	slow
		std::stringstream ss;
		dom::esc_html(ss, s);
		return ss.str();
	}

	std::string generator::italic(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "<span style=\"font-style: italic;\">";
		dom::to_html(ss, vec, " ");
		ss << "</span>";
		return ss.str();
	}

	std::string generator::bold(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "<span style=\"font-weight: bold;\">";
		dom::to_html(ss, vec, " ");
		ss << "</span>";
		return ss.str();
	}

	std::string generator::typewriter(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "<span style=\"font-family: monospace;\">";
		dom::to_html(ss, vec, " ");
		ss << "</span>";
		return ss.str();
	}

	void generator::set(cyng::param_map_t pm) {
		//std::cout << "SET(" << pm << ")" << std::endl;
		vars_.insert(pm.begin(), pm.end());
	}
	void generator::meta(cyng::param_map_t pm) {
		//std::cout << "META(" << pm << ")" << std::endl;
		//meta_.insert(pm.begin(), pm.end());
		for (auto const& v : pm) {
			meta_.insert_or_assign(v.first, v.second);
		}
	}

	cyng::vector_t generator::get(cyng::vector_t vec) {
		cyng::vector_t res;
		for (auto const& v : vec) {
			auto const pos = vars_.find(cyng::io::to_plain(v));
			if (pos != vars_.end()) {
				res.push_back(pos->second);
			}
		}
		return res;
	}

	void generator::paragraph(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		if (!vec.empty()) {
			os_ << "<p>";
			dom::to_html(os_, vec, " ");
			os_ << "</p>" << std::endl;
		}
	}

	void generator::label(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		//
		//	write an anchor
		//	generate a UUID from the text and use this
		//	as id.
		//

		std::stringstream ss;
		dom::to_html(ss, vec, ":");
		auto const name = ss.str();	//	label name

		auto const id = name_gen_(name);

		os_ 
			<< "<a id=\""
			<< boost::uuids::to_string(id)
			<< "\" style = \"display:inline;\" href=\""
			<< name
			<< "\"></a>";

		//
		//	update reference list
		// 
		//if (!refs_.emplace(name, id).second) {
		//	//
		//	//	duplicate label name
		//	//
		//	fmt::print(
		//		fg(fmt::color::dark_orange) | fmt::emphasis::bold,
		//		"***error: duplicate label [{}]\n", name);

		//}
	}

	std::string generator::ref(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		dom::to_html(ss, vec, ":");
		auto const name = ss.str();	//	label name
		auto const id = name_gen_(name);

		return name;
	}
	void generator::h1(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const id = name_gen_(title);
		auto const r = toc_.add(0, id, title);
		emit_header(0, id, r.first, title);
	}
	void generator::h2(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const id = name_gen_(title);
		auto const r = toc_.add(1, id, title);
		emit_header(1, id, r.first, title);
	}
	void generator::h3(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const id = name_gen_(title);
		auto const r = toc_.add(2, id, title);
		emit_header(2, id, r.first, title);
	}
	void generator::h4(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const id = name_gen_(title);
		auto const r = toc_.add(3, id, title);
		emit_header(3, id, r.first, title);
	}
	void generator::h5(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const id = name_gen_(title);
		auto const r = toc_.add(4, id, title);
		emit_header(4, id, r.first, title);
	}
	void generator::h6(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const id = name_gen_(title);
		auto const r = toc_.add(5, id, title);
		emit_header(5, id, r.first, title);
	}
	void generator::header(cyng::param_map_t pm) {
		//	"level":0000000000000001),("tag":<uuid>'79bf3ba0-2362-4ea5-bcb5-ed93844ac59a'),("title":[Basics]))
		auto const reader = cyng::make_reader(pm);
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0);
		auto vec = cyng::container_cast<cyng::vector_t>(reader.get("title"));
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const tag = cyng::value_cast(reader.get("tag"), name_gen_(title));
		auto const r = toc_.add(level - 1, tag, title);
		emit_header(level - 1, tag, r.first, title);
	}

	void generator::emit_header(std::size_t level, boost::uuids::uuid tag, std::string const& num, std::string const& title) {

		os_
			<< "<h"
			<< level + 1
			<< ">"
			<< num
			<< "&nbsp;"
			<< title
			<< "<a id=\""
			<< tag
			<< "\" aria-hidden=\"true\" "
			<< "href=\"#" << tag << "\">"
			//<< "class=\"oction\">"
			<< "<svg width=\"16\" height=\"16\"><use xlink:href=\"#home\" /></svg>"
			<< "</a>"
			<< "</h"
			<< level + 1
			<< ">"
			<< std::endl
			;
	}

	void generator::figure(cyng::param_map_t pm) {
		//std::cout << "FIGURE(" << pm << ")" << std::endl;

		auto const reader = cyng::make_reader(pm);
		//	no HTML code allowed
		std::filesystem::path const source = cyng::value_cast(reader.get("source"), "");
		auto const tag = cyng::value_cast(reader.get("tag"), name_gen_(source.string()));
		auto const caption = cyng::value_cast(reader.get("caption"), boost::uuids::to_string(tag));
		auto const alt = cyng::value_cast(reader.get("alt"), caption);
		auto const r = ctx_.lookup(source, "png");
		if (r.second) {

			//	generate unique tag for SVG
			auto const id = boost::uuids::to_string(tag);
			auto const scale = cyng::numeric_cast(reader.get("scale"), 1.0);

			if (scale > 1.0 || scale < 0.01) {
				std::cerr
					<< "***warning: unusual scaling factor ["
					<< scale
					<< "] for figure: "
					<< source
					<< std::endl;
			}

			auto const ext = docruntime::get_extension(r.first);
			if (boost::algorithm::iequals(ext, "svg")) {
				//
				//	embed SVG
				//
				embed_svg(caption, alt, tag, r.first, scale);
			}
			else {
				//
				//	base64 encoded images
				//
				embed_base64(caption, alt, tag, r.first, ext, scale);
			}
			os_ << std::endl;

		}
		else {
			os_
				<< "<p>"
				<< source
				<< " not found</p>"
				<< std::endl
				;
		}
	}

	void generator::embed_svg(std::string const& caption, std::string const& alt, boost::uuids::uuid tag, std::filesystem::path const& source, double scale) {

		//
		//	embedding SVG 
		//	<figure>
		//		<svg>...</svg>
		//		<figcaption>CAPTION</figcaption>
		//	</figure>
		//
		//	* remove <XML> trailer 
		//	* add title info (aria-labelledby="title")

		cyng::xml::document doc;
		auto svg = doc.read_file(source.string(), "svg");
		if (!svg.empty()) {
			svg.set_attribute("aria-labelledby", "title");
			svg.add_leaf("title", caption);
			auto const id = boost::uuids::to_string(tag);
			svg.set_attribute("id", id);

			auto const max_width = std::to_string(scale * 100.0) + "%";
			svg.set_attribute("width", max_width);
			//	https://www.w3.org/TR/SVG/types.html#DataTypeLength
			svg.set_attribute("height", "100%");
			//	remove private data
			svg.del_attribute("inkscape:export-filename");
			auto const src = doc.to_str();
			auto const title = compute_title_figure(tag, caption);
			auto const figure = dom::figure(dom::id_(id), dom::div(dom::class_("smf-svg"), src), dom::figcaption(title));
			figure.serialize(os_);
		}
		else {
			os_ << "<div>SVG not found</div>" << std::endl;
		}
	}
	void generator::embed_base64(std::string const& caption
		, std::string const& alt
		, boost::uuids::uuid tag
		, std::filesystem::path const& source
		, std::string const& ext
		, double scale) {

		std::ifstream ifs(source.string(), std::ios::binary | std::ios::ate);
		ifs.unsetf(std::ios::skipws);

		//
		//	load image into buffer
		//
		auto [buffer, length] = docruntime::stream_to_buffer(ifs);

		//
		//	create HTML structure
		//
		auto const id = boost::uuids::to_string(tag);
		auto const max_width = std::to_string(scale * 100.0) + "%";
		//	compute title (number + caption)
		auto const title = compute_title_figure(tag, caption);
		auto const figure = dom::figure(dom::id_(id), dom::img(dom::alt_(alt), dom::title_(caption), dom::class_("docscript-img"), dom::style_("max-width: " + max_width), dom::src_("data:image/" + ext + ";base64," + cyng::crypto::base64_encode(buffer.data(), buffer.size()))), dom::figcaption(title));
		figure.serialize(os_);
	}

	void generator::table_of_content(cyng::param_map_t) {
		os_ << "<div>TOC</div>\n";

		//os_
		//	<< "<details>\n"
		//	<< "\t<summary>"
		//	<< get_name(get_language_code(), i18n::TOC)
		//	<< "</summary>\n"
		//	;

		////
		////	read toc file generated during the last run.
		////
		//auto vec = cyng::container_cast<cyng::vector_t>(cyng::json::parse_file(index_file_));
		//emit_toc(vec, 0);

		//os_
		//	<< "</details>\n"
		//	;
	}

	void generator::emit_toc(cyng::vector_t vec, std::size_t level) {

		os_
			<< std::string(level + 1, '\t')
			<< "<ul toclevel-"
			<< level
			<< ">"
			<< std::endl
			;

		for (auto const& obj : vec) {

			auto const reader = cyng::make_reader(obj);
			auto const title = reader.get("title", "title");
			auto const number = reader.get("number", "1");
			auto const tag = reader.get("tag", "");
			auto const href = "#" + tag;

			auto const a = dom::a(dom::href_(href), dom::title_("section " + number), number + "&nbsp;" + title);
			auto const sub = reader.get("sub");
			if (sub) {
				os_
					<< std::string(level + 1, '\t')
					<< "<li>"
					<< a(0)
					<< std::endl
					;

				emit_toc(cyng::container_cast<cyng::vector_t>(sub), level + 1);

				os_
					<< std::string(level + 1, '\t')
					<< "</li>"
					<< std::endl;

			}
			else {
				os_
					<< std::string(level + 1, '\t')
					<< "<li>"
					<< a(0)
					<< "</li>"
					<< std::endl
					;

			}

		}

		os_
			<< std::string(level + 1, '\t')
			<< "</ul>"
			<< std::endl
			;

	}


	void generator::code(cyng::param_map_t pm) {
		//std::cout << "CODE(" << pm << ")" << std::endl;
		auto const reader = cyng::make_reader(pm);
		auto const source = reader.get("source", "main.cpp");
		auto const lang = reader.get("language", docruntime::get_extension(source));
		auto const numbers = reader.get("linenumbers", true);
		auto const caption = reader.get("caption", "");

		auto const r = ctx_.lookup(source, lang);
		if (r.second) {
			//os_ << "<div>source file</div>" << std::endl;
			dom::code_to_html(os_, r.first, lang, numbers, caption);
		}
		else {
			fmt::print(
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***error: source file [{}] not found\n", source);
			os_ << "<div>source file not found</div>" << std::endl;

		}
	}

	void generator::tree(cyng::param_map_t pm) {
		auto const reader = cyng::make_reader(pm);
		auto const root = reader.get("root", ".");
		dom::render_tree(os_, root);
	}

	void generator::table(cyng::param_map_t pm) {
		auto const reader = cyng::make_reader(pm);
		auto const source = reader.get("source", "table.csv");
		auto const header = reader.get("title", true);
		dom::render_table(os_, source, header);
	}

	void generator::resource(cyng::param_map_t pm) {
		std::cout << "RESOURCE(" << pm << ")" << std::endl;
	}
	std::chrono::system_clock::time_point generator::now(cyng::param_map_t pm) {
		return std::chrono::system_clock::now();
	}
	boost::uuids::uuid generator::uuid(cyng::param_map_t) {
		return uuid_gen_();
	}

	cyng::vector_t generator::range(cyng::vector_t vec) {
		return vec;
	}
	std::string generator::fuse(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		dom::to_html(ss, vec, "");	//	empty separator here
		return ss.str();
	}
	std::string generator::cat(cyng::param_map_t pm) {
		//std::reverse(std::begin(vec), std::end(vec));
		//std::stringstream ss;
		//dom::to_html(ss, vec, "SEP");	//	separator here
		return "CAT(SEP)";
	}

	//		insert_method(table, method("repeat", parameter_type::MAP, true, { "count", "value", "sep"}));
	std::string generator::repeat(cyng::param_map_t pm) {
		//	ToDo: read CSV file
		return "";
	}

	std::string generator::currency(cyng::param_map_t pm) {
		//	https://www.fileformat.info/info/unicode/category/Sc/list.htm
		//	https://www.w3schools.com/charsets/ref_utf_currency.asp
		//return std::string("\xE2\x82\xB9");	//	indian rupee

		auto const reader = cyng::make_reader(pm);
		auto const value = cyng::numeric_cast<std::size_t>(reader.get("value"), 0);
		auto const name = cyng::value_cast<std::string>(reader.get("name"), "euro");

		return docruntime::currency_html(value, name);
	}

	std::function<std::string(cyng::vector_t)> generator::f_esc() {
		return std::bind(&generator::esc, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::vector_t)> generator::f_quote() {
		return std::bind(&generator::quote, this, std::placeholders::_1);
	}

	std::function<void(cyng::param_map_t)> generator::f_set() {
		return std::bind(&generator::set, this, std::placeholders::_1);
	}
	std::function<cyng::vector_t(cyng::vector_t)> generator::f_get() {
		return std::bind(&generator::get, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> generator::f_meta() {
		return std::bind(&generator::meta, this, std::placeholders::_1);
	}

	std::function<void(cyng::vector_t)> generator::f_paragraph() {
		return std::bind(&generator::paragraph, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> generator::f_italic() {
		return std::bind(&generator::italic, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> generator::f_bold() {
		return std::bind(&generator::bold, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> generator::f_typewriter() {
		return std::bind(&generator::typewriter, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> generator::f_label() {
		return std::bind(&generator::label, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> generator::f_ref() {
		return std::bind(&generator::ref, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> generator::f_h1() {
		return std::bind(&generator::h1, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> generator::f_h2() {
		return std::bind(&generator::h2, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> generator::f_h3() {
		return std::bind(&generator::h3, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> generator::f_h4() {
		return std::bind(&generator::h4, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> generator::f_h5() {
		return std::bind(&generator::h5, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> generator::f_h6() {
		return std::bind(&generator::h6, this, std::placeholders::_1);
	}

	std::function<void(cyng::param_map_t)> generator::f_header() {
		return std::bind(&generator::header, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> generator::f_figure() {
		return std::bind(&generator::figure, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> generator::f_toc() {
		return std::bind(&generator::table_of_content, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> generator::f_resource() {
		return std::bind(&generator::resource, this, std::placeholders::_1);
	}
	std::function<std::chrono::system_clock::time_point(cyng::param_map_t)> generator::f_now() {
		return std::bind(&generator::now, this, std::placeholders::_1);
	}
	std::function<boost::uuids::uuid(cyng::param_map_t)> generator::f_uuid() {
		return std::bind(&generator::uuid, this, std::placeholders::_1);
	}
	std::function<cyng::vector_t(cyng::vector_t)> generator::f_range() {
		return std::bind(&generator::range, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> generator::f_fuse() {
		return std::bind(&generator::fuse, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::param_map_t)> generator::f_cat() {
		return std::bind(&generator::cat, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::param_map_t pm)> generator::f_repeat() {
		return std::bind(&generator::repeat, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::param_map_t)> generator::f_currency() {
		return std::bind(&generator::currency, this, std::placeholders::_1);
	}

	std::function<void(cyng::param_map_t)> generator::f_code() {
		return std::bind(&generator::code, this, std::placeholders::_1);
	}

	std::function<void(cyng::param_map_t)> generator::f_tree() {
		return std::bind(&generator::tree, this, std::placeholders::_1);
	}

	std::function<void(cyng::param_map_t)> generator::f_table() {
		return std::bind(&generator::table, this, std::placeholders::_1);
	}

	std::string generator::compute_title_figure(boost::uuids::uuid tag, std::string caption) {
		//
		// append to figure list
		//
		//figures_.emplace_back(tag, caption);

		//auto const idx = figures_.size();

		//std::stringstream ss;
		//ss
		//	<< get_name(get_language_code(), i18n::FIGURE)
		//	<< ": "
		//	<< idx
		//	<< " - "
		//	<< caption
		//	;

		//return ss.str();

		return "FIGURE";
	}

	std::string generator::compute_title_table(boost::uuids::uuid tag, std::string caption) {
		//
		// append to table list
		//
		//tables_.emplace_back(tag, caption);

		//auto const idx = tables_.size();

		//std::stringstream ss;
		//ss
		//	<< get_name(get_language_code(), i18n::TABLE)
		//	<< ": "
		//	<< idx
		//	<< " - "
		//	<< caption
		//	;

		//return ss.str();
		return "TABLE";
	}

	cyng::io::language_codes generator::get_language_code() const {
		auto const lang = get_language();
		return (lang.size() == 2)
			? cyng::io::get_language_code(lang.at(0), lang.at(1))
			: cyng::io::LC_EN
			;
	}

	std::string generator::get_language() const
	{
		auto const reader = cyng::make_reader(meta_);
		auto const lang = cyng::value_cast<std::string>(reader.get("language"), "en");
		return (lang.size() == 2)
			? boost::algorithm::to_lower_copy(lang)
			: "en"
			;
	}

	void generator::emit_navbar(std::ostream& os, navbar const& nb, page const& p) {
		//	see https://getbootstrap.com/docs/5.0/examples/headers/#
		os << "<div class=\"container-fluid\">\n";
		os << "\t<nav class=\"navbar navbar-dark bg-primary\">\n";
		os << "\t\t<a class=\"navbar-brand\" href=\"#\">"
			<< p.title_
			<< "</a>\n";
		os << "\t</nav>\n";
		os << "</div>\n";
	}

	void generator::emit_svg(std::ostream& os) {

		//	reference this with:
		//	<svg class="bi me-2" width="16" height="16" role="img" aria-label="Logo"><use xlink:href="#logo"/></svg>
		//
		os << R"svg(<svg xmlns="http://www.w3.org/2000/svg" style="display: none;">)svg" << '\n';
		os << R"html(<symbol id="logo" viewBox="0 0 16 16">)html" << '\n';
		os << R"html(		<title>Logo</title>)html" << '\n';
		os << R"svg(		<ellipse fill="#003f7f" stroke="#000" cx="16" cy="16" id="logo" rx="16" ry="16" stroke-width="2"/>)svg" << '\n';
		os << R"html(	</symbol>)html" << '\n';
		os << R"svg(
	  <symbol id="home" viewBox="0 0 16 16">
		<path d="M8.354 1.146a.5.5 0 0 0-.708 0l-6 6A.5.5 0 0 0 1.5 7.5v7a.5.5 0 0 0 .5.5h4.5a.5.5 0 0 0 .5-.5v-4h2v4a.5.5 0 0 0 .5.5H14a.5.5 0 0 0 .5-.5v-7a.5.5 0 0 0-.146-.354L13 5.793V2.5a.5.5 0 0 0-.5-.5h-1a.5.5 0 0 0-.5.5v1.293L8.354 1.146zM2.5 14V7.707l5.5-5.5 5.5 5.5V14H10v-4a.5.5 0 0 0-.5-.5h-3a.5.5 0 0 0-.5.5v4H2.5z"/>
	  </symbol>
	  <symbol id="speedometer2" viewBox="0 0 16 16">
		<path d="M8 4a.5.5 0 0 1 .5.5V6a.5.5 0 0 1-1 0V4.5A.5.5 0 0 1 8 4zM3.732 5.732a.5.5 0 0 1 .707 0l.915.914a.5.5 0 1 1-.708.708l-.914-.915a.5.5 0 0 1 0-.707zM2 10a.5.5 0 0 1 .5-.5h1.586a.5.5 0 0 1 0 1H2.5A.5.5 0 0 1 2 10zm9.5 0a.5.5 0 0 1 .5-.5h1.5a.5.5 0 0 1 0 1H12a.5.5 0 0 1-.5-.5zm.754-4.246a.389.389 0 0 0-.527-.02L7.547 9.31a.91.91 0 1 0 1.302 1.258l3.434-4.297a.389.389 0 0 0-.029-.518z"/>
		<path fill-rule="evenodd" d="M0 10a8 8 0 1 1 15.547 2.661c-.442 1.253-1.845 1.602-2.932 1.25C11.309 13.488 9.475 13 8 13c-1.474 0-3.31.488-4.615.911-1.087.352-2.49.003-2.932-1.25A7.988 7.988 0 0 1 0 10zm8-7a7 7 0 0 0-6.603 9.329c.203.575.923.876 1.68.63C4.397 12.533 6.358 12 8 12s3.604.532 4.923.96c.757.245 1.477-.056 1.68-.631A7 7 0 0 0 8 3z"/>
	  </symbol>
	  <symbol id="table" viewBox="0 0 16 16">
		<path d="M0 2a2 2 0 0 1 2-2h12a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2H2a2 2 0 0 1-2-2V2zm15 2h-4v3h4V4zm0 4h-4v3h4V8zm0 4h-4v3h3a1 1 0 0 0 1-1v-2zm-5 3v-3H6v3h4zm-5 0v-3H1v2a1 1 0 0 0 1 1h3zm-4-4h4V8H1v3zm0-4h4V4H1v3zm5-3v3h4V4H6zm4 4H6v3h4V8z"/>
	  </symbol>
	  <symbol id="people-circle" viewBox="0 0 16 16">
		<path d="M11 6a3 3 0 1 1-6 0 3 3 0 0 1 6 0z"/>
		<path fill-rule="evenodd" d="M0 8a8 8 0 1 1 16 0A8 8 0 0 1 0 8zm8-7a7 7 0 0 0-5.468 11.37C3.242 11.226 4.805 10 8 10s4.757 1.225 5.468 2.37A7 7 0 0 0 8 1z"/>
	  </symbol>
	  <symbol id="grid" viewBox="0 0 16 16">
		<path d="M1 2.5A1.5 1.5 0 0 1 2.5 1h3A1.5 1.5 0 0 1 7 2.5v3A1.5 1.5 0 0 1 5.5 7h-3A1.5 1.5 0 0 1 1 5.5v-3zM2.5 2a.5.5 0 0 0-.5.5v3a.5.5 0 0 0 .5.5h3a.5.5 0 0 0 .5-.5v-3a.5.5 0 0 0-.5-.5h-3zm6.5.5A1.5 1.5 0 0 1 10.5 1h3A1.5 1.5 0 0 1 15 2.5v3A1.5 1.5 0 0 1 13.5 7h-3A1.5 1.5 0 0 1 9 5.5v-3zm1.5-.5a.5.5 0 0 0-.5.5v3a.5.5 0 0 0 .5.5h3a.5.5 0 0 0 .5-.5v-3a.5.5 0 0 0-.5-.5h-3zM1 10.5A1.5 1.5 0 0 1 2.5 9h3A1.5 1.5 0 0 1 7 10.5v3A1.5 1.5 0 0 1 5.5 15h-3A1.5 1.5 0 0 1 1 13.5v-3zm1.5-.5a.5.5 0 0 0-.5.5v3a.5.5 0 0 0 .5.5h3a.5.5 0 0 0 .5-.5v-3a.5.5 0 0 0-.5-.5h-3zm6.5.5A1.5 1.5 0 0 1 10.5 9h3a1.5 1.5 0 0 1 1.5 1.5v3a1.5 1.5 0 0 1-1.5 1.5h-3A1.5 1.5 0 0 1 9 13.5v-3zm1.5-.5a.5.5 0 0 0-.5.5v3a.5.5 0 0 0 .5.5h3a.5.5 0 0 0 .5-.5v-3a.5.5 0 0 0-.5-.5h-3z"/>
	  </symbol>
	)svg";
		os << "</svg>\n";
		//	start main container
		os << "<div class=\"container\">\n";
	}


	void generator::emit_footer(std::ostream& os, footer const& f) {
		//	end main container
		os << "</div>\n";
		os << "<div class=\"container-fluid\">\n";
		os << "\t<footer class=\"fixed-bottom mt-auto py-3 " << f.bg_color_ << "\">\n";
		os << "\t\t<div class=\"container\">\n";
		os << "\t\t\t<span class=\"text-muted\">" << f.content_ << "</span>\n";
		os << "\t\t</div>\n";
		os << "\t</footer>\n";
		os << "</div>\n";
	}

}
