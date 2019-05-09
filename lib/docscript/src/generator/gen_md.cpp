/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/gen_md.h>

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/numeric_cast.hpp>
#include <cyng/dom/reader.h>

#include <boost/algorithm/string.hpp>

namespace docscript
{

	gen_md::gen_md(std::vector< boost::filesystem::path > const& inc)
		: generator(inc)
	{
		register_this();
	}

	void gen_md::register_this()
	{
		generator::register_this();

		vm_.register_function("demo", 0, std::bind(&gen_md::demo, this, std::placeholders::_1));

		vm_.register_function("generate.file", 1, std::bind(&gen_md::generate_file, this, std::placeholders::_1));
		vm_.register_function("generate.meta", 1, std::bind(&gen_md::generate_meta, this, std::placeholders::_1));

		vm_.register_function("convert.numeric", 1, std::bind(&gen_md::convert_numeric, this, std::placeholders::_1));
		vm_.register_function("convert.alpha", 1, std::bind(&gen_md::convert_alpha, this, std::placeholders::_1));

		vm_.register_function("paragraph", 1, std::bind(&gen_md::paragraph, this, std::placeholders::_1));
		vm_.register_function("quote", 1, std::bind(&gen_md::quote, this, std::placeholders::_1));
		vm_.register_function("list", 1, std::bind(&gen_md::list, this, std::placeholders::_1));
		vm_.register_function("link", 1, std::bind(&gen_md::link, this, std::placeholders::_1));
		vm_.register_function("figure", 1, std::bind(&gen_md::figure, this, std::placeholders::_1));
		vm_.register_function("code", 1, std::bind(&gen_md::code, this, std::placeholders::_1));
		vm_.register_function("def", 1, std::bind(&gen_md::def, this, std::placeholders::_1));

		vm_.register_function("header", 1, std::bind(&gen_md::header, this, std::placeholders::_1));
		vm_.register_function("h1", 1, std::bind(&gen_md::section, this, 1, std::placeholders::_1));
		vm_.register_function("h2", 1, std::bind(&gen_md::section, this, 2, std::placeholders::_1));
		vm_.register_function("h3", 1, std::bind(&gen_md::section, this, 3, std::placeholders::_1));
		vm_.register_function("h4", 1, std::bind(&gen_md::section, this, 4, std::placeholders::_1));
		vm_.register_function("h5", 1, std::bind(&gen_md::section, this, 5, std::placeholders::_1));
		vm_.register_function("h6", 1, std::bind(&gen_md::section, this, 6, std::placeholders::_1));
		vm_.register_function("footnote", 1, std::bind(&gen_md::make_footnote, this, std::placeholders::_1));

		vm_.register_function("i", 1, std::bind(&gen_md::format_italic, this, std::placeholders::_1));
		vm_.register_function("b", 1, std::bind(&gen_md::format_bold, this, std::placeholders::_1));
		vm_.register_function("bold", 1, std::bind(&gen_md::format_bold, this, std::placeholders::_1));
		vm_.register_function("color", 1, std::bind(&gen_md::format_color, this, std::placeholders::_1));
	}

	void gen_md::generate_file(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const p = cyng::value_cast(frame.at(0), boost::filesystem::path());
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
			//slug();

			//
			//	write output file
			//
			auto pos = frame.begin();
			auto end = frame.end();
			emit_file(ofs, std::next(pos), end);
		}
	}

	std::ofstream& gen_md::emit_file(std::ofstream& ofs, cyng::vector_t::const_iterator pos, cyng::vector_t::const_iterator end) const
	{
		while (pos != end) {
			emit_obj(ofs, *pos);
			++pos;
		}
		emit_footnotes(ofs);
		return ofs;
	}

	std::ofstream& gen_md::emit_obj(std::ofstream& ofs, cyng::object obj) const
	{
		ofs
			<< cyng::io::to_str(obj)
			<< std::endl
			;
		return ofs;
	}

	std::ofstream& gen_md::emit_footnotes(std::ofstream& ofs) const
	{
		if (!footnotes_.empty()) {

			//
			//	horizontal line
			//
			ofs
				<< std::string(72, '-')
				<< std::endl;

			//
			//	footnotes
			//
			std::size_t idx{ 0 };
			for (auto const& note : footnotes_) {

				++idx;

				ofs
					<< '['
					<< '^'
					<< idx
					<< ']'
					<< ':'
					<< ' '
					<< note.get_note()
					<< std::endl
				;

			}
		}
		return ofs;
	}

	void gen_md::generate_meta(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const p = cyng::value_cast(frame.at(0), boost::filesystem::path());
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

		}
	}

	void gen_md::convert_numeric(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		ctx.push(frame.at(0));
	}

	void gen_md::convert_alpha(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto str = cyng::value_cast<std::string>(frame.at(0), "");

		//
		//	Special characters are:
		//	\ backslash itself
		//	` backtick
		//	* asterisk
		//	_ underscore
		//	{} curly braces
		//	[] square brackets
		//	() parentheses
		//	# hash mark
		//	+ plus sign
		//	- minus sign(hyphen)
		//	. dot
		//	! exclamation mark
		//
		replace_all(str, "\\", "\\\\");
		replace_all(str, "`", "\\`");
		replace_all(str, "*", "\\*");
		replace_all(str, "!", "\\!");
		replace_all(str, "{", "\\{");
		replace_all(str, "}", "\\}");
		replace_all(str, "[", "\\[");
		replace_all(str, "]", "\\]");
		ctx.push(cyng::make_object(str));
	}

	void gen_md::paragraph(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::stringstream ss;
		ss << std::endl;
		for (auto obj : frame) {
			ss
				<< cyng::io::to_str(obj)
				<< ' '
				;
		}
		ss << std::endl;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::quote(cyng::context& ctx)
	{
		//	[%(("q":{1,2,3}),("source":Earl Wilson),("url":https://www.brainyquote.com/quotes/quotes/e/earlwilson385998.html))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
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
		//auto const el = html::figure(html::blockquote(html::cite_(cite), quote), html::figcaption(html::cite(source)));
		ctx.push(cyng::make_object("\n> " + quote));
	}

	void gen_md::list(cyng::context& ctx)
	{
		//	[%(("items":[<p>one </p>,<p>two </p>,<p>three </p>]),("style":{lower-roman}),("type":ordered))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const type = cyng::value_cast<std::string>(reader.get("type"), "ordered");
		auto const style = cyng::value_cast<std::string>(reader.get("style"), "disc");
		cyng::vector_t items;
		items = cyng::value_cast(reader.get("items"), items);

		auto b = boost::algorithm::equals(type, "ordered") || boost::algorithm::equals(type, "ol");
		//	? html::ol(html::style_("list-style-type:" + style))
		//	: html::ul(html::style_("list-style-type:" + style));

		std::stringstream ss;
		std::size_t idx{ 0 };
		for (auto const& item : items) {
			if (b) {
				++idx;
				ss << idx << '.' << ' ';
			}
			else {
				ss << "* ";
			}
			ss
				<< accumulate_plain_text(item)
				<< std::endl;
		}

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::link(cyng::context& ctx)
	{
		//	[%(("text":{LaTeX}),("url":{https,:,//www,.,latex-project,.,org/}))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));
		std::stringstream ss;
		ss
			<< '['
			<< accumulate_plain_text(reader.get("text"))
			<< "]("
			<< cyng::io::to_str(reader.get("url"))
			<< ')'
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::figure(cyng::context& ctx)
	{
		//	 [%(("alt":{Giovanni,Bellini,,,Man,wearing,a,turban}),("caption":{Giovanni,Bellini,,,Man,wearing,a,turban}),("source":{LogoSmall,.,jpg}),("tag":{338,d542a-a4e3-4a4c-9efe-b8d3032306c3}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//ctx.push(frame.at(0));
		auto const reader = cyng::make_reader(frame.at(0));
		auto const alt = cyng::io::to_str(reader.get("alt"));
		auto const caption = cyng::io::to_str(reader.get("caption"));
		auto const source = cyng::io::to_str(reader.get("source"));
		auto const tag = cyng::io::to_str(reader.get("tag"));

		//![Tux, the Linux mascot](/assets/images/tux.png)
		std::stringstream ss;
		ss
			<< "!["
			<< caption
			<< "]("
			<< resolve_path(source).string()
			<< ")"
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::code(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const caption = accumulate_plain_text(reader.get("caption"));

		auto const source = cyng::value_cast<std::string>(reader.get("source"), "source.txt");
		auto const p = resolve_path(source);

		std::ifstream  ifs(p.string()/*, std::ios::binary*/);
		if (!ifs.is_open())
		{
			std::cerr
				<< "***error cannot open source file ["
				<< p
				<< ']'
				<< std::endl;
			ctx.push(cyng::make_object(p));
		}
		else {

			auto const language = cyng::value_cast(reader.get("language"), get_extension(p));

			std::stringstream ss;
			ss
				<< "```"
				<< language
				<< std::endl
				<< ifs.rdbuf()
				<< std::endl
				<< "```"
				<< std::endl
				;
			ifs.close();
			ctx.push(cyng::make_object(ss.str()));
		}
	}

	void gen_md::def(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		//
		//	Markdown lacks support for a definition list. But some implementations
		//	like Maruku, kramdown, dillinger and pandoc offer a syntax with a ":" at the beginning
		//	of a new line, that the following text is a definition
		//

		std::stringstream ss;
		ss << std::endl;

		cyng::param_map_t defs;
		defs = cyng::value_cast(frame.at(0), defs);
		for (auto const& def : defs) {
			ss
				<< std::endl	//	required to detect the definition at all
				<< def.first
				<< std::endl
				<< ':'
				<< ' '
				<< accumulate_plain_text(def.second)
				<< std::endl
				;
		}

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::format_italic(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object("_" + accumulate_plain_text(frame) + "_"));
	}

	void gen_md::format_bold(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object("**" + accumulate_plain_text(frame) + "**"));
	}

	void gen_md::format_color(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		//
		//	there is no support for colors in markdown
		//

		auto const reader = cyng::make_reader(frame);
		const auto map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		if (!map.empty()) {
			auto const color = map.begin()->first;
			auto const str = cyng::io::to_str(map.begin()->second);
			ctx.push(cyng::make_object(str));
		}
		else {

			ctx.push(cyng::make_object("***error in COLOR definition"));
		}
	}

	void gen_md::header(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const title = accumulate_plain_text(reader.get("title"));
		auto const tag = cyng::io::to_str(reader.get("tag"));
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0ul);

		auto const number = content_table_.add(level, uuid_gen_(), title);

		std::cout << std::string(level, '#') << " " << number << " " << title << std::endl;

		ctx.push(cyng::make_object(std::string(level, '#') + " " + number + " " + title));
	}

	void gen_md::section(int level, cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const title = accumulate_plain_text(frame);
		auto const number = content_table_.add(level, uuid_gen_(), title);

		ctx.push(cyng::make_object(std::string(level, '#') + " " + number + " " + title));

		//std::stringstream ss;
		//ss
		//	<< std::string(level, '#')
		//	<< ' '
		//	<< cyng::io::to_str(frame)
		//	<< std::endl
		//	;
		//ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::make_footnote(cyng::context& ctx)
	{
		//	[%(("level":1),("tag":{79bf3ba0-2362-4ea5-bcb5-ed93844ac59a}),("title":{Basics}))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const note = accumulate_plain_text(frame);
		auto const tag = uuid_gen_();
		footnotes_.emplace_back(footnote(tag, note));
		auto const idx = footnotes_.size();

		std::stringstream ss;
		ss
			<< '['
			<< '^'
			<< idx
			<< ']'
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::demo(cyng::context& ctx)
	{
		//	 [%(("red":{spiced,up}))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame);
		auto const map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		ctx.push(cyng::make_object("DEMO"));
	}

}


