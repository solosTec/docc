/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/generator.h>

#include <cyng/vm/generator.h>
#include <cyng/io/serializer.h>
#include <cyng/value_cast.hpp>
#include <cyng/dom/reader.h>
#include <cyng/json.h>

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
		, language_("en")
		, includes_(inc)
		, content_table_()
		, meta_()
	{
		register_this();
	}

	void generator::run(cyng::vector_t&& prg)
	{
		vm_.async_run(std::move(prg));
		vm_.halt();
		scheduler_.stop();
	}

	void generator::register_this()
	{
		vm_.register_function("now", 0, [this](cyng::context& ctx) {

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
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		cyng::param_map_t m;
		m = cyng::value_cast(frame.at(0), m);
		meta_.insert(m.begin(), m.end());	//	merge
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
		auto const symbol = cyng::value_cast<std::string>(frame.at(0), "");
		if (boost::algorithm::equals(symbol, "pilgrow")) {
			ctx.push(cyng::make_object(u8"¶"));
		}
		else if (boost::algorithm::equals(symbol, "copyright")) {
			ctx.push(cyng::make_object(u8"©"));
		}
		else if (boost::algorithm::equals(symbol, "registered")) {
			ctx.push(cyng::make_object(u8"®"));
		}
		else {
			ctx.push(cyng::make_object(symbol));
		}
	}

	void generator::print_currency(cyng::context& ctx)
	{
		auto const frame = ctx.get_frame();
		//std::cout << ctx.get_name() << " - " << cyng::io::to_str(frame) << std::endl;
		auto const currency = cyng::value_cast<std::string>(frame.at(0), "");

		if (boost::algorithm::equals(currency, "euro")) {
			ctx.push(cyng::make_object(u8""));
		}
		else if (boost::algorithm::equals(currency, "yen")) {
			ctx.push(cyng::make_object(u8"¥"));
		}
		else if (boost::algorithm::equals(currency, "pound")) {
			ctx.push(cyng::make_object(u8"£"));
		}
		else {
			ctx.push(cyng::make_object(currency));
		}
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
			if (!text.empty() && !(s == "." || s == "," || s == ":" || s == "?" || s == "!" || s == ")" || s == "]" || s == "}")) {
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
			if (!text.empty() && !(s == "." || s == "," || s == ":" || s == "?" || s == "!" || s == ")" || s == "]" || s == "}")) {
				text.append(1, ' ');
			}
			text.append(s);
		}
		return text;
	}

	void replace_all(std::string& str, std::string old, std::string rep)
	{
		for (int pos = 0;
			(pos = str.find(old, pos)) != std::string::npos;
			pos += rep.length())
		{
			str.replace(pos, old.length(), rep);
		}
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

}


