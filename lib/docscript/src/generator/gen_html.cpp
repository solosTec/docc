/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/gen_html.h>
#include "filter/json_to_html.h"
#include "filter/cpp_to_html.h"
#include "filter/docscript_to_html.h"
#include "filter/text_to_html.h"
#include "filter/binary_to_html.h"
#include "filter/html_to_html.h"
#include <html/node.hpp>

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/numeric_cast.hpp>
#include <cyng/dom/reader.h>
#include <cyng/crypto/base64.h>
#include <cyng/csv.h>
#include <pugixml.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace docscript
{
	gen_html::gen_html(std::vector< boost::filesystem::path > const& inc, bool body_only)
		: generator(inc)
		, footnotes_()
		, figures_()
		, body_only_(body_only)
	{
		register_this();
	}

	void gen_html::register_this()
	{
		generator::register_this();

		vm_.register_function("demo", 0, std::bind(&gen_html::demo, this, std::placeholders::_1));

		vm_.register_function("generate.file", 1, std::bind(&gen_html::generate_file, this, std::placeholders::_1));
		vm_.register_function("generate.meta", 1, std::bind(&gen_html::generate_meta, this, std::placeholders::_1));

		vm_.register_function("hline", 0, std::bind(&gen_html::print_hline, this, std::placeholders::_1));

		vm_.register_function("convert.numeric", 1, std::bind(&gen_html::convert_numeric, this, std::placeholders::_1));
		vm_.register_function("convert.alpha", 1, std::bind(&gen_html::convert_alpha, this, std::placeholders::_1));

		vm_.register_function("paragraph", 1, std::bind(&gen_html::paragraph, this, std::placeholders::_1));
		vm_.register_function("abstract", 1, std::bind(&gen_html::abstract, this, std::placeholders::_1));
		vm_.register_function("quote", 1, std::bind(&gen_html::quote, this, std::placeholders::_1));
		vm_.register_function("list", 1, std::bind(&gen_html::list, this, std::placeholders::_1));
		vm_.register_function("link", 1, std::bind(&gen_html::link, this, std::placeholders::_1));
		vm_.register_function("figure", 1, std::bind(&gen_html::figure, this, std::placeholders::_1));
		vm_.register_function("code", 1, std::bind(&gen_html::code, this, std::placeholders::_1));
		vm_.register_function("def", 1, std::bind(&gen_html::def, this, std::placeholders::_1));
		vm_.register_function("note", 1, std::bind(&gen_html::annotation, this, std::placeholders::_1));
		vm_.register_function("table", 1, std::bind(&gen_html::table, this, std::placeholders::_1));

		vm_.register_function("i", 1, std::bind(&gen_html::format_italic, this, std::placeholders::_1));
		vm_.register_function("b", 1, std::bind(&gen_html::format_bold, this, std::placeholders::_1));
		vm_.register_function("bold", 1, std::bind(&gen_html::format_bold, this, std::placeholders::_1));
		vm_.register_function("color", 1, std::bind(&gen_html::format_color, this, std::placeholders::_1));
		vm_.register_function("sub", 1, std::bind(&gen_html::format_sub, this, std::placeholders::_1));
		vm_.register_function("sup", 1, std::bind(&gen_html::format_sup, this, std::placeholders::_1));

		vm_.register_function("header", 1, std::bind(&gen_html::header, this, std::placeholders::_1));
		vm_.register_function("h1", 1, std::bind(&gen_html::section, this, 1, std::placeholders::_1));
		vm_.register_function("h2", 1, std::bind(&gen_html::section, this, 2, std::placeholders::_1));
		vm_.register_function("h3", 1, std::bind(&gen_html::section, this, 3, std::placeholders::_1));
		vm_.register_function("h4", 1, std::bind(&gen_html::section, this, 4, std::placeholders::_1));
		vm_.register_function("h5", 1, std::bind(&gen_html::section, this, 5, std::placeholders::_1));
		vm_.register_function("h6", 1, std::bind(&gen_html::section, this, 6, std::placeholders::_1));
		vm_.register_function("footnote", 1, std::bind(&gen_html::make_footnote, this, std::placeholders::_1));
		vm_.register_function("ref", 1, std::bind(&gen_html::make_ref, this, std::placeholders::_1));

	}

	void gen_html::generate_file(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto p = cyng::value_cast(frame.at(0), boost::filesystem::path());
		if (boost::filesystem::is_directory(p)) {
			p = p / "out.html";
		}
		std::ofstream ofs(p.string(), std::ios::out | std::ios::trunc);
		if (!ofs.is_open())
		{
			std::cerr
				<< "***error cannot open output file ["
				<< p
				<< ']'
				<< std::endl;
		}
		else
		{
			//
			//	provide a slug if not defined yet.
			//
			slug();

			//
			//	write output file
			//
			auto pos = frame.begin();
			auto end = frame.end();
			//emit_file(ofs, pos, end);
			emit_file(ofs, std::next(pos), end);
		}
	}

	std::ofstream& gen_html::emit_file(std::ofstream& ofs, cyng::vector_t::const_iterator pos, cyng::vector_t::const_iterator end) const
	{
		if (!body_only_) {
			emit_doctype(ofs);
			emit_head(ofs);
			ofs
				<< "<body>"
				<< std::endl
				;
		}
		else {
			ofs
				<< "<div>"
				<< std::endl
				;
		}
		emit_body(ofs, pos, end);
		if (!body_only_) {
			ofs
				<< "</body>"
				<< std::endl
				<< "</html>"
				<< std::endl
				;
		}
		else {
			ofs
				<< "</div>"
				<< std::endl
				;
		}
		return ofs;

	}

	std::ofstream& gen_html::emit_doctype(std::ofstream& ofs) const
	{
		ofs
			<< "<!doctype html>"
			<< std::endl
			<< "<html lang=\""
			<< get_language()
			<< "\">"
			<< std::endl
			;
		return ofs;
	}

	std::ofstream& gen_html::emit_head(std::ofstream& ofs) const
	{
		ofs
			<< "<head>"
			<< std::endl
			<< "\t<meta charset=\"utf-8\" />"
			<< std::endl
			;

		emit_meta(ofs);
		emit_styles(ofs);

		ofs
			<< "</head>"
			<< std::endl
			;

		return ofs;
	}

	std::ofstream& gen_html::emit_meta(std::ofstream& ofs) const
	{
		//	emit meta data
		for (auto const& e : meta_) {
			if (boost::algorithm::equals(e.first, "title"))
			{ 
				ofs
					<< "\t<title>"
					<< accumulate_plain_text(e.second)
					<< "</title>"
					<< std::endl
					//	@see http://ogp.me/
					<< "\t<meta name=\"og:title\" content=\""
					<< cyng::io::to_str(e.second)
					<< "\" />"
					<< std::endl
					<< "\t<meta name=\"og:type\" content=\"article\" />"
					<< std::endl
					;
			}
			else {
				ofs
					<< "\t<meta name=\""
					<< e.first
					<< "\" content=\""
					<< cyng::io::to_str(e.second)
					<< "\" />"
					<< std::endl
					;
			}
		}
		return ofs;
	}

	std::ofstream& gen_html::emit_styles(std::ofstream& ofs) const
	{
		ofs
			<< "\t<style>"
			<< std::endl
			<< "\t\tbody { "
			<< std::endl
			//	Georgia,Cambria,serif;
			<< "\t\t\tfont-family:'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;"	
			<< std::endl
			//	https://jrl.ninja/etc/1/
			<< "\t\t\tmax-width: 52rem; "
			<< std::endl
			<< "\t\t\tpadding: 2rem; "
			<< std::endl
			<< "\t\t\tmargin: auto; "
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\tp > a {"
			<< std::endl
			<< "\t\t\ttext-decoration: none;"
			<< std::endl
			<< "\t\t\tcolor: blue;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\tblockquote > p { margin-bottom: 1px; }"
			<< std::endl
			<< "\t\tpre { background-color: #fafafa; }"
			<< std::endl

			<< "\t\tpre > code:hover {"
			<< std::endl
			<< "\t\t\tbackground-color: orange;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\tblockquote {"
			<< std::endl
			<< "\t\t\tborder-left: 4px solid #eee;"
			<< std::endl
			<< "\t\t\tpadding-left: 10px;"
			<< std::endl
			<< "\t\t\tcolor: #777;"
			<< std::endl
			<< "\t\t\tmargin: 16px 20px;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\tfigure {"
			<< std::endl
			<< "\t\t\tmargin: 2%;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\timg {"
			<< std::endl
			<< "\t\t\tmax-width: 95%;"
			<< std::endl
			<< "\t\t\tborder: 2px solid #777;"
			<< std::endl
			<< "\t\t}"
			<< std::endl
			<< "\t\timg:hover {"
			<< std::endl
			<< "\t\t\tbox-shadow: 0 0 10px #ccc;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			//	definition lists with flexbox
			<< "\t\tdl {"
			<< std::endl
			<< "\t\t\tdisplay: flex;"
			<< std::endl
			<< "\t\t\tflex-flow: row wrap;"
			<< std::endl
			<< "\t\t}"
			<< std::endl
			<< "\t\tdt {"
			<< std::endl
			<< "\t\t\tfont-weight: bold;"
			<< std::endl
			<< "\t\t\tflex-basis: 20% ;"
			<< std::endl
			<< "\t\t}"
			<< std::endl
			<< "\t\tdt::after {"
			<< std::endl
			<< "\t\t\tcontent: \":\";"
			<< std::endl
			<< "\t\t}"
			<< std::endl
			<< "\t\tdd {"
			<< std::endl
			<< "\t\t\tflex-basis: 70%;"
			<< std::endl
			<< "\t\t\tflex-grow: 1;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			//	formatting sub and supscript
			<< "\t\tsup { top: -.5em; }"
			<< std::endl
			<< "\t\tsub { bottom: -.25em; }"
			<< std::endl
			<< "\t\tsub, sup {"
			<< std::endl
			<< "\t\t\tfont-size: 75%;"
			<< std::endl
			<< "\t\t\tline-height: 0;"
			<< std::endl
			<< "\t\t\tposition: relative;"
			<< std::endl
			<< "\t\t\tvertical-align: baseline;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			//	There is an alternative styling for aside tag that works
			//	for bootstrap 4 too: float: right;
			<< "\t\taside {"
			<< std::endl
			<< "\t\t\tposition: absolute;"
			<< std::endl
			<< "\t\t\tright: 2em;"
			<< std::endl
			<< "\t\t\twidth: 20%;"
			<< std::endl
			<< "\t\t\tborder: 1px #D5DBDB solid;"
			<< std::endl
			<< "\t\t\tcolor: #9C640C;"
			<< std::endl
			<< "\t\t\tpadding: 0.5em;"
			<< std::endl
			<< "\t\t\tz-index: -1;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\ttable {"
			<< std::endl
			<< "\t\t\tborder-collapse: collapse;"
			<< std::endl
			<< "\t\t\tborder-spacing: 0px;"
			<< std::endl
			<< "\t\t}"
			<< std::endl
			<< "\t\ttable, th, td {"
			<< "\t\t\tpadding: 5px;"
			<< std::endl
			<< "\t\t\tborder: 1px solid black;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\tcaption {"
			<< std::endl
			<< "\t\t\tfont-weight: bold;"
			<< std::endl
			<< "\t\t\tcolor: white;"
			<< std::endl
			<< "\t\t\tbackground-color: DimGray;"
			<< std::endl
			<< "\t\t\tpadding: 5px;"
			<< std::endl
			<< "\t\t\ttext-align: left;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t</style>"
			<< std::endl

			;
		return ofs;
	}

	std::ofstream& gen_html::emit_body(std::ofstream& ofs, cyng::vector_t::const_iterator pos, cyng::vector_t::const_iterator end) const
	{
		while (pos != end) {
			emit_body(ofs, *pos);
			++pos;
		}

		emit_footnotes(ofs);
		return ofs;
	}

	std::ofstream& gen_html::emit_body(std::ofstream& ofs, cyng::object obj) const
	{
		ofs
			<< cyng::io::to_str(obj)
			<< std::endl
			;
		return ofs;
	}

	std::ofstream& gen_html::emit_footnotes(std::ofstream& ofs) const
	{
		if (!footnotes_.empty()) {

			//
			//	horizontal line
			//
			ofs 
				<< html::hr().to_str()
				<< std::endl;

			//
			//	footnotes
			//
			std::size_t idx{ 0 };
			std::stringstream ss;
			for (auto const& note : footnotes_) {

				++idx;

				ss
					<< '['
					<< idx
					<< ']'
					<< ' '
					<< note.get_note()
					;

				ofs 
					<< html::p(html::id_(note.get_tag()), html::class_("docscript-footnote"), ss.str()).to_str()
					<< std::endl;

				ss.str("");
			}
		}
		return ofs;
	}

	void gen_html::generate_meta(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const p = cyng::value_cast(frame.at(0), boost::filesystem::path());
	}

	void gen_html::convert_numeric(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		ctx.push(frame.at(0));
	}

	void gen_html::convert_alpha(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
//		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const str = cyng::value_cast<std::string>(frame.at(0), "");

		//
		//	escape HTML entities
		//
		ctx.push(cyng::make_object(replace_html_entities(str)));
	}

	void gen_html::paragraph(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::string par = accumulate_plain_text(frame);
//		for (auto obj : frame) {
//			par.append(cyng::io::to_str(obj));
//			par.append(" ");
//		}
		auto el = html::p(par);
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::abstract(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
// 		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));

		auto const title = cyng::value_cast<std::string>(reader.get("title"), "Abstract");
		auto const text = accumulate_plain_text(reader.get("text"));


		// default state is: open
		auto const el = html::details(html::open_(std::string()), html::summary(title), html::p(text));
		ctx.push(cyng::make_object(el.to_str()));
	}
	
	void gen_html::quote(cyng::context& ctx)
	{
		//	[%(("q":{1,2,3}),("source":Earl Wilson),("url":https://www.brainyquote.com/quotes/quotes/e/earlwilson385998.html))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));

		//	Attribution for the quotation, if any, must be placed outside the blockquote element.
		//	structure:
		//	<figure>
		//		<blockquote cite=[CITE]>
		//			<p>[QUOTE]</p>
		//		</blockquote>
		//		<figcaption>
		//			<cite>[SOURCE]</cite>
		//		</figcaption>
		//	</figure>
		//
		auto const cite = cyng::value_cast<std::string>(reader.get("url"), "");
		auto const source = cyng::value_cast<std::string>(reader.get("source"), "");
		auto const quote = accumulate_plain_text(reader.get("q"));
		auto const el = html::figure(html::blockquote(html::cite_(cite), html::class_("docscript-quote"), quote), html::figcaption(html::cite(source)));
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::list(cyng::context& ctx)
	{
		//	[%(("items":[<p>one </p>,<p>two </p>,<p>three </p>]),("style":{lower-roman}),("type":ordered))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const type = cyng::value_cast<std::string>(reader.get("type"), "ordered");
		auto const style = cyng::value_cast<std::string>(reader.get("style"), "disc");
		cyng::vector_t items;
		items = cyng::value_cast(reader.get("items"), items);

		auto el = (boost::algorithm::equals(type, "ordered") || boost::algorithm::equals(type, "ol"))
			? html::ol(html::style_("list-style-type:" + style), html::class_("docscript-ol"))
			: html::ul(html::style_("list-style-type:" + style), html::class_("docscript-ul"));

		for (auto const& item : items) {
			el += html::li(accumulate_plain_text(item));
		}

		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::link(cyng::context& ctx)
	{
		//	[%(("text":{LaTeX}),("url":{https,:,//www,.,latex-project,.,org/}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));
		auto const text = accumulate_plain_text(reader.get("text"));
		auto const url = cyng::io::to_str(reader.get("url"));
		auto const title = accumulate_plain_text(reader.get("title"));

		auto const el = html::a(html::href_(url), html::title_(title), text);
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::figure(cyng::context& ctx)
	{
		//	  [%(("alt":{Giovanni,Bellini,,,Man,wearing,a,turban}),("caption":{Giovanni,Bellini,,,Man,wearing,a,turban}),("source":LogoSmall.jpg),("tag":{338d542a-a4e3-4a4c-9efe-b8d3032306c3}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const alt = accumulate_plain_text(reader.get("alt"));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const source = cyng::io::to_str(reader.get("source"));
		auto const tag = cyng::value_cast(reader.get("tag"), source);
		//	generate unique tag for SVG
		auto const id = cyng::value_cast(reader.get("id"), boost::uuids::to_string(uuid_gen_()));

		auto const p = resolve_path(source);
		if (boost::filesystem::exists(p) && boost::filesystem::is_regular(p)) {

			//
			// append to figure list
			//
			figures_.emplace_back(name_gen_(tag), caption);

			auto const idx = figures_.size();

			std::stringstream ss;
			ss
				<< get_name(i18n::WID_FIGURE)
				<< ": "
				<< idx
				<< " - "
				<< caption
				;
			auto const title = ss.str();


			auto const ext = get_extension(p);
			if (boost::algorithm::iequals(ext, "svg")) {

				//
				//	embedding SVG 
				//	<figure>
				//		<svg>...</svg>
				//		<figcaption>CAPTION</figcaption>
				//	</figure>
				//
				//	* remove <XML> trailer 
				//	* add title info (aria-labelledby="title")

				//
				//	read into buffer
				//
				pugi::xml_document doc;
				pugi::xml_parse_result result = doc.load_file(p.string().c_str());
				if (result) {
					
					pugi::xml_node svg = doc.child("svg");
					if (svg) {
						svg.prepend_attribute("aria-labelledby") = "title";
						auto node = svg.prepend_child("title");
						node.append_child(pugi::node_pcdata).set_value(caption.c_str());

						auto id_attr = svg.attribute("id");
						if (!id_attr) {
							//
							//	does not exist
							//
							svg.prepend_attribute("id") = id.c_str();
						}
						else {
							//
							//	update
							//
							id_attr.set_value(id.c_str());
						}

						//
						//	fix with and height attribute
						//
						auto width = svg.attribute("width");
						if (!width) {
							svg.prepend_attribute("width") = "100%";
						}
						else {
							width.set_value("100%");
						}

						//	https://www.w3.org/TR/SVG/types.html#DataTypeLength
						auto height = svg.attribute("height");
						if (!height) {
							svg.prepend_attribute("height") = "100%";
						}
						else {
							height.set_value("100%");
						}

						//
						//	remove private data
						//
						svg.remove_attribute("inkscape:export-filename");
					}

					std::stringstream ss;
					ss << std::endl;
					doc.save(ss, "\t", pugi::format_default | pugi::format_no_declaration);
					auto const el = html::figure(html::id_(tag), ss.str(), html::figcaption(title));
					ctx.push(cyng::make_object(el.to_str()));
				}
				else {
					std::cerr << "SVG [" << p << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
					std::cerr << "Error description: " << result.description() << "\n";
					std::cerr << "Error offset: " << result.offset << " (error at [..." << result.offset << "]\n\n";
					ctx.push(cyng::make_object(result.description()));
				}
			}
			else {
				std::ifstream ifs(p.string(), std::ios::binary | std::ios::ate);
				//
				//	do not skip 
				//
				ifs.unsetf(std::ios::skipws);

				//
				//	get file size
				//
				std::streamsize size = ifs.tellg();
				ifs.seekg(0, std::ios::beg);

				//
				//	read into buffer
				//
				cyng::buffer_t buffer(size);
				ifs.read(buffer.data(), size);
				BOOST_ASSERT(ifs.gcount() == size);

				//<figure>
				//  <img src="SOURCE" alt="ALT">
				//  <figcaption>CAPTION</figcaption>
				//</figure>

				//
				//	encode image as base 64
				//
				//"data:image/" + get_extension(p) + ";base64," + cyng::crypto::base64_encode(buffer.data(), buffer.size())

				auto const el = html::figure(html::id_(tag), html::img(html::alt_(alt), html::title_(caption), html::class_("docscript-img"), html::src_("data:image/" + ext + ";base64," + cyng::crypto::base64_encode(buffer.data(), buffer.size()))), html::figcaption(title));
				ctx.push(cyng::make_object(el.to_str()));
			}
		}
		else {

			std::cerr
				<< "***error cannot open figure file ["
				<< source
				<< "]"
				<< std::endl;
			ctx.push(cyng::make_object("cannot open file [" + source + "]"));
		}
	}

	void gen_html::code(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const line_numbers = value_cast(reader.get("linenumbers"), false);

		auto const source = cyng::value_cast<std::string>(reader.get("source"), "source.txt");
		auto const p = resolve_path(source);
		auto const language = cyng::value_cast(reader.get("language"), get_extension(p));

		if (boost::filesystem::exists(p) && boost::filesystem::is_regular(p)) {

			std::stringstream ss;
			if (boost::algorithm::iequals(language, "json")) {

				std::ifstream  ifs(p.string());
				ss << "<pre class=\"docscript-pre-json\">" << std::endl;
				std::string const inp(static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str());
				json_to_html filter(line_numbers, uuid_gen_());
				filter.convert(ss, inp);
				ss << "</pre>" << std::endl;

			}
			else if (boost::algorithm::equals(language, "C++") || boost::algorithm::iequals(language, "cpp") || boost::algorithm::iequals(language, "h")) {

				std::ifstream  ifs(p.string());
				ss << "<pre class=\"docscript-pre\">" << std::endl;
				std::string const inp(static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str());
				cpp_to_html filter(line_numbers, uuid_gen_());
				filter.convert(ss, inp);
				ss << "</code></pre>" << std::endl;
			}
			else if (boost::algorithm::iequals(language, "docscript")) {

				std::ifstream  ifs(p.string());
				ss << "<pre class=\"docscript-pre\"><code>";
				std::string const inp(static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str());
				docscript_to_html filter(line_numbers, uuid_gen_());
				filter.convert(ss, inp);
				ss << "</code></pre>" << std::endl;
			}
			else if (boost::algorithm::equals(language, "txt") || boost::algorithm::iequals(language, "text") || boost::algorithm::iequals(language, "verbatim")) {

				std::ifstream  ifs(p.string());
				ss << "<pre class=\"docscript-pre-txt\">" << std::endl;
				std::string const inp(static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str());
				text_to_html filter(line_numbers, uuid_gen_());
				filter.convert(ss, inp);
				ss << "</code></pre>" << std::endl;
			}
			else if (boost::algorithm::equals(language, "bin") || boost::algorithm::iequals(language, "binary")) {

				// binary mode required
				std::ifstream  ifs(p.string(), std::ios::binary);
				ifs.unsetf(std::ios::skipws);
				ss << "<pre class=\"docscript-pre-binary\">" << std::endl;
				cyng::buffer_t const inp((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
				//cyng::buffer_t const inp(static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str());
				binary_to_html filter(line_numbers, uuid_gen_());
				filter.convert(ss, inp);
				ss << "</code></pre>" << std::endl;
			}
			else if (boost::algorithm::equals(language, "html") || boost::algorithm::iequals(language, "htm")) {

				std::ifstream  ifs(p.string());
				ss << "<pre class=\"docscript-pre-html\">" << std::endl;
				std::string const inp(static_cast<std::stringstream const&>(std::stringstream() << ifs.rdbuf()).str());
				html_to_html filter(line_numbers, uuid_gen_());
				filter.convert(ss, inp);
				ss << "</code></pre>" << std::endl;
			}
			else {
				std::ifstream  ifs(p.string());
				ss
					<< "<pre><code>"
					<< std::endl
					<< ifs.rdbuf()
					<< std::endl
					<< "</code></pre>"
					<< std::endl
					;
			}

			ctx.push(cyng::make_object(ss.str()));
		}
		else {

			std::cerr
				<< "***error ["
				<< p
				<< "] does not exist or is not a regular file"
				<< std::endl;
				auto const el = html::strong(p.string());
				ctx.push(cyng::make_object(el.to_str()));

		}
	}

	void gen_html::def(cyng::context& ctx)
	{
		//	[%(("abstract syntax":[Specification,of,a,structure]),("big endian":[A,byte,order,sequence]))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		std::stringstream ss;
		ss
			<< "<dl class=\"docscript-dl\">"
			<< std::endl
			;

		cyng::param_map_t defs;
		defs = cyng::value_cast(frame.at(0), defs);
		for (auto const& def : defs) {
			ss
				<< html::dt(def.first).to_str()
				<< '\t'
				<< html::dd(accumulate_plain_text(def.second)).to_str()
				<< std::endl
				;
		}

		ss
			<< "</dl>"
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_html::annotation(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const el = html::aside(accumulate_plain_text(frame));
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::table(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const title = accumulate_plain_text(reader.get("title"));
		cyng::tuple_t header;
		header = cyng::value_cast(reader.get("header"), header);
		auto const source = cyng::io::to_str(reader.get("source"));
		auto const tag = cyng::value_cast(reader.get("tag"), source);

		auto const p = resolve_path(source);
		if (boost::filesystem::exists(p) && boost::filesystem::is_regular(p)) {

			auto table = html::table(html::class_("docscript-table"));
			table += html::caption(html::class_("docscript-table-caption"), title);

			//
			//	parse the CSV file into a vector
			//
			auto const csv = cyng::csv::read_file(p.string());

			bool initial{ true };
			auto head = html::thead(html::class_("docscript-table-head"));
			auto body = html::tbody(html::class_("docscript-table-body"));
			for (auto const& row : csv) {

				auto tr = html::tr(html::class_("docscript-tr"));
				cyng::tuple_t tpl;
				tpl = cyng::value_cast(row, tpl);
				for (auto const& cell : tpl) {
					auto td = (initial) 
						? html::th(html::class_("docscript-th"), cyng::io::to_str(cell))
						: html::td(html::class_("docscript-td"), cyng::io::to_str(cell))
						;
					tr += std::move(td);
				}

				if (initial) {
					initial = false;
					head += std::move(tr);
				}
				else {
					body += std::move(tr);
				}
			}

			table += std::move(head);
			table += std::move(body);
			ctx.push(cyng::make_object(table.to_str()));
		}
		else {

			std::cerr
				<< "***error cannot open figure file ["
				<< source
				<< "]"
				<< std::endl;
			ctx.push(cyng::make_object("cannot open file [" + source + "]"));
		}
	}

	void gen_html::make_ref(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(cyng::io::to_str(frame)));
	}

	void gen_html::format_italic(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto el = html::em(accumulate_plain_text(frame));
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::format_bold(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto el = html::b(accumulate_plain_text(frame));
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::format_color(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame);
		const auto map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		if (!map.empty()) {
			auto const color = map.begin()->first;
			auto const str = accumulate_plain_text(map.begin()->second);
			auto el = html::span(html::style_("color:" + color), str);
			ctx.push(cyng::make_object(el.to_str()));
		}
		else {

			ctx.push(cyng::make_object("***error in COLOR definition"));
		}
	}

	void gen_html::format_sub(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const el = html::sub(accumulate_plain_text(frame));
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::format_sup(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const el = html::sup(accumulate_plain_text(frame));
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::print_hline(cyng::context& ctx)
	{
		auto const el = html::hr();
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::header(cyng::context& ctx)
	{
		//	[%(("level":1),("tag":{79bf3ba0-2362-4ea5-bcb5-ed93844ac59a}),("title":{Basics}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const title = accumulate_plain_text(reader.get("title"));
		auto const tag = cyng::io::to_str(reader.get("tag"));
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0ul);

		auto const number = content_table_.add(level, uuid_gen_(), title);
		auto const header = create_section(level, tag, number + " " + title);
		//std::cout << header << std::endl;
		ctx.push(cyng::make_object(header));
	}

	std::string gen_html::create_section(std::size_t level, std::string tag, std::string title)
	{
		switch (level) {
		case 1:	return html::h1(html::id_(tag), title).to_str();
		case 2: return html::h2(html::id_(tag), title).to_str();
		case 3: return html::h3(html::id_(tag), title).to_str();
		case 4: return html::h4(html::id_(tag), title).to_str();
		case 5: return html::h5(html::id_(tag), title).to_str();
		case 6: return html::h6(html::id_(tag), title).to_str();
		default:
			break;
		}
		return title;
	}

	void gen_html::section(int level, cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const title = accumulate_plain_text(frame);
		auto const tag = uuid_gen_();
		auto const number = content_table_.add(level, tag, title);
		auto const header = create_section(level, boost::uuids::to_string(tag), number + " " + title);
		//std::cout << header << std::endl;
		ctx.push(cyng::make_object(header));
	}

	void gen_html::make_footnote(cyng::context& ctx)
	{
		//	[This,is,a,footnote,.]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const note = accumulate_plain_text(frame);
		auto const tag = name_gen_(note);
		footnotes_.emplace_back(tag, note);
		auto const idx = footnotes_.size();

		std::stringstream ss;
		ss
			<< '['
			<< idx
			<< ']'
			;
		auto const el = html::sup(html::a(html::href_("#" + boost::uuids::to_string(tag)), ss.str()));

		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::print_symbol(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::string r;
		for (auto obj : frame) {
			auto const symbol = cyng::value_cast<std::string>(obj, "");
			if (boost::algorithm::equals(symbol, "pilcrow")) {
				r.append("&para;");
			}
			else if (boost::algorithm::equals(symbol, "copyright")) {
				r.append("&copy;");
			}
			else if (boost::algorithm::equals(symbol, "registered")) {
				r.append("&reg;");
			}
			else if (boost::algorithm::iequals(symbol, "latex")) {
				auto const el = html::span("L", html::sup("A"), "T", html::sub("E"), "X");
				r += el.to_str();
			}
			else if (boost::algorithm::iequals(symbol, "celsius")) {
				r.append("&#8451;");
			}
			else if (boost::algorithm::equals(symbol, "micro")) {
				r.append("&micro;");
			}
			else if (boost::algorithm::iequals(symbol, "ohm")) {
				r.append("&ohm;");
			}
			else if (boost::algorithm::equals(symbol, "degree")) {
				r.append("&deg;");
			}
			else if (boost::algorithm::equals(symbol, "promille")) {
				r.append("&permil;");
			}
			else if (boost::algorithm::iequals(symbol, "lambda")) {
				r.append("&caret;");
			}
			else if (boost::algorithm::equals(symbol, "ellipsis")) {
				r.append("&hellip;");
			}
			else {
				r += symbol;
			}
		}
		auto const el = html::span(html::style_("font-family:Georgia, Cambria, serif;"), r);
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::print_currency(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const currency = cyng::value_cast<std::string>(frame.at(0), "");

		if (boost::algorithm::equals(currency, "euro")) {
			ctx.push(cyng::make_object("&euro;"));
		}
		else if (boost::algorithm::equals(currency, "yen")) {
			ctx.push(cyng::make_object("&yen;"));
		}
		else if (boost::algorithm::equals(currency, "pound")) {
			ctx.push(cyng::make_object("&pound;"));
		}
		//₣	&#8355;	 	franc sign
		//₤	&#8356;	 	lira symbol
		//₧	&#8359;	 	peseta sign
		//₹	&#x20B9;	 	rupee symbol
		else if (boost::algorithm::equals(currency, "rupee")) {
			ctx.push(cyng::make_object("&#x20B9;"));
		}
		//₩	&#8361;	 	won sign
		//₴	&#8372;	 	hryvnia sign
		//₯	&#8367;	 	drachma sign
		//₮	&#8366;	 	tugrik sign
		//₰	&#8368;	 	german penny sign
		//₲	&#8370;	 	guarani sign
		//₱	&#8369;	 	peso sign
		//₳	&#8371;	 	austral sign
		//₵	&#8373;	 	cedi sign
		//₭	&#8365;	 	kip sign
		//₪	&#8362;	 	new sheqel sign
		else if (boost::algorithm::equals(currency, "sheqel")) {
			ctx.push(cyng::make_object("&#8362;"));
		}
		//₫	&#8363;	 	dong sign
		else {
			ctx.push(cyng::make_object(currency));
		}
	}

	void gen_html::demo(cyng::context& ctx)
	{
		//	 [%(("red":{spiced,up}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame);
		auto const map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		ctx.push(cyng::make_object("DEMO"));
	}

	std::string replace_html_entities(std::string const& str)
	{
		//	simple state engine
		enum  {
			STATE_DEFAULT,
			STATE_LT,
			STATE_GT,
			STATE_EQ,
			STATE_MINUS,
			STATE_EXCL,	//!<	"!"
			STATE_COL,	//!<	":"
		} state = STATE_DEFAULT;

		std::string r;

		//	small optimization
		r.reserve(str.length());

		for (auto const c : str) {
			switch (state) {

			case STATE_LT:
				switch (c) {
				case '=':
					r.append("&leq;");
					break;
				case '-':
					r.append("&xlarr;");
					break;
				case '<':
					r.append("&Lt;");
					break;
				default:
					r.append("&lt;");
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_GT:
				switch (c) {
				case '=':
					r.append("&ge;");
					break;
				case '>':
					r.append("&Gt;");
					break;
				default:
					r.append("&gt;");
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_EQ:
				switch (c) {
				case '>':
					r.append("&xrArr;");
					break;
				case '=':
					r.append("&equiv;");
					break;
				default:
					r += '=';
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_MINUS:
				switch (c) {
				case '>':
					r.append("&xrarr;");
					break;
				default:
					r += '-';
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_EXCL:
				switch (c) {
				case '=':
					r.append("&ne;");
					break;
				default:
					r += '!';
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_COL:
				switch (c) {
				case ':':
					r.append("&Colon;");
					break;
				case '=':	//	:=
					r.append("&Assign;");
					break;
				default:
					r += ':';
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			default:
				switch (c) {
				case '<':
					state = STATE_LT;
					break;
				case '>':
					state = STATE_GT;
					break;
				case '=':
					state = STATE_EQ;
					break;
				case '-':
					state = STATE_MINUS;
					break;
				case '!':
					state = STATE_EXCL;
					break;
				case ':':
					state = STATE_COL;
					break;
				case '&':
					r.append("&amp;");
					break;
				case '"':
					r.append("&quot;");
					break;
				case '\'':
					r.append("&apos;");
					break;
				default:
					r += c;
					break;
				}
				break;
			}
		}

		switch (state) {

		case STATE_LT:		r += '<';	break;
		case STATE_GT:		r += '>';	break;
		case STATE_EQ:		r += '=';	break;
		case STATE_MINUS:	r += '-';	break;
		case STATE_EXCL:	r += '!';	break;
		case STATE_COL:		r += ':';	break;
		default:
			break;
		}

		return r;
	}

}


