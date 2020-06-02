/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/generator.h>
#include "DOCC_project_info.h"

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/dom/reader.h>
#include <cyng/json.h>
#include <cyng/intrinsics/version.h>

#include <boost/algorithm/string.hpp>

namespace docscript
{

	generator::generator(std::vector< boost::filesystem::path > const& inc)
		: uuid_gen_()
		, name_gen_(uuid_gen_())
		, scheduler_()
		, vm_(scheduler_.get_io_service(), uuid_gen_(), std::cout, std::cerr)
		, vars_()
		, const_()
		, includes_(inc)
		, content_table_()
		, meta_()
	{
		register_this();
	}

	generator::~generator()
	{}

	void generator::run(cyng::vector_t&& prg)
	{
		vm_.async_run(std::move(prg));
		vm_.halt();
		scheduler_.stop();
	}

	cyng::param_map_t const& generator::get_meta() const
	{
		return meta_;
	}

	void generator::register_this()
	{
		vm_.register_function("now", 0, [](cyng::context& ctx) {

			//
			//	produce result value
			//
			ctx.push(cyng::make_now());
		});

		//vm_.register_function("generate.meta", 1, std::bind(&gen_md::generate_meta, this, std::placeholders::_1));
		vm_.register_function("generate.index", 1, std::bind(&generator::generate_index, this, std::placeholders::_1));
		vm_.register_function("init.meta.data", 1, std::bind(&generator::init_meta_data, this, std::placeholders::_1));

		vm_.register_function("meta", 1, std::bind(&generator::meta, this, std::placeholders::_1));
		vm_.register_function("set", 1, std::bind(&generator::var_set, this, std::placeholders::_1));
		vm_.register_function("get", 1, std::bind(&generator::var_get, this, std::placeholders::_1));
		vm_.register_function("symbol", 1, std::bind(&generator::print_symbol, this, std::placeholders::_1));
		vm_.register_function("currency", 1, std::bind(&generator::print_currency, this, std::placeholders::_1));
		vm_.register_function("tag", 1, std::bind(&generator::create_uuid, this, std::placeholders::_1));
		vm_.register_function("map", 1, std::bind(&generator::make_map, this, std::placeholders::_1));
		vm_.register_function("version", 1, std::bind(&generator::get_version, this, std::placeholders::_1));

		vm_.register_function("generate.file", 1, std::bind(&generator::generate_file, this, std::placeholders::_1));
		vm_.register_function("generate.meta", 1, std::bind(&generator::generate_meta, this, std::placeholders::_1));

		vm_.register_function("hline", 0, std::bind(&generator::print_hline, this, std::placeholders::_1));

		vm_.register_function("convert.numeric", 1, std::bind(&generator::convert_numeric, this, std::placeholders::_1));
		vm_.register_function("convert.alpha", 1, std::bind(&generator::convert_alpha, this, std::placeholders::_1));

		vm_.register_function("paragraph", 1, std::bind(&generator::paragraph, this, std::placeholders::_1));
		vm_.register_function("abstract", 1, std::bind(&generator::abstract, this, std::placeholders::_1));
		vm_.register_function("quote", 1, std::bind(&generator::quote, this, std::placeholders::_1));
		vm_.register_function("list", 1, std::bind(&generator::list, this, std::placeholders::_1));
		vm_.register_function("link", 1, std::bind(&generator::link, this, std::placeholders::_1));
		vm_.register_function("figure", 1, std::bind(&generator::figure, this, std::placeholders::_1));
		vm_.register_function("gallery", 1, std::bind(&generator::gallery, this, std::placeholders::_1));
		vm_.register_function("code", 1, std::bind(&generator::code, this, std::placeholders::_1));
		vm_.register_function("def", 1, std::bind(&generator::def, this, std::placeholders::_1));
		vm_.register_function("note", 1, std::bind(&generator::annotation, this, std::placeholders::_1));
		vm_.register_function("table", 1, std::bind(&generator::table, this, std::placeholders::_1));
		vm_.register_function("alert", 1, std::bind(&generator::alert, this, std::placeholders::_1));

		vm_.register_function("i", 1, std::bind(&generator::format_italic, this, std::placeholders::_1));
		vm_.register_function("b", 1, std::bind(&generator::format_bold, this, std::placeholders::_1));
		vm_.register_function("bold", 1, std::bind(&generator::format_bold, this, std::placeholders::_1));
		vm_.register_function("tt", 1, std::bind(&generator::format_tt, this, std::placeholders::_1));
		vm_.register_function("color", 1, std::bind(&generator::format_color, this, std::placeholders::_1));
		vm_.register_function("sub", 1, std::bind(&generator::format_sub, this, std::placeholders::_1));
		vm_.register_function("sup", 1, std::bind(&generator::format_sup, this, std::placeholders::_1));
		vm_.register_function("mark", 1, std::bind(&generator::format_mark, this, std::placeholders::_1));

		vm_.register_function("header", 1, std::bind(&generator::header, this, std::placeholders::_1));
		vm_.register_function("h1", 1, std::bind(&generator::section, this, 1, std::placeholders::_1));
		vm_.register_function("h2", 1, std::bind(&generator::section, this, 2, std::placeholders::_1));
		vm_.register_function("h3", 1, std::bind(&generator::section, this, 3, std::placeholders::_1));
		vm_.register_function("h4", 1, std::bind(&generator::section, this, 4, std::placeholders::_1));
		vm_.register_function("h5", 1, std::bind(&generator::section, this, 5, std::placeholders::_1));
		vm_.register_function("h6", 1, std::bind(&generator::section, this, 6, std::placeholders::_1));
		vm_.register_function("footnote", 1, std::bind(&generator::make_footnote, this, std::placeholders::_1));
		vm_.register_function("ref", 1, std::bind(&generator::make_ref, this, std::placeholders::_1));
		vm_.register_function("toc", 1, std::bind(&generator::make_tok, this, std::placeholders::_1));

	}

	void generator::init_meta_data(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		meta_ = cyng::value_cast(frame.at(0), meta_);
	}

	void generator::meta(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
//		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		cyng::param_map_t m;
		m = cyng::value_cast(frame.at(0), m);
		//meta_.insert(m.begin(), m.end());	//	merge
		//	overwrite
		for (auto const& i : m) {
			meta_[i.first] = i.second;
		}
	}

	void generator::slug()
	{
		auto pos = meta_.find("slug");
		if (pos == meta_.end()) {

			//
			//	no slug found - generate one
			//
			pos = meta_.find("title");
			if (pos != meta_.end()) {
				meta_.emplace("slug", generate_slug(pos->second));
			}
			else {
				pos = meta_.find("file-name");
				if (pos != meta_.end()) {
					meta_.emplace("slug", generate_slug(pos->second));
				}
			}
		}
	}

	void generator::var_set(cyng::context& ctx)
	{
		//	example
		//	[%(("language":C++),("name":DocScript))]
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		cyng::param_map_t tmp;
		tmp = cyng::value_cast(frame.at(0), tmp);

		for (auto const& param : tmp) {
			vars_[param.first] = param.second;
		}
		//
		//
		//	no return value
		//
		//ctx.push(frame.at(0));
	}

	void generator::var_get(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const name = cyng::value_cast<std::string>(frame.at(0), "");
		auto const pos = vars_.find(name);
		(pos != vars_.end())	
			? ctx.push(pos->second)
			: ctx.push(frame.at(0))	//	use variable name if value not found
			;
	}

	void generator::generate_index(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		auto const p = cyng::value_cast(frame.at(0), boost::filesystem::path()) / "index.json";
		std::ofstream ofs(p.string(), std::ios::out | std::ios::trunc);
		if (!ofs.is_open())
		{
			std::cerr
				<< "***error cannot open index file ["
				<< p
				<< ']'
				<< std::endl;
		}
		else
		{
			auto const index = serialize(content_table_);
			//std::cout << cyng::io::to_str(index) << std::endl;
			std::string const json = cyng::json::to_string(index);
			//std::cout << json << std::endl;
			ofs << json << std::flush;

		}
	}

	void generator::print_symbol(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const currency = cyng::value_cast<std::string>(frame.at(0), "");
		if (boost::algorithm::equals(currency, "pilcrow")) {
			ctx.push(cyng::make_object(u8"¶"));
		}
		else if (boost::algorithm::equals(currency, "copyright")) {
			ctx.push(cyng::make_object(u8"©"));
		}
		else if (boost::algorithm::equals(currency, "registered")) {
			ctx.push(cyng::make_object(u8"®"));
		}
		else if (boost::algorithm::equals(currency, "ellipsis")) {
			ctx.push(cyng::make_object("..."));
		}
		else {
			ctx.push(cyng::make_object(currency));
		}
	}

	void generator::print_currency(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const currency = cyng::value_cast<std::string>(frame.at(0), "");

		if (boost::algorithm::equals(currency, "euro")) {
			ctx.push(cyng::make_object(u8"¤"));
		}
		else if (boost::algorithm::equals(currency, "yen")) {
			ctx.push(cyng::make_object(u8"¥"));
		}
		else if (boost::algorithm::equals(currency, "pound")) {
			ctx.push(cyng::make_object(u8"£"));
		}
		else if (boost::algorithm::equals(currency, "rupee")) {
			ctx.push(cyng::make_object(u8"?"));
		}
		else if (boost::algorithm::equals(currency, "sheqel")) {
			ctx.push(cyng::make_object(u8"?"));
		}
		else {
			ctx.push(cyng::make_object(currency));
		}
	}

	void generator::create_uuid(cyng::context& ctx)
	{
		ctx.push(cyng::make_object(uuid_gen_()));
	}

	void generator::make_map(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
// 		std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;

		switch (frame.at(0).get_class().tag()) {
		case cyng::TC_PARAM_MAP:
			ctx.push(frame.at(0));
			break;
		default:
			ctx.push(cyng::param_map_factory("error", frame.at(0).get_class().type_name())());
			break;
		}
	}

	void generator::get_version(cyng::context& ctx)
	{
		ctx.push(cyng::make_object(cyng::revision(DOCC_VERSION_MAJOR, DOCC_VERSION_MINOR, DOCC_VERSION_PATCH, DOCC_VERSION_TWEAK)));
	}

	boost::filesystem::path generator::resolve_path(std::string const& s) const
	{
		boost::filesystem::path p(s);
		for (auto dir : includes_)
		{
			if (boost::filesystem::exists(dir / p))
			{
				return boost::filesystem::path(dir / p);
			}
		}
		return boost::filesystem::path();
	}

	std::string generator::get_type() const
	{
		auto const reader = cyng::make_reader(meta_);
		return cyng::value_cast<std::string>(reader.get("og:type"), "report");
	}

	bool generator::is_report() const
	{
		auto const type = get_type();
		return boost::algorithm::equals(type, "report");
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

	cyng::io::language_codes generator::get_language_code() const
	{
		auto const lang = get_language();
		return (lang.size() == 2)
			? cyng::io::get_language_code(lang.at(0), lang.at(1))
			: cyng::io::LC_EN
			;
	}

	std::string generator::get_name(i18n::word_id id) const
	{
		switch(id) {
		case i18n::WID_FIGURE:
			switch(get_language_code()) {
			case cyng::io::LC_ES: return "gráfica";
			case cyng::io::LC_SV: return "illustration";
			case cyng::io::LC_PT: return "ilustração";
			case cyng::io::LC_DE: return "Abbildung";
			case cyng::io::LC_BG: return "??????????";
			case cyng::io::LC_RU: return "????????";
			case cyng::io::LC_UK: return "??????????";
//			case cyng::io::LC_CS: return "czech";
			case cyng::io::LC_FI: return "kuva";
			case cyng::io::LC_FR: return "illustration";
			case cyng::io::LC_EL: return "??????";
//			case cyng::io::LC_HU: return "magyar";
			case cyng::io::LC_IS: return "útskýring";
			case cyng::io::LC_IT: return "figura";
			case cyng::io::LC_NN: return "illustrasjon";
			case cyng::io::LC_JA: return "????";
			case cyng::io::LC_KO: return "??";
			case cyng::io::LC_FA: return "?????";
			case cyng::io::LC_PL: return "rysunek";
			case cyng::io::LC_SK: return "ilustrácie";
			case cyng::io::LC_HE: return "????";
			case cyng::io::LC_NL: return "afbeelding";
			case cyng::io::LC_EU: return "ilustrazioa";
//			case cyng::io::LC_BR: return "breton";
//			case cyng::io::LC_CA: return "catalan";
			case cyng::io::LC_HR: return "ilustracija";
			case cyng::io::LC_ET: return "näide";
			case cyng::io::LC_GL: return "ilustración";
			case cyng::io::LC_GA: return "léaráid";
			case cyng::io::LC_LA: return "illustratio";
//			case cyng::io::LC_SE: return "samin";
			case cyng::io::LC_RO: return "ilustrare";
			case cyng::io::LC_GD: return "dealbh";
			case cyng::io::LC_TR: return "resim";
			case cyng::io::LC_CY: return "darlunio";
			default:
				break;
			}
			return "figure";
		case i18n::WID_TABLE:
			switch(get_language_code()) {
			case cyng::io::LC_ES: return "mesa";
			case cyng::io::LC_SV: return "bord";
			case cyng::io::LC_PT: return "mesa";
			case cyng::io::LC_DE: return "Tabelle";
			case cyng::io::LC_BG: return "????";
			case cyng::io::LC_RU: return "????";
			case cyng::io::LC_UK: return "table";
//			case cyng::io::LC_CS: return "czech";
			case cyng::io::LC_FI: return "pöytä";
			case cyng::io::LC_FR: return "table";
			case cyng::io::LC_EL: return "???????";
			case cyng::io::LC_HU: return "táblázat";
			case cyng::io::LC_IS: return "Taflan";
			case cyng::io::LC_IT: return "tavolo";
			case cyng::io::LC_NN: return "bord";
			case cyng::io::LC_JA: return "????";
			case cyng::io::LC_KO: return "???";
			case cyng::io::LC_FA: return "????";
			case cyng::io::LC_PL: return "stó?";
			case cyng::io::LC_SK: return "stôl";
			case cyng::io::LC_HE: return "?????";
			case cyng::io::LC_NL: return "tafel";
			case cyng::io::LC_EU: return "taula";
//			case cyng::io::LC_BR: return "breton";
//			case cyng::io::LC_CA: return "catalan";
			case cyng::io::LC_HR: return "stol";
			case cyng::io::LC_ET: return "tabel";
			case cyng::io::LC_GL: return "mesa";
			case cyng::io::LC_GA: return "tábla";
			case cyng::io::LC_LA: return "mensam";
//			case cyng::io::LC_SE: return "samin";
			case cyng::io::LC_RO: return "tabel";
			case cyng::io::LC_GD: return "Clàr";
			case cyng::io::LC_TR: return "tablo";
			case cyng::io::LC_CY: return "tabl";
			default:
				break;
			}
			return "table";
		case i18n::WID_TOC:
			switch (get_language_code()) {
			case cyng::io::LC_ES: return "Tabla de contenido";
			case cyng::io::LC_SV: return "Innehållsförteckning";
			case cyng::io::LC_PT: return "Índice";	//	Portuguese
			case cyng::io::LC_DE: return "Inhaltsverzeichnis";
			//case cyng::io::LC_BG: return "Съдържание";
			//case cyng::io::LC_RU: return "Оглавление";
			case cyng::io::LC_UK: return "Table of Contents";
				//			case cyng::io::LC_CS: return "czech";
			case cyng::io::LC_FI: return "Sisällysluettelo";	//	Finnish
			case cyng::io::LC_FR: return "Table des matières";	//	French
			//case cyng::io::LC_EL: return "Πίνακας περιεχομένων";	//	greek
				//			case cyng::io::LC_HU: return "magyar";
			case cyng::io::LC_IS: return "Efnisyfirlit";	//	Icelandic
			case cyng::io::LC_IT: return "Sommario";	//	Italian
			case cyng::io::LC_NN: return "Innholdsfortegnelse";	//	Norwegian
			//case cyng::io::LC_JA: return "目次";
			//case cyng::io::LC_KO: return "목차";
			case cyng::io::LC_FA: return "?????";
			case cyng::io::LC_PL: return "??????????";
			case cyng::io::LC_SK: return "??????????";
			case cyng::io::LC_HE: return "????";
			case cyng::io::LC_NL: return "??????????";
			case cyng::io::LC_EU: return "??????????";
				//			case cyng::io::LC_BR: return "breton";
				//			case cyng::io::LC_CA: return "catalan";
			case cyng::io::LC_HR: return "??????????";
			case cyng::io::LC_ET: return "??????????";
			case cyng::io::LC_GL: return "??????????";
			case cyng::io::LC_GA: return "??????????";
			case cyng::io::LC_LA: return "Table of Contents";	//	Latin
				//			case cyng::io::LC_SE: return "samin";
			case cyng::io::LC_RO: return "Cuprins";
			case cyng::io::LC_GD: return "Táboa de contidos";		//	Gaelic
			//case cyng::io::LC_TR: return "İçindekiler";		//	Turkish
			case cyng::io::LC_CY: return "Tabl Cynnwys";	//	Welsh
			default:
				break;
			}
			return "Table of Contents";
		default:
			break;
		}
		return "";
	}

	std::string get_extension(boost::filesystem::path const& p)
	{
		if (p.empty())
		{
			return "";
		}
		std::string s = p.extension().string();
		return s.substr(1);
	}

	std::string accumulate_plain_text(cyng::object obj)
	{
		switch (obj.get_class().tag()) {
		case cyng::TC_TUPLE:
			return accumulate_plain_text(cyng::value_cast(obj, cyng::tuple_t()));
		case cyng::TC_VECTOR:
			return accumulate_plain_text(cyng::value_cast(obj, cyng::vector_t()));
		case cyng::TC_NULL:
			return "";
		case cyng::TC_TIME_POINT:
			return cyng::to_str(cyng::value_cast(obj, std::chrono::system_clock::now()));
		default:
			break;
		}
		return cyng::io::to_str(obj);
	}

	std::string accumulate_plain_text(cyng::tuple_t tpl)
	{
		std::string text;
		for (auto const& obj : tpl) {
			std::string const s = accumulate_plain_text(obj);
			if (!text.empty() 
				&& !(s == "." || s == "," || s == ":" || s == "?" || s == "!" || s == ")" || s == "]" || s == "}")
				&& !(text.back() == '(' || text.back() == '[' || text.back() == '{')) {
				text.append(1, ' ');
			}
			text.append(s);
		}
		return text;
	}

	std::string accumulate_plain_text(cyng::vector_t vec)
	{
		std::string text;
		for (auto const& obj : vec) {
			std::string const s = accumulate_plain_text(obj);
			if (!text.empty() 
				&& !(s == "." || s == "," || s == ":" || s == "?" || s == "!" || s == ")" || s == "]" || s == "}")
				&& !(text.back() == '(' || text.back() == '[' || text.back() == '{')) {
				text.append(1, ' ');
			}
			text.append(s);
		}
		return text;
	}

	std::string generate_slug(std::string title)
	{
		std::string slug;
		for (auto const& c : title) {
			if ((c > 47 && c < 58) || (c > 96 && c < 123)) {
				slug += c;
			}
			else if (c > 64 && c < 91) {
				slug += (c + 32);
			}
			else if (c == '-' || c == ' '|| c == '\\'|| c == '/') {
				if (!slug.empty() && slug.back() != '-') {
					slug += '-';
				}
			}
		}
		return slug;
	}

	cyng::object generate_slug(cyng::object obj)
	{
		auto const title = cyng::value_cast<std::string>(obj, "no-title");
		return cyng::make_object(generate_slug(title));
	}

}


