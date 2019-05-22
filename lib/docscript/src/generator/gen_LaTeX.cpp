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
#include <boost/uuid/uuid_io.hpp>

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

		vm_.register_function("hline", 0, std::bind(&gen_latex::print_hline, this, std::placeholders::_1));

		vm_.register_function("convert.numeric", 1, std::bind(&gen_latex::convert_numeric, this, std::placeholders::_1));
		vm_.register_function("convert.alpha", 1, std::bind(&gen_latex::convert_alpha, this, std::placeholders::_1));

		vm_.register_function("paragraph", 1, std::bind(&gen_latex::paragraph, this, std::placeholders::_1));
		vm_.register_function("abstract", 1, std::bind(&gen_latex::abstract, this, std::placeholders::_1));
		vm_.register_function("quote", 1, std::bind(&gen_latex::quote, this, std::placeholders::_1));
		vm_.register_function("list", 1, std::bind(&gen_latex::list, this, std::placeholders::_1));
		vm_.register_function("link", 1, std::bind(&gen_latex::link, this, std::placeholders::_1));
		vm_.register_function("figure", 1, std::bind(&gen_latex::figure, this, std::placeholders::_1));
		vm_.register_function("code", 1, std::bind(&gen_latex::code, this, std::placeholders::_1));
		vm_.register_function("def", 1, std::bind(&gen_latex::def, this, std::placeholders::_1));
		vm_.register_function("note", 1, std::bind(&gen_latex::annotation, this, std::placeholders::_1));

		vm_.register_function("i", 1, std::bind(&gen_latex::format_italic, this, std::placeholders::_1));
		vm_.register_function("b", 1, std::bind(&gen_latex::format_bold, this, std::placeholders::_1));
		vm_.register_function("bold", 1, std::bind(&gen_latex::format_bold, this, std::placeholders::_1));
		vm_.register_function("color", 1, std::bind(&gen_latex::format_color, this, std::placeholders::_1));
		vm_.register_function("sub", 1, std::bind(&gen_latex::format_sub, this, std::placeholders::_1));
		vm_.register_function("sup", 1, std::bind(&gen_latex::format_sup, this, std::placeholders::_1));

		vm_.register_function("header", 1, std::bind(&gen_latex::header, this, std::placeholders::_1));
		vm_.register_function("h1", 1, std::bind(&gen_latex::section, this, 1, std::placeholders::_1));
		vm_.register_function("h2", 1, std::bind(&gen_latex::section, this, 2, std::placeholders::_1));
		vm_.register_function("h3", 1, std::bind(&gen_latex::section, this, 3, std::placeholders::_1));
		vm_.register_function("h4", 1, std::bind(&gen_latex::section, this, 4, std::placeholders::_1));
		vm_.register_function("h5", 1, std::bind(&gen_latex::section, this, 5, std::placeholders::_1));
		vm_.register_function("h6", 1, std::bind(&gen_latex::section, this, 6, std::placeholders::_1));
		vm_.register_function("footnote", 1, std::bind(&gen_latex::make_footnote, this, std::placeholders::_1));
		vm_.register_function("ref", 1, std::bind(&gen_latex::make_ref, this, std::placeholders::_1));

	}

	void gen_latex::generate_file(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto p = cyng::value_cast(frame.at(0), boost::filesystem::path());
		if (boost::filesystem::is_directory(p)) {
			p = p / "out.tex";
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
//		"og:type"
		std::string const type = (is_report())
			? "scrreprt"
			 : "scrartcl"
			 ;

//		\usepackage[ngerman]{babel}
//		\usepackage[english]{babel}
		ofs
			<< "\\documentclass[10pt,a4paper]{"
			<< type
			<< "}"
			<< std::endl
			<< "%\tmeta"
			<< std::endl
			<< build_cmd("usepackage", "utf8", "inputenc")
			<< std::endl
			<< build_cmd("usepackage", get_babel_language(), "babel")
			<< std::endl
			<< "%uncomment next two lines to use sans-serif fonts"
			<< std::endl
			<< "%\\usepackage{helvet}"
			<< std::endl
			<< "%\\renewcommand{\\familydefault}{\\sfdefault}"
			<< std::endl
			<< build_cmd("usepackage", "official", "eurosym")
			<< std::endl
			<< build_cmd("usepackage", "stix")
			<< std::endl
			<< build_cmd("usepackage", "amsmath")//v2.14
			<< std::endl
			<< build_cmd("usepackage", "amssymb") // v3.01
			<< std::endl
			<< build_cmd("usepackage", "amsfonts") // v3.01
			<< std::endl
			<< build_cmd("usepackage", "latexsym")
			<< std::endl
			<< build_cmd("usepackage", "hyperref")
			<< std::endl
			<< build_cmd("usepackage", "graphicx")	// Required for including images
			<< std::endl
			<< build_cmd("hypersetup", "colorlinks=true, linkcolor=blue, filecolor=magenta, urlcolor=brown")
			<< std::endl
			<< build_cmd("usepackage", "listings")
			<< std::endl
			<< build_cmd("lstset", "basicstyle=\\small\\ttfamily,breaklines=true")
			<< std::endl
			;
		return ofs;

	}

	std::ofstream& gen_latex::emit_title(std::ofstream& ofs) const
	{
		auto const reader = cyng::make_reader(meta_);
// 		std::cout << "META: " << cyng::io::to_str(meta_) << std::endl;
		
		auto const title = accumulate_plain_text(reader.get("title"));
		auto const author = accumulate_plain_text(reader.get("author"));
		
		ofs
			<< "\\title{"
			<< title
			<< "}"
			<< std::endl
			<< "\\author{"
			<< author
			<< "}"
			<< std::endl
			<< "\\date{"
			<< cyng::io::to_str(reader.get("last-write-time"))
			<< "}"
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
			<< "\\setcounter{tocdepth}{3}"
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
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const p = cyng::value_cast(frame.at(0), boost::filesystem::path());
	}

	void gen_latex::convert_numeric(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		ctx.push(cyng::make_object("$" + accumulate_plain_text(frame) + "$"));
	}

	void gen_latex::convert_alpha(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const str = cyng::value_cast<std::string>(frame.at(0), "");

		ctx.push(cyng::make_object(replace_latex_entities(str)));
	}

	void gen_latex::print_symbol(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::string r;
		for (auto obj : frame) {

			auto const symbol = cyng::value_cast<std::string>(obj, "");

			if (boost::algorithm::equals(symbol, "pilgrow")) {
				r.append("\\P");
			}
			else if (boost::algorithm::equals(symbol, "copyright")) {
				r.append("\\textcopyright");
			}
			else if (boost::algorithm::equals(symbol, "copyleft")) {
				r.append("\\textcopyleft");
			}
			else if (boost::algorithm::equals(symbol, "registered")) {
				r.append("\\textregistered");
			}
			else if (boost::algorithm::iequals(symbol, "latex")) {
				r.append("\\LaTeX");
			}
			else if (boost::algorithm::iequals(symbol, "celsius")) {
				r.append("\\celsius");
			}
			else if (boost::algorithm::equals(symbol, "micro")) {
				r.append("\\micro");
			}
			else if (boost::algorithm::iequals(symbol, "ohm")) {
				r.append("\\ohm");
			}
			else if (boost::algorithm::equals(symbol, "degree")) {
				r.append("\\degree");
			}
			else if (boost::algorithm::equals(symbol, "promille")) {
				r.append("\\perthousand");
			}
			else if (boost::algorithm::iequals(symbol, "lambda")) {
				r.append("\\lambda");
			}
			else {
				r.append(symbol);
			}
		}
		ctx.push(cyng::make_object(r));
	}

	void gen_latex::print_currency(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const currency = cyng::value_cast<std::string>(frame.at(0), "");

		if (boost::algorithm::equals(currency, "euro")) {
			//	"\usepackage[official]{eurosym}"
			ctx.push(cyng::make_object("\\euro"));
		}
		else if (boost::algorithm::equals(currency, "yen")) {
			ctx.push(cyng::make_object(u8"�"));
		}
		else if (boost::algorithm::equals(currency, "pound")) {
			ctx.push(cyng::make_object("\\pounds"));
		}
		else {
			ctx.push(cyng::make_object(currency));
		}
	}

	void gen_latex::print_hline(cyng::context& ctx)
	{
		//auto const frame = ctx.get_frame();
		ctx.push(cyng::make_object("\\hline"));

		//
		//	other options are:
		//	\rule
		//	\line
		//	\dotfill
		//	\hrulefill
		//
	}

	void gen_latex::paragraph(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::string par = accumulate_plain_text(frame);
		ctx.push(cyng::make_object(par));
	}

	void gen_latex::abstract(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
// 		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));

		auto const title = cyng::value_cast<std::string>(reader.get("title"), "Abstract");
		auto const text = accumulate_plain_text(reader.get("text"));
		
		std::stringstream ss;
		ss
			<< std::endl
			//	This section will not appear in the table of contents due to the star 
			<< build_cmd("section*", "Abstract")
			<< std::endl
			<< text
			<< std::endl
			<< "\\newpage"
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));

	}
	
	void gen_latex::quote(cyng::context& ctx)
	{
		//	[%(("q":{1,2,3}),("source":Earl Wilson),("url":https://www.brainyquote.com/quotes/quotes/e/earlwilson385998.html))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
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
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

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
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
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
			<< std::endl
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
		auto const language = cleanup_language(cyng::value_cast(reader.get("language"), get_extension(p)));

		if (boost::filesystem::exists(p) && boost::filesystem::is_regular(p)) {

			std::ifstream  ifs(p.string());
			if (is_language_supported(language)) {

				//
				//	use listings package
				//
				std::stringstream ss;
				ss
					<< build_cmd_alt("begin", "lstlisting", ("language=" + language + ", caption=" + caption + ", numbers=" + (line_numbers ? "left" : "none")))
					<< std::endl
					<< ifs.rdbuf()
					<< std::endl
					<< build_cmd("end", "lstlisting")
					<< std::endl
					;
				ctx.push(cyng::make_object(ss.str()));
			}
			else {

				//
				//	use verbatim
				//
				std::stringstream ss;
				ss
					<< build_cmd("begin", "verbatim")
					<< std::endl
					<< ifs.rdbuf()
					<< std::endl
					<< build_cmd("end", "verbatim")
					<< std::endl
					;
				ctx.push(cyng::make_object(ss.str()));
			}
		}
		else {
			std::cerr
				<< "***error ["
				<< p
				<< "] does not exist or is not a regular file"
				<< std::endl;
			ctx.push(cyng::make_object(p));

		}
	}

	void gen_latex::def(cyng::context& ctx)
	{
		//	[%(("abstract syntax":[Specification,of,a,structure]),("big endian":[A,byte,order,sequence]))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
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
				//	definition item must be cleaned up
				<< replace_latex_entities(def.first)
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

	void gen_latex::annotation(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		std::stringstream ss;
		ss
			<< build_cmd("marginpar", accumulate_plain_text(frame))
			<< std::endl
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_latex::make_ref(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(cyng::io::to_str(frame)));
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
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(build_cmd("textbf", accumulate_plain_text(frame))));
	}

	void gen_latex::format_color(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

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

	void gen_latex::format_sub(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//	_{...}
		ctx.push(cyng::make_object(build_cmd("_", accumulate_plain_text(frame))));
	}

	void gen_latex::format_sup(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//	^{...}
		ctx.push(cyng::make_object(build_cmd("^", accumulate_plain_text(frame))));
	}

	void gen_latex::header(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const title = accumulate_plain_text(reader.get("title"));
		auto const tag = cyng::io::to_str(reader.get("tag"));
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0ul);

		auto const header = create_section(level, tag, title);
		ctx.push(cyng::make_object(header));
	}

	std::string gen_latex::create_section(std::size_t level, std::string tag, std::string title)
	{
		auto const label = build_cmd("label", boost::uuids::to_string(name_gen_(tag)));

		if (is_report()) {
			switch (level) {
			case 1:	return build_cmd("chapter", title) + label;
			case 2: return build_cmd("section", title) + label;
			case 3: return build_cmd("subsection", title) + label;
			case 4: return build_cmd("subsubsection", title) + label;
			case 5: return build_cmd("paragraph", title) + label;
			case 6: return build_cmd("subparagraph", title) + label;
			default:
				break;
			}
		}
		else {

			//	chapter is not supported by documentclass scrartcl
			switch (level) {
			case 1:	return build_cmd("section", title) + label;
			case 2: return build_cmd("subsection", title) + label;
			case 3: return build_cmd("subsubsection", title) + label;
			case 4: return build_cmd("paragraph", title) + label;
			case 5: return build_cmd("subparagraph", title) + label;
			default:
				break;
			}
		}

		return title + label;
	}

	void gen_latex::section(int level, cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const title = accumulate_plain_text(frame);
		auto const header = create_section(level, title, title);
		ctx.push(cyng::make_object(header));
	}

	void gen_latex::make_footnote(cyng::context& ctx)
	{
		//	[%(("level":1),("tag":{79bf3ba0-2362-4ea5-bcb5-ed93844ac59a}),("title":{Basics}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(build_cmd("footnote", accumulate_plain_text(frame))));
	}

	void gen_latex::demo(cyng::context& ctx)
	{
		//	 [%(("red":{spiced,up}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame);
		auto const map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		ctx.push(cyng::make_object("DEMO"));
	}

	std::string gen_latex::get_babel_language() const
	{
		switch(get_language_code()) {
		case cyng::io::LC_ES: return "spanish";
		case cyng::io::LC_SV: return "swedish";
		case cyng::io::LC_PT: return "portugese";
		case cyng::io::LC_DE: return "ngerman";
		case cyng::io::LC_BG: return "bulgarian";
		case cyng::io::LC_RU: return "russian";
		case cyng::io::LC_UK: return "ukrainian";
		case cyng::io::LC_CS: return "czech";
		case cyng::io::LC_FI: return "finnish";
		case cyng::io::LC_FR: return "french";
		case cyng::io::LC_EL: return "greek";
		case cyng::io::LC_HU: return "magyar";
		case cyng::io::LC_IS: return "icelandic";
		case cyng::io::LC_IT: return "italian";
		case cyng::io::LC_NN: return "norsk";
		case cyng::io::LC_JA: return "japanese";
		case cyng::io::LC_KO: return "hangul";
		case cyng::io::LC_FA: return "xepersian";
		case cyng::io::LC_PL: return "polish";
		case cyng::io::LC_SK: return "slovak";
		case cyng::io::LC_HE: return "hebrew";
		case cyng::io::LC_NL: return "dutch";
		case cyng::io::LC_EU: return "basque";
		case cyng::io::LC_BR: return "breton";
		case cyng::io::LC_CA: return "catalan";
		case cyng::io::LC_HR: return "croatian";
		case cyng::io::LC_ET: return "estonian";
		case cyng::io::LC_GL: return "galician";
		case cyng::io::LC_GA: return "irish";
		case cyng::io::LC_LA: return "latin";
		case cyng::io::LC_SE: return "samin";
		case cyng::io::LC_RO: return "romanian";
		case cyng::io::LC_GD: return "scottish";
		case cyng::io::LC_TR: return "turkish";
		case cyng::io::LC_CY: return "welsh";
		default:
			break;
		}
		return "english";
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

	std::string replace_latex_entities(std::string const& str)
	{
		//
		//	escape 
		//	\&     \$     \%     \#     \_    \{     \}
		//
		//replace_all(str, "&", "\\&");
		//replace_all(str, "$", "\\$");
		//replace_all(str, "%", "\\%");
		//replace_all(str, "#", "\\#");
		//replace_all(str, "_", "\\_");
		//replace_all(str, "{", "\\{");
		//replace_all(str, "}", "\\}");
		//replace_all(str, "<", "\\textless");
		//replace_all(str, ">", "\\textgreater");

		//	simple state engine
		enum {
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
				case '=':	//	<=
					r.append("\\leq");
					break;
				case '-':	//	<-
					r.append("\\leftarrow\\ ");
					break;
				case '<':	//	<<
					r.append("\\ll\\ ");
					break;
				default:
					r.append("\\textless\\ ");
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_GT:
				switch (c) {
				case '=':	//	>=
					r.append("$\\geq$");
					break;
				case '>':	//	>>
					r.append("\\gg\\ ");
					break;
				default:
					r.append("\\textgreater\\ ");
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_EQ:
				switch (c) {
				case '>':	//	=>
					r.append("\\Rightarrow\\ ");
					break;
				case '=':	//	==
					r.append("\\equiv\\ ");
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
				case '>':	//	->
					r.append("\\rightarrow\\ ");
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
				case '=':	//	!=
					r.append("\\neq\\ ");
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
				case ':':	//"::" not supported by LaTeX - mathtools required
					r.append("\\dblcolon\\ ");
					break;
				case '=':	//	:=
					r.append("\\coloneq\\ ");
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
				case '\\':
					r.append("\\\\");
					break;
				case '_':
					r.append("\\_");
					break;
				case '#':
					r.append("\\#");
					break;
				case '%':
					r.append("\\%");
					break;
				case '$':
					r.append("\\$");
					break;
				case '{':
					r.append("\\{");
					break;
				case '}':
					r.append("\\}");
					break;
				//case '[':
				//	r.append("\\[");
				//	break;
				//case ']':
				//	r.append("\\]");
				//	break;
				default:
					r += c;
					break;
				}
				break;
			}
		}

		switch (state) {

		case STATE_LT:		r += "\\textless\\ ";	break;
		case STATE_GT:		r += "\\textgreater\\ ";	break;
		case STATE_EQ:		r += '=';	break;
		case STATE_MINUS:	r += '-';	break;
		case STATE_EXCL:	r += '!';	break;
		case STATE_COL:		r += ':';	break;
		default:
			break;
		}

		auto start = std::begin(r);
		auto stop = std::end(r);
		return replace_unicode(boost::u8_to_u32_iterator<std::string::const_iterator>(start), boost::u8_to_u32_iterator<std::string::const_iterator>(stop));
//		return r;

	}

	std::string replace_unicode(boost::u8_to_u32_iterator<std::string::const_iterator> pos, boost::u8_to_u32_iterator<std::string::const_iterator> end)
	{
		std::u32string r;

		//	small optimization
//		r.reserve(std::distance(pos, end));
		// see http://mirrors.ibiblio.org/CTAN/fonts/stix/doc/stix.pdf
		while(pos != end) {
			switch(*pos) {

			case 0x2191: r += U"$\\uparrow$"; break; // ↑
			case 0x2192: r += U"$\\rightarrow$"; break; // →
			case 0x2193: r += U"$\\downarrow$"; break; // ↓
			case 0x2194: r += U"$\\leftrightarrow$"; break; // ↔
			case 0x2195: r += U"$\\updownarrow$"; break; // ↕
			case 0x2196: r += U"$\\nwarrow$"; break; // ↖
			case 0x2197: r += U"$\\nearrow$"; break; // ↗
			case 0x2198: r += U"$\\searrow$"; break; //↘
			case 0x2199: r += U"$\\swarrow$"; break; //↙
			case 0x219A: r += U"$\\nleftarrow$"; break; //↚
			case 0x219B: r += U"$\\nrightarrow$"; break; //↛
			case 0x219C: r += U"$\\leftwavearrow$"; break; //↜
			case 0x219D: r += U"$\\rightwavearrow$"; break; //↝
			case 0x219E: r += U"$\\twoheadleftarrow$"; break; //↞
			case 0x219F: r += U"$\\twoheaduparrow$"; break; //↟
			case 0x21A0: r += U"$\\twoheadrightarrow$"; break; //↠
			case 0x21A1: r += U"$\\twoheaddownarrow$"; break; //↡
			case 0x21A2: r += U"$\\leftarrowtail$"; break; //↢
			case 0x21A3: r += U"$\\rightarrowtail$"; break; //↣
			case 0x21A4: r += U"$\\mapsfrom$"; break; //↤
			case 0x21A5: r += U"$\\mapsup$"; break; //↥
			case 0x21A6: r += U"$\\mapsto$"; break; //↦
			case 0x21A7: r += U"$\\mapsdown$"; break; //↧
			case 0x21A9: r += U"$\\hookleftarrow$"; break; //↩
			case 0x21AA: r += U"$\\hookrightarrow$"; break; //↪
			case 0x21AB: r += U"$\\looparrowleft$"; break; //↫
			case 0x21AC: r += U"$\\looparrowrightv$"; break; // ↬
			case 0x21AD: r += U"$\\leftrightsquigarrow$"; break; //↭
			case 0x21AE: r += U"$\\nleftrightarrow$"; break; //↮
			case 0x21AF: r += U"$\\downzigzagarrow$"; break; //↯
			case 0x21B0: r += U"$\\Lsh$"; break; //↰
			case 0x21B1: r += U"$\\Rsh$"; break; //↱
			case 0x21B2: r += U"$\\Ldsh$"; break; //↲
			case 0x21B3: r += U"$\\Rdsh$"; break; //↳
			case 0x21B6: r += U"$\\curvearrowleft$"; break; //↶
			case 0x21B7: r += U"$\\curvearrowright$"; break; //↷
			case 0x21BA: r += U"$\\circlearrowleft$"; break; //↺
			case 0x21BB: r += U"$\\circlearrowright$"; break; //↻
			case 0x21BC: r += U"$\\leftharpoonup$"; break; //↼
			case 0x21BD: r += U"$\\leftharpoondown$"; break; //↽
			case 0x21BE: r += U"$\\upharpoonright$"; break; //↾
			case 0x21BF: r += U"$\\upharpoonleft$"; break; //↿
			case 0x21C0: r += U"$\\rightharpoonup$"; break; //⇀
			case 0x21C1: r += U"$\\rightharpoondown$"; break; //⇁
			case 0x21C2: r += U"$\\downharpoonright$"; break; //⇂
			case 0x21C3: r += U"$\\downharpoonleft$"; break; //⇃
			case 0x21C4: r += U"$\\rightleftarrows$"; break; //⇄
			case 0x21C5: r += U"$\\updownarrows$"; break; //⇅
			case 0x21C6: r += U"$\\leftrightarrows$"; break;
			case 0x21C7: r += U"$\\leftleftarrows$"; break; //⇇
			case 0x21C8: r += U"$\\upuparrows$"; break; //⇈
			case 0x21C9: r += U"$\\rightrightarrows$"; break; //⇉
			case 0x21CA: r += U"$\\downdownarrows$"; break; //⇊
			case 0x21CB: r += U"$\\leftrightharpoons$"; break; //⇋
			case 0x21CC: r += U"$\\rightleftharpoons$"; break; //⇌
			case 0x21CD: r += U"$\\nLeftarrow$"; break; //⇍
			case 0x21CE: r += U"$\\nLeftrightarrow$"; break; //⇎
			case 0x21CF: r += U"$\\nRightarrow$"; break; //⇏
			case 0x21D0: r += U"$\\Leftarrow$"; break; //⇐
			case 0x21D1: r += U"$\\Uparrow$"; break; //⇑
			case 0x21D2: r += U"$\\Rightarrow$"; break; //⇒
			case 0x21D3: r += U"$\\Downarrow$"; break; //⇓
			case 0x21D4: r += U"$\\Leftrightarrow$"; break; //⇔
			case 0x21D5: r += U"$\\Updownarrow$"; break; //⇕
			case 0x21D6: r += U"$\\Nwarrow$"; break; //⇖
			case 0x21D7: r += U"$\\Nearrow$"; break; //⇗
			case 0x21D8: r += U"$\\Searrow$"; break; //⇘
			case 0x21D9: r += U"$\\Swarrow$"; break; //⇙
			case 0x21DA: r += U"$\\Lleftarrow∗$"; break; //⇚
			case 0x21DB: r += U"$\\Rrightarrow∗$"; break; //⇛
			case 0x21DC: r += U"$\\leftsquigarrow$"; break; //⇜
			case 0x21DD: r += U"$\\rightsquigarrow$"; break; //⇝
			case 0x21E4: r += U"$\\barleftarrow$"; break; //⇤
			case 0x21E5: r += U"$\\rightarrowbar$"; break; //⇥
			case 0x21F4: r += U"$\\circleonrightarrow$"; break; //⇴
			case 0x21F5: r += U"$\\downuparrows$"; break; //⇵
			case 0x21F6: r += U"$\\rightthreearrows$"; break; //⇶
			case 0x21F7: r += U"$\\nvleftarrow$"; break; //⇷
			case 0x21F8: r += U"$\\nvrightarrow$"; break; //⇸
			case 0x21F9: r += U"$\\nvleftrightarrow$"; break; //⇹
			case 0x21FA: r += U"$\\nVleftarrow$"; break; //⇺
			case 0x21FB: r += U"$\\nVrightarrow$"; break; //⇻
			case 0x21FC: r += U"$\\nVleftrightarrow$"; break; //⇼
			case 0x21FD: r += U"$\\leftarrowtriangle$"; break; //⇽
			case 0x21FE: r += U"$\\rightarrowtriangle$"; break; //⇾
			case 0x21FF: r += U"$\\leftrightarrowtriangle$"; break; //⇿
			case 0x2208: r += U"$\\in$"; break; //∈
			case 0x2209: r += U"$\\notin$"; break; //∉
			case 0x220A: r += U"$\\smallin$"; break; //∊
			case 0x220B: r += U"$\\ni$"; break; //∋
			case 0x220C: r += U"$\\nni$"; break; //∌
			case 0x220D: r += U"$\\smallni$"; break; //∍

			case 0x221D: r += U"$\\propto$"; break; // ∝
//			case 0x221D: r += U"$\\varpropto$"; break; // ∝
			case 0x2223: r += U"$\\mid$"; break; //  ∣
//			case 0x2223: r += U"$\\shortmid$"; break; // ∣
			case 0x2224: r += U"$\\nmid$"; break; // ∤
//			case 0x2224: r += U"$\\nshortmid$"; break; // ∤
			case 0x2225: r += U"$\\parallel$"; break; // ∥
//			case 0x2225: r += U"$\\shortparallel$"; break; // ∥
			case 0x2226: r += U"$\\nparallel$"; break; // ∦
//			case 0x2226: r += U"$\\nshortparallel$"; break; // ∦
			case 0x2237: r += U"$\\Colon$"; break; //	∷
			case 0x2239: r += U"$\\dashcolon$"; break; // ∹
			case 0x223A: r += U"$\\dotsminusdots$"; break; // ∺
			case 0x223B: r += U"$\\kernelcontraction$"; break; // ∻
			case 0x223C: r += U"$\\sim$"; break; // ∼
//			case 0x223C: r += U"$\\thicksim$"; break; // ∼
			case 0x223D: r += U"$\\backsim$"; break; // ∽
			case 0x2241: r += U"$\\nsim$"; break; // ≁
			case 0x2242: r += U"$\\eqsim$"; break; // ≂
			case 0x2243: r += U"$\\simeq$"; break; // ≃
			case 0x2244: r += U"$\\nsime$"; break; // ≄
			case 0x2245: r += U"$\\cong$"; break; // ≅
			case 0x2246: r += U"$\\simneqq$"; break; // ≆
			case 0x2247: r += U"$\\ncong$"; break; // ≇
			case 0x2248: r += U"$\\approx$"; break; // ≈
//			case 0x2248: r += U"$\\thickapprox$"; break; // ≈
			case 0x2249: r += U"$\\napprox$"; break; // ≉
			case 0x224A: r += U"$\\approxeq$"; break; // ≊
			case 0x224B: r += U"$\\approxident$"; break; // ≋
			case 0x224C: r += U"$\\backcong$"; break; // ≌
			case 0x224D: r += U"$\\asymp$"; break; // ≍
			case 0x224E: r += U"$\\Bumpeq$"; break; // ≎
			case 0x224F: r += U"$\\bumpeq$"; break; // ≏
			case 0x2250: r += U"$\\doteq$"; break; // ≐
			case 0x2251: r += U"$\\Doteq$"; break; // ≑
			case 0x2252: r += U"$\\fallingdotseq$"; break; // ≒
			case 0x2253: r += U"$\\risingdotseq$"; break; // ≓
			case 0x2254: r += U"$\\coloneq$"; break; // ≔
			case 0x2255: r += U"$\\eqcolon$"; break; // ≕
			case 0x2256: r += U"$\\eqcirc$"; break; // ≖
			case 0x2257: r += U"$\\circeq$"; break; // ≗
			case 0x2258: r += U"$\\arceq$"; break; // ≘
			case 0x2259: r += U"$\\wedgeq$"; break; // ≙
			case 0x225A: r += U"$\\veeeq$"; break; // ≚
			case 0x225B: r += U"$\\stareq$"; break; // ≛
			case 0x225C: r += U"$\\triangleq$"; break; // ≜
			case 0x225D: r += U"$\\eqdef$"; break; // ≝
			case 0x225E: r += U"$\\measeq$"; break; // ≞
			case 0x225F: r += U"$\\questeq$"; break; // ≟
			case 0x2260: r += U"$\\ne$"; break; // ≠
			case 0x2261: r += U"$\\equiv$"; break; // ≡
			case 0x2262: r += U"$\\nequiv$"; break; // ≢
			case 0x2263: r += U"$\\Equiv$"; break; //≢
			case 0x2264: r += U"$\\leq$"; break; // ≤
			case 0x2265: r += U"$\\geq$"; break; // ≥
			case 0x2266: r += U"$\\leqq$"; break; // ≦
			case 0x2267: r += U"$\\geqq$"; break; // ≧
			case 0x2268: r += U"$\\lneqq$"; break; // ≨
//			case 0x2268: r += U"$\\lvertneqq$"; break; // ≨
			case 0x2269: r += U"$\\gneqq$"; break; // ≩
//			case 0x2269: r += U"$\\gvertneqq$"; break; // ≩
			case 0x226A: r += U"$\\ll$"; break; // ≪
			case 0x226B: r += U"$\\gg$"; break; // ≫
			case 0x226C: r += U"$\\between$"; break; // ≬
			case 0x226D: r += U"$\\nasymp$"; break; // ≭
			case 0x226E: r += U"$\\nless$"; break; // ≮
			case 0x226F: r += U"$\\ngtr$"; break; // ≯
			case 0x2270: r += U"$\\nleq$"; break; // ≰
			case 0x2271: r += U"$\\ngeq$"; break; // ≱
			case 0x2272: r += U"$\\lesssim$"; break; // ≲
			case 0x2273: r += U"$\\gtrsim$"; break; // ≳
			case 0x2274: r += U"$\\nlesssim$"; break; // ≴
			case 0x2275: r += U"$\\ngtrsim$"; break; // ≵
			case 0x2276: r += U"$\\lessgtr$"; break; // ≶
			case 0x2277: r += U"$\\gtrless$"; break; // ≷
			case 0x2278: r += U"$\\nlessgtr$"; break; // ≸
			case 0x2279: r += U"$\\ngtrless$"; break; // ≹
			case 0x227A: r += U"$\\prec$"; break; // ≺
			case 0x227B: r += U"$\\succ$"; break; // ≻
			case 0x227C: r += U"$\\preccurlyeq$"; break; // ≼
			case 0x227D: r += U"$\\succcurlyeq$"; break; // ≽
			case 0x227E: r += U"$\\precsim$"; break; // ≾
			case 0x227F: r += U"$\\succsim$"; break; // ≿
			case 0x2280: r += U"$\\nprec$"; break; // ⊀
			case 0x2281: r += U"$\\nsucc$"; break; // ⊁
			case 0x2282: r += U"$\\subset$"; break; // ⊂
			case 0x2283: r += U"$\\supset$"; break; // ⊃
			case 0x2284: r += U"$\\nsubset$"; break; // ⊄
			case 0x2285: r += U"$\\nsupset$"; break; // ⊅
			case 0x2286: r += U"$\\subseteq$"; break; // ⊆
			case 0x2287: r += U"$\\supseteq$"; break; // ⊇
			case 0x2288: r += U"$\\nsubseteq$"; break; // ⊈

			case 0x2289: r += U"$\\nsupseteq$"; break; // ⊉
			case 0x228A: r += U"$\\subsetneq$"; break; // ⊊
//			case 0x228A: r += U"$\\varsubsetneq$"; break; // ⊊
			case 0x228B: r += U"$\\supsetneq$"; break; // ⊋
//			case 0x228B: r += U"$\\varsupsetneq$"; break; // ⊋s
			case 0x228F: r += U"$\\sqsubset$"; break; // ⊏
			case 0x2290: r += U"$\\sqsupset$"; break; // ⊐
			case 0x2291: r += U"$\\sqsubseteq$"; break; // ⊑
			case 0x2292: r += U"$\\sqsupseteq$"; break; //⊒
			case 0x22A2: r += U"$\\vdash$"; break; // ⊢
			case 0x22A3: r += U"$\\dashv$"; break; // ⊣
			case 0x22A6: r += U"$\\assert$"; break; // ⊦
			case 0x22A7: r += U"$\\models$"; break; // ⊧
			case 0x22A8: r += U"$\\vDash$"; break; // ⊨
			case 0x22A9: r += U"$\\Vdash$"; break; // ⊩
			case 0x22AA: r += U"$\\Vvdash$"; break; // ⊪
			case 0x22AB: r += U"$\\VDash$"; break; // ⊫
			case 0x22AC: r += U"$\\nvdash$"; break; // ⊬
			case 0x22AD: r += U"$\\nvDash$"; break; // ⊭
			case 0x22AE: r += U"$\\nVdash$"; break; // ⊮
			case 0x22AF: r += U"$\\nVDash$"; break; // ⊯
			case 0x22B0: r += U"$\\prurel$"; break; // ⊰
			case 0x22B1: r += U"$\\scurel$"; break; // ⊱
			case 0x22B2: r += U"$\\vartriangleleft$"; break; // ⊲
			case 0x22B3: r += U"$\\vartriangleright$"; break; // ⊳
			case 0x22B4: r += U"$\\trianglelefteq$"; break; // ⊴
			case 0x22B5: r += U"$\\trianglerighteq$"; break; // ⊵
			case 0x22B6: r += U"$\\origof$"; break;
			case 0x22B7: r += U"$\\imageof$"; break;
			case 0x22B8: r += U"$\\multimap$"; break; // ⊸
			case 0x22C8: r += U"$\\bowtie$"; break; // ⋈
			case 0x22CD: r += U"$\\backsimeq$"; break; // ⋍
			case 0x22D0: r += U"$\\Subset$"; break; // ⋐
			case 0x22D1: r += U"$\\Supset$"; break; // ⋑
			case 0x22D4: r += U"$\\pitchfork$"; break; // ⋔
			case 0x22D5: r += U"$\\equalparallel$"; break; // ⋕
			case 0x22D6: r += U"$\\lessdot$"; break; // ⋖
			case 0x22D7: r += U"$\\gtrdot$"; break; // ⋗
			case 0x22D8: r += U"$\\lll$"; break; // less ⋘
			case 0x22D9: r += U"$\\ggg$"; break; // ⋙
			case 0x22DA: r += U"$\\lesseqgtr$"; break; // ⋚
			case 0x22DB: r += U"$\\gtreqless$"; break; // ⋛
			case 0x22DC: r += U"$\\eqless$"; break; // ⋜
			case 0x22DD: r += U"$\\eqgtr$"; break; // ⋝
			case 0x22DE: r += U"$\\curlyeqprec$"; break; // ⋞
			case 0x22DF: r += U"$\\curlyeqsucc$"; break; // ⋟
			case 0x22E0: r += U"$\\npreccurlyeq$"; break; // ⋠
			case 0x22E1: r += U"$\\nsucccurlyeq$"; break; // ⋡
			case 0x22E2: r += U"$\\nsqsubseteq$"; break; // ⋢
			case 0x22E3: r += U"$\\nsqsupseteq$"; break; // ⋣
			case 0x22E4: r += U"$\\sqsubsetneq$"; break; // ⋤
			case 0x22E5: r += U"$\\sqsupsetneq$"; break; // ⋥
			case 0x22E6: r += U"$\\lnsim$"; break; // ⋦
			case 0x22E7: r += U"$\\gnsim$"; break; // ⋧
			case 0x22E8: r += U"$\\precnsim$"; break; // ⋨
			case 0x22E9: r += U"$\\succnsim$"; break; // ⋩
			case 0x22EA: r += U"$\\nvartriangleleft$"; break; // ⋪
			case 0x22EB: r += U"$\\nvartriangleright$"; break; // ⋫
			case 0x22EC: r += U"$\\ntrianglelefteq$"; break; // ⋬
			case 0x22ED: r += U"$\\ntrianglerighteq$"; break; // ⋭
			case 0x22EE: r += U"$\\vdots$"; break; // ⋮
			case 0x22F0: r += U"$\\adots$"; break; // ⋰
			case 0x22F1: r += U"$\\ddots$"; break; // ⋱
			case 0x22F2: r += U"$\\disin$"; break; // ⋲
			case 0x22F3: r += U"$\\varisins$"; break; // ⋳
			case 0x22F4: r += U"$\\isins$"; break; // ⋴
			case 0x22F5: r += U"$\\isindot$"; break; // ⋵
			case 0x22F6: r += U"$\\varisinobar$"; break; // ⋶
			case 0x22F7: r += U"$\\isinobar$"; break; // ⋷
			case 0x22F8: r += U"$\\isinvb$"; break; // ⋸
			case 0x22F9: r += U"$\\isinE$"; break; // ⋹
			case 0x22FA: r += U"$\\nisd$"; break; // ⋺
			case 0x22FB: r += U"$\\varnis$"; break; // ⋻
			case 0x22FC: r += U"$\\nis$"; break; // ⋼
			case 0x22FD: r += U"$\\varniobar$"; break; // ⋽
			case 0x22FE: r += U"$\\niobar$"; break; // ⋾
			case 0x22FF: r += U"$\\bagmember$"; break; // ⋿
			case 0x2322: r += U"$\\frown$"; break; // ⌢
//			case 0x2322: r += U"$\\smallfrown$"; break; // ⌢
			case 0x2323: r += U"$\\smile$"; break; // ⌣
//			case 0x2323: r += U"$\\smallsmile$"; break; // ⌣
			case 0x233F: r += U"$\\APLnotslash$"; break; // ⌿
			case 0x25B5: r += U"$\\vartriangle$"; break; // ▵
			case 0x27C2: r += U"$\\perp$"; break; // ⟂
			case 0x27C8: r += U"$\\bsolhsub$"; break; // ⟈
			case 0x27C9: r += U"$\\suphsol$"; break; // ⟉
			case 0x27D2: r += U"$\\upin$"; break; // ⟒
			case 0x27D3: r += U"$\\pullback$"; break; // ⟓
			case 0x27D4: r += U"$\\pushout$"; break; // ⟔
			case 0x27DA: r += U"$\\DashVDash$"; break; // ⟚
			case 0x27DB: r += U"$\\dashVdash$"; break; // ⟛
			case 0x27DC: r += U"$\\multimapinv$"; break; // ⟜
			default:
				r += *pos;
				break;
			}
			++pos;
		}
		auto const b = boost::u32_to_u8_iterator<std::u32string::const_iterator>(r.begin());
		auto const e = boost::u32_to_u8_iterator<std::u32string::const_iterator>(r.end());
		return std::string(b, e);
	}

	bool is_language_supported(std::string lang)
	{
		//ABAP(R / 2 4.3, R / 2 5.0, R / 3 3.1, R / 3 4.6C, R / 3 6.10)
		if (boost::algorithm::iequals(lang, "ABAP"))	return true;
		if (boost::algorithm::iequals(lang, "ACM"))	return true;
		if (boost::algorithm::iequals(lang, "ACMscript"))	return true;
		if (boost::algorithm::iequals(lang, "ACSL "))	return true;
		//	Ada(2005, 83, 95)
		if (boost::algorithm::iequals(lang, "Ada"))	return true;
		//	Algol(60, 68)
		if (boost::algorithm::iequals(lang, "Algol"))	return true;
		if (boost::algorithm::iequals(lang, "Ant"))	return true;
		//	Assembler(Motorola68k, x86masm)
		if (boost::algorithm::iequals(lang, "Assembler"))	return true;
		//	Awk(gnu, POSIX)
		if (boost::algorithm::iequals(lang, "Awk"))	return true;
		if (boost::algorithm::iequals(lang, "bash"))	return true;
		//	Basic(Visual)
		if (boost::algorithm::iequals(lang, "Basic"))	return true;
		//	C(ANSI, Handel, Objective, Sharp)
		if (boost::algorithm::iequals(lang, "C"))	return true;
		//	C++ (11, ANSI, GNU, ISO, Visual)
		if (boost::algorithm::iequals(lang, "C++"))	return true;
		//	Caml(light, Objective)
		if (boost::algorithm::iequals(lang, "Caml"))	return true;
		if (boost::algorithm::iequals(lang, "CIL"))	return true;
		if (boost::algorithm::iequals(lang, "Clean"))	return true;
		//	Cobol(1974, 1985, ibm)
		if (boost::algorithm::iequals(lang, "Cobol"))	return true;
		//	Comal 80
		if (boost::algorithm::iequals(lang, "Comal"))	return true;
		//	command.com(WinXP)
		if (boost::algorithm::iequals(lang, "cmd"))	return true;
		if (boost::algorithm::iequals(lang, "command.com"))	return true;
		if (boost::algorithm::iequals(lang, "Comsol"))	return true;
		if (boost::algorithm::iequals(lang, "csh"))	return true;
		if (boost::algorithm::iequals(lang, "Delphi"))	return true;
		if (boost::algorithm::iequals(lang, "Eiffel"))	return true;
		if (boost::algorithm::iequals(lang, "Elan"))	return true;
		if (boost::algorithm::iequals(lang, "erlang"))	return true;
		if (boost::algorithm::iequals(lang, "Euphoria"))	return true;
		//	Fortran(03, 08, 77, 90, 95)
		if (boost::algorithm::iequals(lang, "Fortran"))	return true;
		if (boost::algorithm::iequals(lang, "GAP"))	return true;
		if (boost::algorithm::iequals(lang, "GCL"))	return true;
		if (boost::algorithm::iequals(lang, "Gnuplot"))	return true;
		if (boost::algorithm::iequals(lang, "hansl"))	return true;
		if (boost::algorithm::iequals(lang, "Haskell"))	return true;
		if (boost::algorithm::iequals(lang, "HTML"))	return true;
		//	IDL(empty, CORBA)
		if (boost::algorithm::iequals(lang, "IDL"))	return true;
		if (boost::algorithm::iequals(lang, "inform"))	return true;
		//	Java(empty, AspectJ)
		if (boost::algorithm::iequals(lang, "Java"))	return true;
		if (boost::algorithm::iequals(lang, "JVMIS"))	return true;
		if (boost::algorithm::iequals(lang, "ksh"))	return true;
		if (boost::algorithm::iequals(lang, "Lingo"))	return true;
		//	Lisp(empty, Auto)
		if (boost::algorithm::iequals(lang, "Lisp"))	return true;
		if (boost::algorithm::iequals(lang, "LLVM"))	return true;
		if (boost::algorithm::iequals(lang, "Logo"))	return true;
		//	Lua(5.0, 5.1, 5.2, 5.3)
		if (boost::algorithm::iequals(lang, "Lua"))	return true;
		//	make(empty, gnu)
		if (boost::algorithm::iequals(lang, "make"))	return true;
		//	Mathematica(1.0, 3.0, 5.2)
		if (boost::algorithm::iequals(lang, "Mathematica"))	return true;
		if (boost::algorithm::iequals(lang, "Matlab"))	return true;
		if (boost::algorithm::iequals(lang, "Mercury"))	return true;
		if (boost::algorithm::iequals(lang, "MetaPost"))	return true;
		if (boost::algorithm::iequals(lang, "Miranda"))	return true;
		if (boost::algorithm::iequals(lang, "Mizar"))	return true;
		if (boost::algorithm::iequals(lang, "ML"))	return true;
		if (boost::algorithm::iequals(lang, "Modula-2"))	return true;
		if (boost::algorithm::iequals(lang, "MuPAD"))	return true;
		if (boost::algorithm::iequals(lang, "NASTRAN"))	return true;
		if (boost::algorithm::iequals(lang, "Oberon-2"))	return true;
		//	OCL(decorative, OMG)
		if (boost::algorithm::iequals(lang, "OCL"))	return true;
		if (boost::algorithm::iequals(lang, "Octave"))	return true;
		if (boost::algorithm::iequals(lang, "Oz"))	return true;
		//	Pascal(Borland6, Standard, XSC)
		if (boost::algorithm::iequals(lang, "Pascal"))	return true;
		if (boost::algorithm::iequals(lang, "Perl"))	return true;
		if (boost::algorithm::iequals(lang, "PHP"))	return true;
		if (boost::algorithm::iequals(lang, "PL/I"))	return true;
		if (boost::algorithm::iequals(lang, "Plasm"))	return true;
		if (boost::algorithm::iequals(lang, "PostScript"))	return true;
		if (boost::algorithm::iequals(lang, "POV"))	return true;
		if (boost::algorithm::iequals(lang, "Prolog"))	return true;
		if (boost::algorithm::iequals(lang, "Promela"))	return true;
		if (boost::algorithm::iequals(lang, "PSTricks"))	return true;
		if (boost::algorithm::iequals(lang, "Python"))	return true;
		if (boost::algorithm::iequals(lang, "R"))	return true;
		if (boost::algorithm::iequals(lang, "Reduce"))	return true;
		if (boost::algorithm::iequals(lang, "Rexx"))	return true;
		if (boost::algorithm::iequals(lang, "RSL"))	return true;
		if (boost::algorithm::iequals(lang, "Ruby"))	return true;
		if (boost::algorithm::iequals(lang, "S(empty, PLUS)"))	return true;
		if (boost::algorithm::iequals(lang, "SAS"))	return true;
		if (boost::algorithm::iequals(lang, "Scala"))	return true;
		if (boost::algorithm::iequals(lang, "Scilab"))	return true;
		if (boost::algorithm::iequals(lang, "sh"))	return true;
		if (boost::algorithm::iequals(lang, "SHELXL"))	return true;
		//	Simula(67, CII, DEC, IBM)
		if (boost::algorithm::iequals(lang, "Simula"))	return true;
		if (boost::algorithm::iequals(lang, "SPARQL"))	return true;
		if (boost::algorithm::iequals(lang, "SQL"))	return true;
		if (boost::algorithm::iequals(lang, "tcl(empty, tk)"))	return true;
		//	TeX(AlLaTeX, common, LaTeX, plain, primitive)
		if (boost::algorithm::iequals(lang, "TeX"))	return true;
		if (boost::algorithm::iequals(lang, "VBScript"))	return true;
		if (boost::algorithm::iequals(lang, "Verilog"))	return true;
		//	VHDL(empty, AMS)
		if (boost::algorithm::iequals(lang, "VHDL"))	return true;
		//	VRML(97)
		if (boost::algorithm::iequals(lang, "VRML"))	return true;
		if (boost::algorithm::iequals(lang, "XML"))	return true;
		if (boost::algorithm::iequals(lang, "XSLT"))	return true;
		return false;
	}

	std::string cleanup_language(std::string const& lang)
	{
		if (boost::algorithm::iequals(lang, "cmd"))	return "command.com";
		if (boost::algorithm::iequals(lang, "latex"))	return "TeX";
		return lang;
	}

}


