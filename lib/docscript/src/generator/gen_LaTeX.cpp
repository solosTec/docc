/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/gen_LaTeX.h>

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/numeric_cast.hpp>
#include <cyng/dom/reader.h>

#include <boost/algorithm/string.hpp>

namespace docscript
{

	gen_latex::gen_latex(std::vector< boost::filesystem::path > const& inc)
		: generator(inc)
	{
		register_this();
	}

	void gen_latex::register_this()
	{
		generator::register_this();

		vm_.register_function("demo", 0, std::bind(&gen_latex::demo, this, std::placeholders::_1));

		vm_.register_function("generate.file", 1, std::bind(&gen_latex::generate_file, this, std::placeholders::_1));
		vm_.register_function("generate.meta", 1, std::bind(&gen_latex::generate_meta, this, std::placeholders::_1));

		vm_.register_function("convert.numeric", 1, std::bind(&gen_latex::convert_numeric, this, std::placeholders::_1));
		vm_.register_function("convert.alpha", 1, std::bind(&gen_latex::convert_alpha, this, std::placeholders::_1));

		vm_.register_function("paragraph", 1, std::bind(&gen_latex::paragraph, this, std::placeholders::_1));
		vm_.register_function("quote", 1, std::bind(&gen_latex::quote, this, std::placeholders::_1));
		vm_.register_function("list", 1, std::bind(&gen_latex::list, this, std::placeholders::_1));
		vm_.register_function("link", 1, std::bind(&gen_latex::link, this, std::placeholders::_1));
		vm_.register_function("figure", 1, std::bind(&gen_latex::figure, this, std::placeholders::_1));
		vm_.register_function("code", 1, std::bind(&gen_latex::code, this, std::placeholders::_1));
		vm_.register_function("def", 1, std::bind(&gen_latex::def, this, std::placeholders::_1));

		vm_.register_function("i", 1, std::bind(&gen_latex::format_italic, this, std::placeholders::_1));
		vm_.register_function("b", 1, std::bind(&gen_latex::format_bold, this, std::placeholders::_1));
		vm_.register_function("bold", 1, std::bind(&gen_latex::format_bold, this, std::placeholders::_1));
		vm_.register_function("color", 1, std::bind(&gen_latex::format_color, this, std::placeholders::_1));

		vm_.register_function("header", 1, std::bind(&gen_latex::header, this, std::placeholders::_1));
		vm_.register_function("h1", 1, std::bind(&gen_latex::section, this, 1, std::placeholders::_1));
		vm_.register_function("h2", 1, std::bind(&gen_latex::section, this, 2, std::placeholders::_1));
		vm_.register_function("h3", 1, std::bind(&gen_latex::section, this, 3, std::placeholders::_1));
		vm_.register_function("h4", 1, std::bind(&gen_latex::section, this, 4, std::placeholders::_1));
		vm_.register_function("h5", 1, std::bind(&gen_latex::section, this, 5, std::placeholders::_1));
		vm_.register_function("h6", 1, std::bind(&gen_latex::section, this, 6, std::placeholders::_1));
		vm_.register_function("footnote", 1, std::bind(&gen_latex::make_footnote, this, std::placeholders::_1));

	}

	void gen_latex::generate_file(cyng::context& ctx)
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
			//emit_file(ofs, pos, end);
			emit_file(ofs, std::next(pos), end);
		}
	}

	std::ofstream& gen_latex::emit_file(std::ofstream& ofs, cyng::vector_t::const_iterator pos, cyng::vector_t::const_iterator end) const
	{
		emit_class(ofs);
		emit_title(ofs);
		return emit_document(ofs, pos, end);
	}

	std::ofstream& gen_latex::emit_class(std::ofstream& ofs) const
	{
		ofs
			<< "\\documentclass[ 10pt, a4paper ]{ scrartcl }"
			<< std::endl
			<< "%\tmeta"
			<< std::endl
			<< build_cmd("usepackage", "utf8", "inputenc")
			<< std::endl
			<< build_cmd("usepackage", "official", "eurosym")
			<< std::endl
			<< build_cmd("usepackage", "listings")
			<< std::endl
			;
		return ofs;
	}

	std::ofstream& gen_latex::emit_title(std::ofstream& ofs) const
	{
		ofs
			<< "\\title{TITLE}"
			<< std::endl
			<< "\\author{AUTHOR}"
			<< std::endl
			<< "\\date{}"
			<< std::endl
			;
		return ofs;
	}

	std::ofstream& gen_latex::emit_document(std::ofstream& ofs, cyng::vector_t::const_iterator pos, cyng::vector_t::const_iterator end) const
	{
		ofs
			<< std::endl
			<< "%\tdocument"
			<< std::endl
			<< "\\begin{document}"
			<< std::endl
			<< "\\maketitle"
			<< std::endl
			<< "\\setcounter{ tocdepth }{3}"
			<< std::endl
			<< "\\tableofcontents"
			<< std::endl
			<< "\\listoffigures"
			<< std::endl
			<< "\\listoftables"
			<< std::endl
			<< std::endl
			;

		while (pos != end) {
			emit_document(ofs, *pos);
			++pos;
		}

		ofs
			<< "\\end{document}"
			<< std::endl
			;
		return ofs;
	}

	std::ofstream& gen_latex::emit_document(std::ofstream& ofs, cyng::object obj) const
	{
		ofs
			<< cyng::io::to_str(obj)
			<< std::endl
			;
		return ofs;
	}

	void gen_latex::generate_meta(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const p = cyng::value_cast(frame.at(0), boost::filesystem::path());
	}

	void gen_latex::convert_numeric(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		ctx.push(cyng::make_object("$" + accumulate_plain_text(frame) + "$"));
	}

	void gen_latex::convert_alpha(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto str = cyng::value_cast<std::string>(frame.at(0), "");

		//
		//	escape 
		//	\&     \$     \%     \#     \_    \{     \}
		//
		replace_all(str, "&", "\\&");
		replace_all(str, "$", "\\$");
		replace_all(str, "%", "\\%");
		replace_all(str, "#", "\\#");
		replace_all(str, "_", "\\_");
		replace_all(str, "{", "\\{");
		replace_all(str, "}", "\\}");
		ctx.push(cyng::make_object(str));
	}

	void gen_latex::print_symbol(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const symbol = cyng::value_cast<std::string>(frame.at(0), "");
		if (boost::algorithm::equals(symbol, "pilgrow")) {
			ctx.push(cyng::make_object("\\P"));
		}
		else if (boost::algorithm::equals(symbol, "copyright")) {
			ctx.push(cyng::make_object("\\textcopyright"));
		}
		else if (boost::algorithm::equals(symbol, "registered")) {
			ctx.push(cyng::make_object("\\textregistered"));
		}
		else {
			ctx.push(cyng::make_object(symbol));
		}
	}

	void gen_latex::print_currency(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const currency = cyng::value_cast<std::string>(frame.at(0), "");

		if (boost::algorithm::equals(currency, "euro")) {
			//	"\usepackage[official]{eurosym}"
			ctx.push(cyng::make_object("\\euro"));
		}
		else if (boost::algorithm::equals(currency, "yen")) {
			ctx.push(cyng::make_object(u8"¥"));
		}
		else if (boost::algorithm::equals(currency, "pound")) {
			ctx.push(cyng::make_object("\\pounds"));
		}
		else {
			ctx.push(cyng::make_object(currency));
		}
	}



	void gen_latex::paragraph(cyng::context& ctx)
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

	void gen_latex::quote(cyng::context& ctx)
	{
		//	[%(("q":{1,2,3}),("source":Earl Wilson),("url":https://www.brainyquote.com/quotes/quotes/e/earlwilson385998.html))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));

		//	Attribution for the quotation, if any, must be placed outside the blockquote element.
		//	structure:
		//	\begin{quotation}
		//	
		//	\end{ quotation }
		//
		auto const cite = cyng::value_cast<std::string>(reader.get("url"), "");
		auto const source = cyng::value_cast<std::string>(reader.get("source"), "");
		auto const quote = accumulate_plain_text(reader.get("q"));

		std::stringstream ss;
		ss
			<< std::endl
			<< build_cmd("begin", "quotation")
			<< std::endl
			<< quote
			<< std::endl
			<< build_cmd("end", "quotation")
			<< std::endl
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_latex::list(cyng::context& ctx)
	{
		//	[%(("items":[<p>one </p>,<p>two </p>,<p>three </p>]),("style":{lower-roman}),("type":ordered))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const type = cyng::value_cast<std::string>(reader.get("type"), "ordered");
		auto const style = cyng::value_cast<std::string>(reader.get("style"), "disc");
		cyng::vector_t items;
		items = cyng::value_cast(reader.get("items"), items);

		std::stringstream ss;

		auto const list = (boost::algorithm::equals(type, "ordered") || boost::algorithm::equals(type, "ol"))
			? "enumerate"
			: "itemize"
			;
		ss
			<< build_cmd("begin", list)
			<< std::endl
			;

		for (auto const& item : items) {
			ss
				<< "\\item "
				<< accumulate_plain_text(item)
				<< std::endl;
		}

		ss
			<< build_cmd("end", list)
			<< std::endl
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_latex::link(cyng::context& ctx)
	{
		//	[%(("text":{LaTeX}),("url":{https,:,//www,.,latex-project,.,org/}))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));

		//	\href{URL}{TEXT}
		std::stringstream ss;
		ss
			<< "\\href"
			<< '{'
			<< cyng::io::to_str(reader.get("url"))
			<< '}'
			<< '{'
			<< cyng::io::to_str(reader.get("text"))
			<< '}'
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_latex::figure(cyng::context& ctx)
	{
		//	 [%(("alt":{Giovanni,Bellini,,,Man,wearing,a,turban}),("caption":{Giovanni,Bellini,,,Man,wearing,a,turban}),("source":{LogoSmall,.,jpg}),("tag":{338,d542a-a4e3-4a4c-9efe-b8d3032306c3}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));
		auto const alt = accumulate_plain_text(reader.get("alt"));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const source = cyng::io::to_str(reader.get("source"));
		auto const tag = cyng::io::to_str(reader.get("tag"));

		std::stringstream ss;
		ss
			<< "\\begin{figure}[hbtp]"
			<< std::endl
			<< "%\\centering"
			<< std::endl
			<< build_cmd("includegraphics", "width=1.0\\textwidth", resolve_path(source).string())
			<< std::endl
			<< build_cmd("caption", caption)
			<< std::endl
			<< build_cmd("end", "figure")
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_latex::code(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const line_numbers = value_cast(reader.get("linenumbers"), false);

		auto const source = cyng::value_cast<std::string>(reader.get("source"), "source.txt");
		auto const p = resolve_path(source);

		std::ifstream  ifs(p.string());
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

			//\begin{ lstlisting }[language = Python]
			//	numbers=left, numbers=none
			std::stringstream ss;
			ss
				<< build_cmd_alt("begin", "lstlisting", ("language=" + language + ", caption=" + caption + ", numbers=" + (line_numbers ? "left" : "none")))
				<< std::endl
				<< ifs.rdbuf()
				<< std::endl
				<< build_cmd("end", "lstlisting")
				<< std::endl
				;
			ifs.close();
			ctx.push(cyng::make_object(ss.str()));
		}
	}

	void gen_latex::def(cyng::context& ctx)
	{
		//	[%(("abstract syntax":[Specification,of,a,structure]),("big endian":[A,byte,order,sequence]))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::stringstream ss;
		ss
			<< build_cmd("begin", "description")
			<< std::endl
			;

		cyng::param_map_t defs;
		defs = cyng::value_cast(frame.at(0), defs);
		for (auto const& def : defs) {
			ss
				<< "\\item["
				<< def.first
				<< "] "
				<< accumulate_plain_text(def.second)
				<< std::endl
				;
		}

		ss
			<< build_cmd("end", "description")
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_latex::format_italic(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(build_cmd("emph", accumulate_plain_text(frame))));
	}

	void gen_latex::format_bold(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(build_cmd("textbf", accumulate_plain_text(frame))));
	}

	void gen_latex::format_color(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame);
		const auto map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		if (!map.empty()) {
			auto const color = map.begin()->first;
			auto const str = cyng::io::to_str(map.begin()->second);
			ctx.push(cyng::make_object("***error in COLOR definition"));
		}
		else {

			ctx.push(cyng::make_object("***error in COLOR definition"));
		}
	}

	void gen_latex::header(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const title = accumulate_plain_text(reader.get("title"));
		auto const tag = cyng::io::to_str(reader.get("tag"));
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0ul);

		auto const header = create_section(level, tag, title);
		ctx.push(cyng::make_object(header));
	}

	std::string gen_latex::create_section(std::size_t level, std::string tag, std::string title)
	{
		switch (level) {
		case 1:	return build_cmd("chapter", title);
		case 2: return build_cmd("section", title);
		case 3: return build_cmd("subsection", title);
		case 4: return build_cmd("subsubsection", title);
		default:
			break;
		}

		return title;
	}

	void gen_latex::section(int level, cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const header = create_section(level, "TAG", accumulate_plain_text(frame));
		ctx.push(cyng::make_object(header));
	}

	void gen_latex::make_footnote(cyng::context& ctx)
	{
		//	[%(("level":1),("tag":{79bf3ba0-2362-4ea5-bcb5-ed93844ac59a}),("title":{Basics}))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(build_cmd("footnote", accumulate_plain_text(frame))));
	}

	void gen_latex::demo(cyng::context& ctx)
	{
		//	 [%(("red":{spiced,up}))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame);
		auto const map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		ctx.push(cyng::make_object("DEMO"));
	}

	std::string build_cmd(std::string cmd, std::string param)
	{
		return "\\" + cmd + "{" + param + "}";
	}

	std::string build_cmd(std::string cmd, std::string attr, std::string param)
	{
		return "\\" + cmd + "[" + attr + "]" + "{" + param + "}";
	}

	std::string build_cmd_alt(std::string cmd, std::string param, std::string attr)
	{
		return build_cmd(cmd, param) + "[" + attr + "]";
	}

}


