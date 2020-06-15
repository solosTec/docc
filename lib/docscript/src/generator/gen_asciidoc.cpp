/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2020 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/gen_asciidoc.h>
//#include "filter/binary_to_asciidoc.h"

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/numeric_cast.hpp>
#include <cyng/dom/reader.h>
#include <cyng/csv.h>
#include <cyng/io/bom.h>
#include <cyng/set_cast.h>

#include <fstream>

#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/nil_generator.hpp>

namespace docscript
{
	gen_asciidoc::gen_asciidoc(std::vector< cyng::filesystem::path > const& inc)
		: generator(inc)
	{
		register_this();
	}

	void gen_asciidoc::register_this()
	{
		generator::register_this();

		vm_.register_function("demo", 0, std::bind(&gen_asciidoc::demo, this, std::placeholders::_1));


	}

	void gen_asciidoc::generate_file(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto p = cyng::value_cast(frame.at(0), cyng::filesystem::path());
		if (cyng::filesystem::is_directory(p)) {
			p = p / "out.asciidoc";
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

	std::ofstream& gen_asciidoc::emit_file(std::ofstream& ofs, cyng::vector_t::const_iterator pos, cyng::vector_t::const_iterator end) const
	{
		ofs
			<< ":pdf-page-size: A4"
			<< std::endl
			<< std::endl
			;
		emit_meta(ofs);
		while (pos != end) {
			emit_obj(ofs, *pos);
			++pos;
		}
		return ofs;
	}

	std::ofstream& gen_asciidoc::emit_obj(std::ofstream& ofs, cyng::object obj) const
	{
		ofs
			<< cyng::io::to_str(obj)
			<< std::endl
			;
		return ofs;
	}

	std::ofstream& gen_asciidoc::emit_meta(std::ofstream& ofs) const
	{
		//	meta data nor supported by github
		for (auto const& e : meta_) {
			ofs
				<< "// "
				<< e.first
				<< ":\t"
				<< cyng::io::to_str(e.second)
				<< std::endl
				;
		}

		ofs << std::endl;
		return ofs;
	}

	void gen_asciidoc::generate_meta(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const p = cyng::value_cast(frame.at(0), cyng::filesystem::path());
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

	void gen_asciidoc::print_hline(cyng::context& ctx)
	{
		//	same as MD
		ctx.push(cyng::make_object("\n" + std::string(3, '-')));
	}

	void gen_asciidoc::convert_numeric(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		ctx.push(frame.at(0));
	}

	void gen_asciidoc::convert_alpha(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const str = cyng::value_cast<std::string>(frame.at(0), "");

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
		ctx.push(cyng::make_object(replace_asciidoc_entities(str)));
	}

	void gen_asciidoc::print_symbol(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::string r;
		for (auto obj : frame) {
			auto const symbol = cyng::value_cast<std::string>(obj, "");
			if (boost::algorithm::equals(symbol, "pilcrow")) {
				r.append("&#182;");
			}
			else if (boost::algorithm::equals(symbol, "copyright")) {
				r.append("&#169;"); 
			}
			else if (boost::algorithm::equals(symbol, "registered")) {
				r.append("&#174;");
			}
			else if (boost::algorithm::iequals(symbol, "latex")) {
				r += "L";
				r += "<sup>A</sup>";
				r += "T";
				r += "<sub>E</sub>";
				r += "X";
			}
			else if (boost::algorithm::iequals(symbol, "celsius")) {
				r.append("&#8451;");
			}
			else if (boost::algorithm::equals(symbol, "micro")) {
				r.append("&#181;");
			}
			else if (boost::algorithm::iequals(symbol, "ohm")) {
				r.append("&#8486;");
			}
			else if (boost::algorithm::equals(symbol, "degree")) {
				r.append("&#176;");
			}
			else if (boost::algorithm::equals(symbol, "promille")) {
				r.append("&#8240;");
			}
			else if (boost::algorithm::iequals(symbol, "lambda")) {
				r.append("&#8257;");
			}
			else if (boost::algorithm::equals(symbol, "ellipsis")) {
				r.append("&#2026;");
			}
			else if (boost::algorithm::equals(symbol, "multiply")) {
				r.append("&#215;");
			}
			else {
				r += symbol;
			}
		}
		ctx.push(cyng::make_object(r));
	}

	void gen_asciidoc::print_currency(cyng::context& ctx)
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

	void gen_asciidoc::paragraph(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::string par = accumulate_plain_text(frame);
		ctx.push(cyng::make_object(par));
	}

	void gen_asciidoc::abstract(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
// 		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));

		auto const title = cyng::value_cast<std::string>(reader.get("title"), "Abstract");
		auto const text = accumulate_plain_text(reader.get("text"));

		//
		// simulate an abstract a header with highest level
		//
		std::stringstream ss;
		ss
			<< std::endl
			<< "# "
			<< title
			<< std::endl
			<< text
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));
		
	}
	
	void gen_asciidoc::quote(cyng::context& ctx)
	{
		//	[%(("q":{1,2,3}),("source":Earl Wilson),("url":https://www.brainyquote.com/quotes/quotes/e/earlwilson385998.html))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));

		auto const cite = cyng::value_cast<std::string>(reader.get("url"), "");
		auto const source = cyng::value_cast<std::string>(reader.get("source"), "");
		auto const quote = accumulate_plain_text(reader.get("q"));
		//auto const el = html::figure(html::blockquote(html::cite_(cite), quote), html::figcaption(html::cite(source)));
		ctx.push(cyng::make_object("\n> " + quote));
	}

	void gen_asciidoc::list(cyng::context& ctx)
	{
		//	[%(("items":[<p>one </p>,<p>two </p>,<p>three </p>]),("style":{lower-roman}),("type":ordered))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const type = cyng::value_cast<std::string>(reader.get("type"), "ordered");
		auto const style = cyng::value_cast<std::string>(reader.get("style"), "disc");
		auto const items = cyng::to_vector(reader.get("items"));

		auto b = boost::algorithm::equals(type, "ordered") || boost::algorithm::equals(type, "ol");

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

	void gen_asciidoc::link(cyng::context& ctx)
	{
		//	[%(("text":{LaTeX}),("url":{https,:,//www,.,latex-project,.,org/}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));
		auto const text = accumulate_plain_text(reader.get("text"));
		auto const url = cyng::io::to_str(reader.get("url"));
		auto const title = accumulate_plain_text(reader.get("title"));

		//	"title" attribute not supported by Asciidoc
		std::stringstream ss;
		ss
			<< "link:"
			<< url
			<< '['
			<< text
			<< "]"
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::figure(cyng::context& ctx)
	{
		//	 [%(("alt":{Giovanni,Bellini,,,Man,wearing,a,turban}),("caption":{Giovanni,Bellini,,,Man,wearing,a,turban}),("source":{LogoSmall,.,jpg}),("tag":{338,d542a-a4e3-4a4c-9efe-b8d3032306c3}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//ctx.push(frame.at(0));
		auto const reader = cyng::make_reader(frame.at(0));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const alt = cyng::io::to_str(reader.get("alt"));
		auto const source = cyng::io::to_str(reader.get("source"));
		auto const tag = name_gen_(cyng::value_cast(reader.get("tag"), caption));
		auto const id = boost::uuids::to_string(tag);
		auto const width = cyng::value_cast(reader.get("width"), 1.0);	//	no chance with asciidoc  - works with pixel units only

		//[#label]
		//[caption="Figure 1"]
		//image::tux.png[sunset,300,200]
		std::stringstream ss;
		ss
			<< "[#"
			<< id
			<< "]"
			<< std::endl 
			<< "[caption=\""
			<< caption
			<< "\"]"
			<< std::endl
			<< "image::"
			<< resolve_path(source).string()
			<< "["
			<< alt
			<< "]"
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::gallery(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
	}

	void gen_asciidoc::code(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const caption = accumulate_plain_text(reader.get("caption"));
		auto const line_numbers = value_cast(reader.get("linenumbers"), false);

		auto const source = cyng::value_cast<std::string>(reader.get("source"), "source.txt");
		auto const p = resolve_path(source);
		auto const language = cyng::value_cast(reader.get("language"), get_extension(p));

		if (cyng::filesystem::exists(p) && cyng::filesystem::is_regular(p)) {

			std::stringstream ss;
			if (boost::algorithm::equals(language, "bin") || boost::algorithm::iequals(language, "binary")) {
				// binary mode required
				std::ifstream  ifs(p.string(), std::ios::binary);
				ifs.unsetf(std::ios::skipws);
				cyng::buffer_t const inp((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
				//binary_to_asciidoc filter(line_numbers, uuid_gen_());
				//filter.convert(ss, inp);
			}
			else {
				//	.CAPTION
				//	[source,LANGUAGE]
				//	----
				//	CODE
				//	----
				//
				std::ifstream  ifs(p.string());
				ss
					<< "."
					<< caption
					<< std::endl
					<< "[source,"
					<< language
					<< "]"
					<< std::endl
					<< std::string(4, '-')
					<< std::endl
					<< ifs.rdbuf()
					<< std::endl
					<< std::string(4, '-')
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
			ctx.push(cyng::make_object(source));

		}
	}

	void gen_asciidoc::def(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

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

			auto const text = accumulate_plain_text(def.second);
			ss
				<< std::endl	//	required to detect the definition at all
				<< replace_asciidoc_entities(def.first)
				<< std::endl
				<< ':'
				<< ' '
				<< replace_asciidoc_entities(text)
				<< std::endl
				;
		}

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::annotation(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		std::stringstream ss;
		ss
			<< std::endl
			<< '>'
			<< ' '
			<< accumulate_plain_text(frame)
			<< std::endl
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::table(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame.at(0));
		auto const title = accumulate_plain_text(reader.get("title"));
		auto const header = cyng::to_tuple(reader.get("header"));
		auto const source = cyng::io::to_str(reader.get("source"));
		auto const tag = name_gen_(cyng::value_cast(reader.get("tag"), source));
		auto const id = boost::uuids::to_string(tag);

		auto const p = resolve_path(source);
		if (cyng::filesystem::exists(p) && cyng::filesystem::is_regular(p)) {

			//
			//	parse the CSV file into a vector
			//
			auto const csv = cyng::csv::read_file(p.string());
			std::stringstream ss;
			ss 
				<< std::endl
				<< "[%header,format=csv]"
				<< std::endl
				<< "|==="
				<< std::endl
				;


			for (auto const& row : csv) {
				auto const tpl = cyng::to_tuple(row);

				bool initial{ true };
				for (auto const& cell : tpl) {
					if (initial) {
						ss << ',';
					}
					else {
						initial = true;
					}
					ss << cyng::io::to_str(cell);
				}
				ss << std::endl;
			}
			ss << "|===";

			ctx.push(cyng::make_object(ss.str()));
		}
		else {

			std::cerr
				<< "***error cannot open CSV file ["
				<< source
				<< "]"
				<< std::endl;
			ctx.push(cyng::make_object("cannot open file [" + source + "]"));
		}
	}

	void gen_asciidoc::alert(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();

		auto const reader = cyng::make_reader(frame);
		auto const map = cyng::to_param_map(reader.get(0));

		if (!map.empty()) {
			//	note, tip, info, warning, error, important
			auto const type = boost::algorithm::to_upper_copy(map.begin()->first);
			auto const msg = accumulate_plain_text(map.begin()->second);

			//	The following emojis are working for github. Other MD dialects
			//	like https://dillinger.io/ don't understand this. Since our
			//	main target is github the emojies are used. Feel free to find
			//	a better solution.
			//	A list of supported emojies on github:
			//	https://gist.github.com/rxaviers/7360908
			//
			if (boost::algorithm::equals(type, "INFO")) {

				//	 &#xFE0F; ℹ 
				ctx.push(cyng::make_object("> :information_source: " + msg));
				//ctx.push(cyng::make_object("> &#2139; " + msg));
			}
			else if (boost::algorithm::equals(type, "CAUTION")) {

				//	&#10071; ❗
				ctx.push(cyng::make_object("> :heavy_exclamation_mark: " + msg));
				//ctx.push(cyng::make_object("> &#10071; " + msg));
			}
			else if (boost::algorithm::equals(type, "WARNING")) {

				//	&#9888; ⚠️
				ctx.push(cyng::make_object("> :warning: " + msg));
				//ctx.push(cyng::make_object("> &#9888; " + msg));
			}
			else {

				//	&#x1F4A1; 💡
				ctx.push(cyng::make_object("> :bulb: " + msg));
				//ctx.push(cyng::make_object("> &#x1F4A1; " + msg));
			}
		}
		else {

			ctx.push(cyng::make_object("***error in ALERT definition"));

		}
	}

	void gen_asciidoc::make_ref(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(cyng::io::to_str(frame)));
	}

	void gen_asciidoc::make_tok(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		auto const reader = cyng::make_reader(frame.at(0));
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("depth"), 3u);

		std::stringstream ss;
		ss
			<< ":toc:"
			<< std::endl
			<< ":toc-title: "
			<< get_name(i18n::WID_TOC)
			<< std::endl
			<< ":toclevels: "
			<< level 
			<< std::endl
			;
		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::format_italic(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object("_" + accumulate_plain_text(frame) + "_"));
	}

	void gen_asciidoc::format_bold(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		ctx.push(cyng::make_object("**" + accumulate_plain_text(frame) + "**"));
	}

	void gen_asciidoc::format_tt(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		ctx.push(cyng::make_object("`" + accumulate_plain_text(frame) + "`"));
	}

	void gen_asciidoc::format_color(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

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

	void gen_asciidoc::format_sub(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//	pandoc style
		ctx.push(cyng::make_object("~" + accumulate_plain_text(frame) + "~"));
	}

	void gen_asciidoc::format_sup(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//	pandoc style
		ctx.push(cyng::make_object("^" + accumulate_plain_text(frame) + "^"));
	}
	
	void gen_asciidoc::format_mark(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//	use HTML5
		ctx.push(cyng::make_object("<mark>" + accumulate_plain_text(frame) + "</mark>"));
	}	

	void gen_asciidoc::header(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();

		auto const reader = cyng::make_reader(frame.at(0));
		auto const title = accumulate_plain_text(reader.get("title"));

		auto const tag = name_gen_(cyng::value_cast(reader.get("tag"), title));
		auto const id = boost::uuids::to_string(tag);
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0ul);

		auto const number = content_table_.add(level, tag, title);

		std::stringstream ss;
		ss
			<< "[#"
			<< id
			<< "]"
			<< std::endl 
			<< std::string(level, '=')
			<< ' '
			<< number
			<< ' '
			<< title
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::section(int level, cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const title = accumulate_plain_text(frame);
		auto const tag = name_gen_(title);
		auto const id = boost::uuids::to_string(tag);
		auto const number = content_table_.add(level, tag, title);

		//ctx.push(cyng::make_object(std::string(level, '#') + " " + number + " " + title));

		std::stringstream ss;
		ss
			<< "[#"
			<< id
			<< "]"
			<< std::endl
			<< std::string(level, '=')
			<< ' '
			<< number
			<< ' '
			<< title
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::make_footnote(cyng::context& ctx)
	{
		//	[%(("level":1),("tag":{79bf3ba0-2362-4ea5-bcb5-ed93844ac59a}),("title":{Basics}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const note = accumulate_plain_text(frame);
		auto const tag = uuid_gen_();
		//footnotes_.emplace_back( tag, note );
		//auto const idx = footnotes_.size();

		//	.footnote:[Clarification about this statement.]
		std::stringstream ss;
		ss
			<< ".footnote:["
			<< note
			<< ']'
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_asciidoc::demo(cyng::context& ctx)
	{
		//	 [%(("red":{spiced,up}))]
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const reader = cyng::make_reader(frame);
		auto const map = cyng::value_cast(reader.get(0), cyng::param_map_t());

		ctx.push(cyng::make_object("DEMO"));
	}

	std::string replace_asciidoc_entities(std::string const& str)
	{
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
					r.append("&#8804;");
					break;
				case '-':	//	<-
					r.append("&#10229;");
					break;
				case '<':	//	<<
					r.append("&#8810;");
					break;
				default:
					r += "\\>";
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_GT:
				switch (c) {
				case '=':	//	>=
					r.append("&#8805;");
					break;
				case '>':	//	>>
					r.append("&#8811;");
					break;
				default:
					r.append("\\>");
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_EQ:
				switch (c) {
				case '>':	//	=>
					r.append("&#10233;");
					break;
				case '=':	//	==
					r.append("&#8801;");
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
					r.append("&#10230;");
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
					r.append("&#8800;");
					break;
				default:
					r.append("\\!");
					r += c;
					break;
				}
				state = STATE_DEFAULT;
				break;

			case STATE_COL:
				switch (c) {
				case ':':	//	::
					r.append("&#8759;");
					break;
				case '=':	//	:=
					r.append("&#8788;");
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
				case '`':
					r.append("\\`");
					break;
				case '*':
					r.append("\\*");
					break;
				case '{':
					r.append("\\{");
					break;
				case '}':
					r.append("\\}");
					break;
				case '[':
					r.append("\\[");
					break;
				case ']':
					r.append("\\]");
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
		case STATE_GT:		r.append("\\>");	break;
		case STATE_EQ:		r += '=';	break;
		case STATE_MINUS:	r += '-';	break;
		case STATE_EXCL:	r.append("\\!");	break;
		case STATE_COL:		r += ':';	break;
		default:
			break;
		}

		return r;
	}

}


