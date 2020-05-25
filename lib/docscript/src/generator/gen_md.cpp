/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/gen_md.h>
#include "filter/binary_to_md.h"

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/numeric_cast.hpp>
#include <cyng/dom/reader.h>
#include <cyng/csv.h>
#include <cyng/io/bom.h>
#include <cyng/set_cast.h>

#include <boost/algorithm/string.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/nil_generator.hpp>

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


	}

	void gen_md::generate_file(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto p = cyng::value_cast(frame.at(0), boost::filesystem::path());
		if (boost::filesystem::is_directory(p)) {
			p = p / "out.md";
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

	std::ofstream& gen_md::emit_file(std::ofstream& ofs, cyng::vector_t::const_iterator pos, cyng::vector_t::const_iterator end) const
	{
		emit_meta(ofs);
		while (pos != end) {
			if (pos->get_class().tag() == cyng::TC_UUID) {
				emit_intrinsic(ofs, cyng::value_cast(*pos, boost::uuids::nil_uuid()));
			}
			else {
				emit_obj(ofs, *pos);
			}
			++pos;
		}
		emit_footnotes(ofs);
		return ofs;
	}

	std::ofstream& gen_md::emit_intrinsic(std::ofstream& ofs, boost::uuids::uuid tag) const
	{

		if (tag == name_gen_("[ToC]")) {
			emit_toc(ofs, 5);
		}
		else if (tag == name_gen_("[ToC-1]")) {
			emit_toc(ofs, 1);
		}
		else if (tag == name_gen_("[ToC-2]")) {
			emit_toc(ofs, 2);
		}
		else if (tag == name_gen_("[ToC-3]")) {
			emit_toc(ofs, 3);
		}
		else if (tag == name_gen_("[ToC-4]")) {
			emit_toc(ofs, 4);
		}
		else {
			ofs
				<< "***ERROR: unknown intrinsic: "
				<< boost::uuids::to_string(tag)
				<< std::endl
				;
		}
		return ofs;
	}

	std::ofstream& gen_md::emit_toc(std::ofstream& ofs, std::size_t depth) const
	{

		ofs
			<< "# "
			<< get_name(i18n::WID_TOC)
			<< std::endl
			;

		auto const toc = serialize(content_table_);
		emit_toc(ofs, toc, 0, depth);

		ofs
			<< std::endl
			;

		return ofs;
	}

	std::ofstream& gen_md::emit_toc(std::ofstream& ofs, cyng::vector_t const& toc, std::size_t level, std::size_t depth) const
	{
		bool const descend = level + 1 < depth;
		for (auto const& header : toc) {

			auto const h = cyng::to_param_map(header);
			auto const title = cyng::io::to_str(h.at("title"));
			auto const number = cyng::io::to_str(h.at("number"));
			auto const tag = cyng::value_cast(h.at("tag"), boost::uuids::nil_uuid());
			auto const href = "#" + boost::uuids::to_string(tag);

			ofs
				<< std::string(level, '\t')
				<< '-'
				<< ' '
				<< '['
				<< number
				<< ' '
				<< title
				<< ']'
				<< '('
				<< href
				<< ')'
				<< std::endl
				;

			auto const pos = h.find("sub");
			if (descend && (pos != h.end())) {

				auto const sub = cyng::to_vector(pos->second);
				emit_toc(ofs, sub, level + 1, depth);

			}
		}
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

	std::ofstream& gen_md::emit_meta(std::ofstream& ofs) const
	{
		//	meta data nor supported by github
		for (auto const& e : meta_) {
			ofs
				<< "<!-- "
				<< e.first
				<< ":\t"
				<< cyng::io::to_str(e.second)
				<< " -->"
				<< std::endl
				;
		}

		ofs << std::endl;
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
					<< note.get_text()
					<< std::endl
				;

			}
		}
		return ofs;
	}

	void gen_md::generate_meta(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

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

	void gen_md::print_hline(cyng::context& ctx)
	{
		//auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		ctx.push(cyng::make_object("\n" + std::string(3, '-')));
	}

	void gen_md::convert_numeric(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		ctx.push(frame.at(0));
	}

	void gen_md::convert_alpha(cyng::context& ctx)
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
		ctx.push(cyng::make_object(replace_md_entities(str)));
	}

	void gen_md::print_symbol(cyng::context& ctx)
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

	void gen_md::print_currency(cyng::context& ctx)
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

	void gen_md::paragraph(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		std::string par = accumulate_plain_text(frame);
		ctx.push(cyng::make_object(par));
	}

	void gen_md::abstract(cyng::context& ctx)
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
	
	void gen_md::quote(cyng::context& ctx)
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

	void gen_md::list(cyng::context& ctx)
	{
		//	[%(("items":[<p>one </p>,<p>two </p>,<p>three </p>]),("style":{lower-roman}),("type":ordered))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

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
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const reader = cyng::make_reader(frame.at(0));
		auto const text = accumulate_plain_text(reader.get("text"));
		auto const url = cyng::io::to_str(reader.get("url"));
		auto const title = accumulate_plain_text(reader.get("title"));

		std::stringstream ss;
		ss
			<< '['
			<< text
			<< "]("
			<< url
			<< ' '
			<< '"'
			<< title
			<< '"'
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
		auto const width = cyng::value_cast(reader.get("width"), 1.0);	//	no chance in github  - works with pixel units only

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

	void gen_md::gallery(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
	}

	void gen_md::code(cyng::context& ctx)
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
			if (boost::algorithm::equals(language, "bin") || boost::algorithm::iequals(language, "binary")) {
				// binary mode required
				std::ifstream  ifs(p.string(), std::ios::binary);
				ifs.unsetf(std::ios::skipws);
				cyng::buffer_t const inp((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
				binary_to_md filter(line_numbers, uuid_gen_());
				filter.convert(ss, inp);
			}
			else {
				std::ifstream  ifs(p.string());
				ss
					<< std::string(3, '`')
					<< language
					<< std::endl
					<< ifs.rdbuf()
					<< std::endl
					<< std::string(3, '`')
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

	void gen_md::def(cyng::context& ctx)
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
				<< replace_md_entities(def.first)
				<< std::endl
				<< ':'
				<< ' '
				<< replace_md_entities(text)
				<< std::endl
				;
		}

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::annotation(cyng::context& ctx)
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

	void gen_md::table(cyng::context& ctx)
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

			//
			//	parse the CSV file into a vector
			//
			auto const csv = cyng::csv::read_file(p.string());
			std::stringstream ss;
			ss << std::endl;

			bool initial{ true };
			for (auto const& row : csv) {
				cyng::tuple_t tpl;
				tpl = cyng::value_cast(row, tpl);
				for (auto const& cell : tpl) {
					ss << "| " << cyng::io::to_str(cell);
				}
				ss << " |" << std::endl;
				if (initial) {
					//| ------------ - | ------------ - |
					for (auto const& cell : tpl) {
						ss << "| " << std::string(cyng::io::to_str(cell).size(), '-');
					}
					ss << " |" << std::endl;
					initial = false;
				}
			}

			ctx.push(cyng::make_object(ss.str()));
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

	void gen_md::alert(cyng::context& ctx)
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

	void gen_md::make_ref(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object(cyng::io::to_str(frame)));
	}

	void gen_md::make_tok(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		auto const reader = cyng::make_reader(frame.at(0));
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("depth"), 3u);

		switch (level) {
		case 1:
			ctx.push(cyng::make_object(name_gen_("[ToC-1]")));
			break;
		case 2:
			ctx.push(cyng::make_object(name_gen_("[ToC-2]")));
			break;
		case 3:
			ctx.push(cyng::make_object(name_gen_("[ToC-3]")));
			break;
		case 4:
			ctx.push(cyng::make_object(name_gen_("[ToC-4]")));
			break;
		default:
			ctx.push(cyng::make_object(name_gen_("[ToC]")));
			break;
		}

	}

	void gen_md::format_italic(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		ctx.push(cyng::make_object("_" + accumulate_plain_text(frame) + "_"));
	}

	void gen_md::format_bold(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		ctx.push(cyng::make_object("**" + accumulate_plain_text(frame) + "**"));
	}

	void gen_md::format_tt(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		ctx.push(cyng::make_object("`" + accumulate_plain_text(frame) + "`"));
	}

	void gen_md::format_color(cyng::context& ctx)
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

	void gen_md::format_sub(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//	pandoc style
		ctx.push(cyng::make_object("~" + accumulate_plain_text(frame) + "~"));
	}

	void gen_md::format_sup(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		//	pandoc style
		ctx.push(cyng::make_object("^" + accumulate_plain_text(frame) + "^"));
	}
	
	void gen_md::format_mark(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//	use HTML5
		ctx.push(cyng::make_object("<mark>" + accumulate_plain_text(frame) + "</mark>"));
	}	

	void gen_md::header(cyng::context& ctx)
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
			<< "<a id=\""
			<< id
			<< "\"></a>"
			<< std::endl 
			<< std::string(level, '#')
			<< ' '
			<< number
			<< ' '
			<< title
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::section(int level, cyng::context& ctx)
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
			<< "<a id=\""
			<< id
			<< "\"></a>"
			<< std::endl
			<< std::string(level, '#')
			<< ' '
			<< number
			<< ' '
			<< title
			;

		ctx.push(cyng::make_object(ss.str()));
	}

	void gen_md::make_footnote(cyng::context& ctx)
	{
		//	[%(("level":1),("tag":{79bf3ba0-2362-4ea5-bcb5-ed93844ac59a}),("title":{Basics}))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const note = accumulate_plain_text(frame);
		auto const tag = uuid_gen_();
		footnotes_.emplace_back( tag, note );
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

	std::string replace_md_entities(std::string const& str)
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


