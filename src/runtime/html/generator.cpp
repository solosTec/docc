
#include "generator.h"
#include <rt/currency.h>
#include <rt/stream.h>
#include <rt/i18n.h>
#include <html/formatting.h>
#include <html/dom.hpp>

#include <cyng/vm/vm.h>
#include <cyng/io/parser/parser.h>
#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>
#include <cyng/obj/algorithm/reader.hpp>
#include <cyng/obj/numeric_cast.hpp>
#include <cyng/obj/container_cast.hpp>
#include <cyng/xml/node.h>

#include <smfsec/hash/base64.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>

#include <boost/uuid/uuid_io.hpp>

namespace docruntime {

	void show(std::string str) {
		std::cout << str << std::endl;
	}


	generator::generator(std::istream& is
		, std::ostream& os
		, cyng::mesh& fabric
		, boost::uuids::uuid tag
		, docscript::context& ctx)
		: is_(is)
		, os_(os)
		, vars_()
		, meta_()
		, toc_()
		, footnotes_()
		, figures_()
		, tables_()
		, uuid_gen_()
		, vm_(fabric.make_proxy(tag
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
			, cyng::make_description("fuse", f_fuse())
			, cyng::make_description("cat", f_cat())
			, cyng::make_description("repeat", f_repeat())
			, cyng::make_description("currency", f_currency())
			, cyng::make_description("show", f_show())
		))
		, ctx_(ctx)
	{
		meta_.emplace("build", cyng::make_object(std::chrono::system_clock::now()));
	}

	cyng::param_map_t& generator::get_vars() {
		return vars_;
	}
	cyng::param_map_t& generator::get_meta() {
		return meta_;
	}
	docruntime::toc& generator::get_toc() {
		return toc_;
	}

	int generator::run() {

		//
		//	load program into buffer
		//
		auto [buffer, length] = stream_to_buffer(is_);

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
		bool init = false;
		dom::to_html(ss, vec, " ");
		ss << "&ldquo;";
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
		meta_.insert(pm.begin(), pm.end());
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

	std::string generator::paragraph(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		os_ << "<p>";
		dom::to_html(os_, vec, " ");
		os_ << "</p>" << std::endl;
		return "";
	}

	void generator::label(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "LABEL(" << vec << ")";
		std::cout << ss.str() << std::endl;
	}

	std::string generator::ref(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "REF(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	void generator::h1(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const r = toc_.add(0, uuid_gen_(), title);
		emit_header(0, uuid_gen_(), r.first, title);
	}
	void generator::h2(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const r = toc_.add(1, uuid_gen_(), title);
		emit_header(1, uuid_gen_(), r.first, title);
	}
	void generator::h3(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const r = toc_.add(2, uuid_gen_(), title);
		emit_header(2, uuid_gen_(), r.first, title);
	}
	void generator::h4(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const r = toc_.add(3, uuid_gen_(), title);
		emit_header(3, uuid_gen_(), r.first, title);
	}
	void generator::h5(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const r = toc_.add(4, uuid_gen_(), title);
		emit_header(4, uuid_gen_(), r.first, title);
	}
	void generator::h6(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		auto const title = dom::to_html(vec, " ");
		auto const r = toc_.add(5, uuid_gen_(), title);
		emit_header(5, uuid_gen_(), r.first, title);
	}
	void generator::header(cyng::param_map_t pm) {
		//	"level":0000000000000001),("tag":<uuid>'79bf3ba0-2362-4ea5-bcb5-ed93844ac59a'),("title":[Basics]))
		auto const reader = cyng::make_reader(pm);
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0);
		auto const tag = cyng::value_cast(reader.get("tag"), uuid_gen_());
		auto const vec = cyng::container_cast<cyng::vector_t>(reader.get("title"));
		auto const title = dom::to_html(vec, " ");
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
			//<a id="fb8cb774-6981-4727-8c16-d3e9913012d7" aria-hidden="true" href="fb8cb774-6981-4727-8c16-d3e9913012d7" style="margin-right: 6px;" class="oction">
			<< "<a id=\""
			<< tag
			<< "\" aria-hidden=\"true\" "
			<< "href=\"#" << tag << "\""
			<< "class=\"oction\">"
			<< "<svg viewBox=\"0 0 16 16\" version=\"1.1\" width=\"16\" height=\"16\" aria-hidden=\"true\"><path fill-rule=\"evenodd\" d=\"M4 9h1v1H4c-1.5 0-3-1.69-3-3.5S2.55 3 4 3h4c1.45 0 3 1.69 3 3.5 0 1.41-.91 2.72-2 3.25V8.59c.58-.45 1-1.27 1-2.09C10 5.22 8.98 4 8 4H4c-.98 0-2 1.22-2 2.5S3 9 4 9zm9-3h-1v1h1c1 0 2 1.22 2 2.5S13.98 12 13 12H9c-.98 0-2-1.22-2-2.5 0-.83.42-1.64 1-2.09V6.25c-1.09.53-2 1.84-2 3.25C6 11.31 7.55 13 9 13h4c1.45 0 3-1.69 3-3.5S14.5 6 13 6z\"></path></svg>"
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
		//auto const vec = cyng::container_cast<cyng::vector_t>(reader.get("caption"));
		//auto const caption = dom::to_html(vec, " ");
		auto const tag = cyng::value_cast(reader.get("tag"), uuid_gen_());
		auto const caption = cyng::value_cast(reader.get("caption"), boost::uuids::to_string(tag));
		auto const alt = cyng::value_cast(reader.get("alt"), caption);
		std::filesystem::path const source = cyng::value_cast(reader.get("source"), "");
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

			auto const ext = get_extension(r.first);
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
			//auto node = svg.prepend_child("title");
			//node.append_child(pugi::node_pcdata).set_value(caption.c_str());
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

		//os_ << "<div>BASE64</div>" << std::endl;

		std::ifstream ifs(source.string(), std::ios::binary | std::ios::ate);
		ifs.unsetf(std::ios::skipws);

		//
		//	load image into buffer
		//
		auto [buffer, length] = stream_to_buffer(ifs);

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

	std::function<std::string(cyng::vector_t)> generator::f_paragraph() {
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

	std::function<void(std::string)> generator::f_show() {
		return std::bind(&show, std::placeholders::_1);
	}

	std::string generator::compute_title_figure(boost::uuids::uuid tag, std::string caption) {
		//
		// append to figure list
		//
		figures_.emplace_back(tag, caption);

		auto const idx = figures_.size();

		std::stringstream ss;
		ss
			<< get_name(get_language_code(), i18n::FIGURE)
			<< ": "
			<< idx
			<< " - "
			<< caption
			;

		return ss.str();
	}

	std::string generator::compute_title_table(boost::uuids::uuid tag, std::string caption) {
		//
		// append to table list
		//
		tables_.emplace_back(tag, caption);

		auto const idx = tables_.size();

		std::stringstream ss;
		ss
			<< get_name(get_language_code(), i18n::TABLE)
			<< ": "
			<< idx
			<< " - "
			<< caption
			;

		return ss.str();
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

}
