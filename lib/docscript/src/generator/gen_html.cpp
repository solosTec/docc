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
#include "filter/sml_to_html.h"

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/numeric_cast.hpp>
#include <cyng/dom/reader.h>
#include <cyng/csv.h>
#include <cyng/io/bom.h>

#include <crypto/hash/base64.h>

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
		vm_.register_function("gallery", 1, std::bind(&gen_html::gallery, this, std::placeholders::_1));
		vm_.register_function("code", 1, std::bind(&gen_html::code, this, std::placeholders::_1));
		vm_.register_function("def", 1, std::bind(&gen_html::def, this, std::placeholders::_1));
		vm_.register_function("note", 1, std::bind(&gen_html::annotation, this, std::placeholders::_1));
		vm_.register_function("table", 1, std::bind(&gen_html::table, this, std::placeholders::_1));

		vm_.register_function("i", 1, std::bind(&gen_html::format_italic, this, std::placeholders::_1));
		vm_.register_function("b", 1, std::bind(&gen_html::format_bold, this, std::placeholders::_1));
		vm_.register_function("bold", 1, std::bind(&gen_html::format_bold, this, std::placeholders::_1));
		vm_.register_function("tt", 1, std::bind(&gen_html::format_tt, this, std::placeholders::_1));
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

			//	write BOM
			//
			cyng::write(ofs, cyng::bom::UTF8);

			//
			//	write output file
			//
			auto pos = frame.begin();
			auto end = frame.end();
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
			//<< "\t\t\tmax-width: 52rem; "
			<< "\t\t\tmax-width: 62%; "
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

			//
			//	figure
			//
			<< "\t\tfigure {"
			<< std::endl
			<< "\t\t\tmargin: 2%;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\timg {"
			<< std::endl
			//<< "\t\t\tmax-width: 95%;"
			//<< std::endl
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

			//
			//	gallery
			//
			<< "\t\tdiv.gallery {"
			<< std::endl
			<< "\t\t\tdisplay: grid;"
			<< std::endl
			<< "\t\t\tgrid-gap: 12px;"
			<< std::endl
			<< "\t\t\tbackground-color: #eee;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\tdiv.smf-svg:hover {"
			<< std::endl
			<< "\t\t\tbox-shadow: 0 0 10px #ccc;"
			<< std::endl
			<< "\t\t}"
			<< std::endl

			<< "\t\tdiv.gallery img {"
			<< std::endl
			<< "\t\t\twidth: 100%;"
			<< std::endl
			<< "\t\t\theight: auto;"
			<< std::endl
			<< "\t\t\tobject-fit: cover;"
			<< std::endl
			<< "\t\t}"
			<< std::endl
			//
			//<< "\t\tdiv.desc {"
			//<< "\t\t\tpadding: 15px;"
			//<< "\t\t\ttext-align: center;"
			//<< "\t\t}"

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
			<< std::endl
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
// 		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const type = cyng::value_cast<std::string>(reader.get("type"), "ordered");
		bool const is_ordered = (boost::algorithm::equals(type, "ordered") || boost::algorithm::equals(type, "ol"));
		auto const style = cyng::value_cast<std::string>(reader.get("style"), is_ordered ? "decimal-leading-zero" : "disc");
		cyng::vector_t items;
		items = cyng::value_cast(reader.get("items"), items);

		if (is_ordered) {
			bool const is_valid =  boost::algorithm::equals("decimal", style)
			|| boost::algorithm::equals("cjk-decimal", style)
			|| boost::algorithm::equals("decimal-leading-zero", style)
			|| boost::algorithm::equals("lower-roman", style)
			|| boost::algorithm::equals("upper-roman", style)
			|| boost::algorithm::equals("lower-greek", style)
			|| boost::algorithm::equals("lower-alpha", style)
			|| boost::algorithm::equals("lower-latin", style)
			|| boost::algorithm::equals("upper-alpha", style)
			|| boost::algorithm::equals("upper-latin", style)
			|| boost::algorithm::equals("arabic-indic", style)
			|| boost::algorithm::equals("armenian", style)
			|| boost::algorithm::equals("bengali", style)
			|| boost::algorithm::equals("cambodian ", style)
			|| boost::algorithm::equals("cjk-earthly-branch", style)
			|| boost::algorithm::equals("cjk-heavenly-stem", style)
			|| boost::algorithm::equals("cjk-ideographic", style)
			|| boost::algorithm::equals("devanagari", style)
			|| boost::algorithm::equals("ethiopic-numeric", style)
			|| boost::algorithm::equals("georgian", style)
			|| boost::algorithm::equals("gujarati", style)
			|| boost::algorithm::equals("gurmukhi", style)
			|| boost::algorithm::equals("hebrew", style);
			
			if (!is_valid) {
				std::cerr
					<< "***warning: ["
					<< style
					<< "] is an unknwon style for ordered lists "
					<< std::endl;
				
			}
		}
		else {
			bool const is_valid =  boost::algorithm::equals("disc", style)
			|| boost::algorithm::equals("circle", style)
			|| boost::algorithm::equals("square", style);
			
			if (!is_valid) {
				std::cerr
					<< "***warning: ["
					<< style
					<< "] is an unknwon style for unorderd lists "
					<< std::endl;
				
			}			
		}
		
		auto el = (is_ordered)
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

	std::string gen_html::compute_title(std::string tag, std::string caption)
	{
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

		return ss.str();
	}

	void gen_html::figure(cyng::context& ctx)
	{
		//	  [%(("alt":{Giovanni,Bellini,,,Man,who,plows,daisies}),("caption":{Daisies,in,the,rain}),("source":example.jpg),("tag":338d542a-a4e3-4a4c-9efe-b8d3032306c3),("width":0.5))]
		auto const frame = ctx.get_frame();
// 		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const alt = accumulate_plain_text(reader.get("alt"));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const source = cyng::io::to_str(reader.get("source"));
		auto const tag = cyng::value_cast(reader.get("tag"), source);
		//	generate unique tag for SVG
		auto const id = cyng::value_cast(reader.get("id"), boost::uuids::to_string(uuid_gen_()));
		auto const width = cyng::value_cast(reader.get("width"), 1.0);

		if (width > 1.0 || width < 0.01) {
			std::cerr
				<< "***warning: unusual scaling factor ["
				<< width
				<< "] for figure: "
				<< source
				<< std::endl;
		}

		auto const p = resolve_path(source);
		if (boost::filesystem::exists(p) && boost::filesystem::is_regular(p)) {

			auto const title = compute_title(tag, caption);

			//
			//	generate <figure> tag
			//
			auto const el = make_figure(p
				, tag
				, id
				, width
				, caption
				, title
				, alt);
			ctx.push(cyng::make_object(el.to_str()));

		}
		else {

			std::cerr
				<< "***error cannot open figure file ["
				<< source
				<< "]"
				<< std::endl;

			auto const el = html::h2(html::id_(tag), "cannot open file [" + source + "]", html::title_(caption));
			ctx.push(cyng::make_object(el.to_str()));
		}
	}

	void gen_html::gallery(cyng::context& ctx)
	{
		//	[%(("caption":{Image,Gallery}),
		//	("images":[
		//		[source,:,logo.svg,,,caption,:,",solosTec,Logo,",,,alt,:,",solosTec,Logo,",,,tag,:,9d0dd0fd-de54-49cc-86b0-64fe35b6d5a5],
		//		[source,:,example.jpg,,,caption,:,",Daisies,in,the,rain,",,,alt,:,",Giovanni,Bellini,,,Man,who,plows,daisies,",,,tag,:,338d542a-a4e3-4a4c-9efe-b8d3032306c3],
		//		[source,:,frog.jpg,,,caption,:,",Daisies,in,the,rain,",,,alt,:,",Giovanni,Bellini,,,Man,who,plows,daisies,",,,tag,:,338d542a-a4e3-4a4c-9efe-b8d3032306c3]
		//	]))]
		auto const frame = ctx.get_frame();
// 		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const tag = cyng::value_cast(reader.get("tag"), boost::uuids::to_string(uuid_gen_()));

		cyng::vector_t vec;
		vec = cyng::value_cast(reader.get("images"), vec);

		auto const size = cyng::numeric_cast<std::size_t>(reader.get("size"), vec.size());

		auto div = html::div(html::id_(tag));
		div += html::h4(caption);

		if (!vec.empty()) {

			//"grid-template-columns: repeat(N, 1fr)"
			std::ostringstream ss;
			ss << "grid-template-columns: repeat(";

			if ((size < vec.size()) && (size != 0u)) {
				ss << size;
			}
			else {
				ss << vec.size();
			}
			ss << ", 1fr)";

			auto grid = html::div(html::id_(tag), html::class_("gallery"), html::style_(ss.str()));

			for (auto pos = 0u; pos < vec.size(); ++pos) {
				auto const alt = accumulate_plain_text(reader["images"][pos].get("alt"));
				auto const caption = accumulate_plain_text(reader["images"][pos].get("caption"));
				auto const source = cyng::io::to_str(reader["images"][pos].get("source"));
				auto const tag = cyng::value_cast(reader["images"][pos].get("tag"), source);
				auto const width = cyng::value_cast(reader["images"][pos].get("width"), 1.0);
				auto const id = cyng::value_cast(reader["images"][pos].get("id"), boost::uuids::to_string(uuid_gen_()));

				auto const p = resolve_path(source);
				if (boost::filesystem::exists(p) && boost::filesystem::is_regular(p)) {

					auto el = make_figure(p
						, tag
						, id
						, width
						, caption
						, compute_title(tag, caption)
						, alt);

					grid += std::move(el);
				}
				else {

					std::cerr
						<< "***error cannot open figure file ["
						<< source
						<< "]"
						<< std::endl;

				}
			}

			//
			//	ToDo: improve CSS
			//
			div += std::move(grid);

			ctx.push(cyng::make_object(div.to_str()));
		}
		else {
			std::cerr
				<< "***error: gallery ["
				<< caption
				<< "] contains no images"
				<< std::endl;
			ctx.push(cyng::make_object("[" + caption + "]"));
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
			else if (boost::algorithm::equals(language, "sml")) {

				// binary mode required
				std::ifstream  ifs(p.string(), std::ios::binary);
				ifs.unsetf(std::ios::skipws);
				ss << "<pre class=\"docscript-pre-binary\">" << std::endl;
				cyng::buffer_t const inp((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
				try {
					sml_to_html filter(line_numbers, uuid_gen_());
					filter.convert(ss, inp);
				}
				catch (std::exception const& ex) {

					ss
						<< "***error: converting SML file "
						<< p
						<< " to html failed with error: "
						<< ex.what()
						<< std::endl;
				}
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
				<< source
				<< "] does not exist or is not a regular file"
				<< std::endl;
				auto const el = html::strong(source);
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
		auto el = html::b(accumulate_plain_text(frame));
		ctx.push(cyng::make_object(el.to_str()));
	}

	void gen_html::format_tt(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		auto el = html::tt(accumulate_plain_text(frame));
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
			else if (boost::algorithm::equals(symbol, "multiply")) {
				r.append("&times;");
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

	html::node make_figure(boost::filesystem::path p
		, std::string tag
		, std::string id
		, double width
		, std::string caption
		, std::string title
		, std::string alt)
	{
		auto const max_width = std::to_string(width * 100.0) + "%";
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
						svg.prepend_attribute("width") = max_width.c_str();
					}
					else {
						width.set_value(max_width.c_str());
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
                auto const src = ss.str();
				return html::figure(html::id_(tag), html::div(html::class_("smf-svg"), src), html::figcaption(title));
			}
			else {
				std::cerr << "SVG [" << p << "] parsed with errors, attr value: [" << doc.child("node").attribute("attr").value() << "]\n";
				std::cerr << "Error description: " << result.description() << "\n";
				std::cerr << "Error offset: " << result.offset << " (error at [..." << result.offset << "]\n\n";
				//ctx.push(cyng::make_object(result.description()));
				return html::h2(html::id_(tag), result.description(), html::title_(title));
			}
		}

		//
		//	base64 encoded images
		//
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
        
//         std::cout << std::endl;
//         std::cout << "tag: " << tag << ", size: " << tag.size() << std::endl;
//         std::cout << "id: " << id << ", size: " << id.size() << std::endl;
//         std::cout << "alt: " << alt << ", size: " << alt.size() << std::endl;
//         std::cout << std::endl;
//         std::cout << std::endl;

        
		return html::figure(html::id_(id), html::img(html::alt_(alt), html::title_(caption), html::class_("docscript-img"), html::style_("max-width: " + max_width), html::src_("data:image/" + ext + ";base64," + cyng::crypto::base64_encode(buffer.data(), buffer.size()))), html::figcaption(title));

	}


}


