
#include "controller.h"
#include <currency.h>

#include <cyng/task/controller.h>
#include <cyng/task/scheduler.h>
#include <cyng/vm/vm.h>
#include <cyng/vm/mesh.h>
#include <cyng/io/parser/parser.h>
#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>
#include <cyng/obj/algorithm/reader.hpp>
#include <cyng/obj/numeric_cast.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>

namespace docruntime {

	void show(std::string str) {
		std::cout << str << std::endl;
	}

	controller::controller(std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, int verbose)
		: vars_()
		, toc_()
		, uuid_gen_()
	{}

	int controller::run(std::filesystem::path&& inp
		, std::size_t pool_size
		, boost::uuids::uuid tag) {

		auto const now = std::chrono::high_resolution_clock::now();

		//
		//	Create an scheduler with specified size
		//	of the thread pool.
		//
		cyng::controller ctl(pool_size);
		cyng::mesh fabric(ctl);

		//
		//	Create VM
		//
		auto vm = fabric.make_proxy(tag
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
			, cyng::make_description("cat", f_cat())
			, cyng::make_description("repeat", f_repeat())
			, cyng::make_description("currency", f_currency())
			, cyng::make_description("show", f_show())
		);

		//
		//	load program
		//
		std::ifstream ifs(inp.string(), std::ios::binary);
		if (ifs.is_open()) {
			ifs.unsetf(std::ios::skipws);

			cyng::buffer_t buffer;
			
			ifs.seekg(0, std::ios::end);
			buffer.reserve(ifs.tellg());
			ifs.seekg(0, std::ios::beg);


			auto pos = std::istream_iterator<char>(ifs);
			auto end = std::istream_iterator<char>();

			buffer.insert(buffer.begin(), pos, end);

			cyng::deque_t deq;
			cyng::io::parser p([&](cyng::object&& obj) -> void {
				std::cout << cyng::io::to_typed(obj) << std::endl;
				deq.push_back(std::move(obj));
				});
			p.read(std::begin(buffer), std::end(buffer));


			//
			//	execute program
			// 
			vm.load(std::move(deq));
			vm.run();

		}
		else {
			fmt::print(
				stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : input file [{}] not found\n", inp.string());
		}

		//
		//	wait for pending requests
		//
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		vm.stop();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		ctl.cancel();
		ctl.stop();
		//ctl.shutdown();

		std::cout << "TOC:" << std::endl;
		std::cout << toc_ << std::endl;

		//	JSON
		//auto const vec = to_vector(toc_);
		//std::cout << cyng::io::to_json_pretty(cyng::make_object(vec)) << std::endl;

		auto const delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now);

		fmt::print(
			stdout,
			fg(fmt::color::forest_green),
			"***info : complete after {} milliseconds\n", delta.count());

		return EXIT_SUCCESS;
	}

	std::string controller::quote(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "QUOTE(\"";
		bool init = false;
		for (auto const& v : vec) {
			if (init) {
				ss << ' ';
			}
			else {
				init = true;
			}
			ss << v;
		}
		ss << "\")";
		//std::cout << ss.str() << std::endl;
		return ss.str();
	}

	std::string controller::italic(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "ITALIC(" << vec << ")";
		//std::cout << ss.str() << std::endl;
		return ss.str();
	}

	std::string controller::bold(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "BOLD(" << vec << ")";
		return ss.str();
	}

	std::string controller::typewriter(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "TYPEWRITER(" << vec << ")";
		return ss.str();
	}

	void controller::set(cyng::param_map_t pm) {
		std::cout << "SET(" << pm << ")" << std::endl;
		vars_.insert(pm.begin(), pm.end());
	}
	void controller::meta(cyng::param_map_t pm) {
		std::cout << "META(" << pm << ")" << std::endl;
	}

	cyng::vector_t controller::get(cyng::vector_t vec) {
		cyng::vector_t res;
		for (auto const& v : vec) {
			auto const pos = vars_.find(cyng::io::to_plain(v));
			if (pos != vars_.end()) {
				res.push_back(pos->second);
			}
		}
		return res;
	}

	std::string controller::paragraph(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "PARAGRAPH*" << vec.size() << "(";
		bool init = false;
		for (auto const& v : vec) {
			if (init) {
				ss << ' ';
			}
			else {
				init = true;
			}
			ss << v;
		}
		ss << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}

	void controller::label(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "LABEL(" << vec << ")";
		std::cout << ss.str() << std::endl;
	}

	std::string controller::ref(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "REF(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	std::string controller::h1(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H1*" << vec.size() << "(";
		bool init = false;
		for (auto const& v : vec) {
			if (init) {
				ss << ' ';
			}
			else {
				init = true;
			}
			ss << v;
		}
		ss << ")";
		std::cout << ss.str() << std::endl;
		toc_.add(0, uuid_gen_(), cyng::to_string(vec));
		return ss.str();
	}
	std::string controller::h2(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H2*" << vec.size() << "(";
		bool init = false;
		for (auto const& v : vec) {
			if (init) {
				ss << ' ';
			}
			else {
				init = true;
			}
			ss << v;
		}
		ss << ")";
		std::cout << ss.str() << std::endl;
		toc_.add(1, uuid_gen_(), cyng::to_string(vec));
		return ss.str();
	}
	std::string controller::h3(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H3(" << vec << ")";
		std::cout << ss.str() << std::endl;
		toc_.add(2, uuid_gen_(), cyng::to_string(vec));
		return ss.str();
	}
	std::string controller::h4(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H4(" << vec << ")";
		std::cout << ss.str() << std::endl;
		toc_.add(3, uuid_gen_(), cyng::to_string(vec));
		return ss.str();
	}
	std::string controller::h5(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H5(" << vec << ")";
		std::cout << ss.str() << std::endl;
		toc_.add(4, uuid_gen_(), cyng::to_string(vec));
		return ss.str();
	}
	std::string controller::h6(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H6(" << vec << ")";
		std::cout << ss.str() << std::endl;
		toc_.add(5, uuid_gen_(), cyng::to_string(vec));
		return ss.str();
	}
	std::string controller::header(cyng::param_map_t pm) {
		std::stringstream ss;
		ss << "HEADER(" << pm << ")";
		std::cout << ss.str() << std::endl;

		//	"level":0000000000000001),("tag":<uuid>'79bf3ba0-2362-4ea5-bcb5-ed93844ac59a'),("title":[Basics]))
		auto const reader = cyng::make_reader(pm);
		auto const level = cyng::numeric_cast<std::size_t>(reader.get("level"), 0);
		auto const tag = cyng::value_cast(reader.get("tag"), uuid_gen_());
		auto const title = cyng::io::to_plain(reader.get("title"));

		toc_.add(level, tag, title);
		return ss.str();
	}

	std::string controller::figure(cyng::param_map_t pm) {
		std::stringstream ss;
		ss << "FIGURE(" << pm << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}

	void controller::resource(cyng::param_map_t pm) {
		std::cout << "RESOURCE(" << pm << ")" << std::endl;
	}
	std::chrono::system_clock::time_point controller::now(cyng::param_map_t pm) {
		//std::stringstream ss;
		//std::cout << "NOW(" << pm << ")" << std::endl;
		return std::chrono::system_clock::now();
	}
	boost::uuids::uuid controller::uuid(cyng::param_map_t) {
		return uuid_gen_();
	}

	cyng::vector_t controller::range(cyng::vector_t vec) {
		return vec;
	}
	std::string controller::cat(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "CAT*" << vec.size() << "(";
		for (auto const& v : vec) {
			ss << v;
		}
		ss << ")";
		//std::cout << ss.str() << std::endl;
		return ss.str();
	}

	//		insert_method(table, method("repeat", parameter_type::MAP, true, { "count", "value", "sep"}));
	std::string controller::repeat(cyng::param_map_t pm) {
		return "";
	}

	std::string controller::currency(cyng::param_map_t pm) {
		//	https://www.fileformat.info/info/unicode/category/Sc/list.htm
		//	https://www.w3schools.com/charsets/ref_utf_currency.asp
		//return std::string("\xE2\x82\xB9");	//	indian rupee

		auto const reader = cyng::make_reader(pm);
		auto const value = cyng::numeric_cast<std::size_t>(reader.get("value"), 0);
		auto const name = cyng::value_cast<std::string>(reader.get("name"), "euro");

		return docruntime::currency(value, name);
	}

	std::function<std::string(cyng::vector_t)> controller::f_quote() {
		return std::bind(&controller::quote, this, std::placeholders::_1);
	}

	std::function<void(cyng::param_map_t)> controller::f_set() {
		return std::bind(&controller::set, this, std::placeholders::_1);
	}
	std::function<cyng::vector_t(cyng::vector_t)> controller::f_get() {
		return std::bind(&controller::get, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> controller::f_meta() {
		return std::bind(&controller::meta, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::vector_t)> controller::f_paragraph() {
		return std::bind(&controller::paragraph, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_italic() {
		return std::bind(&controller::italic, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_bold() {
		return std::bind(&controller::bold, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_typewriter() {
		return std::bind(&controller::typewriter, this, std::placeholders::_1);
	}
	std::function<void(cyng::vector_t)> controller::f_label() {
		return std::bind(&controller::label, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_ref() {
		return std::bind(&controller::ref, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h1() {
		return std::bind(&controller::h1, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h2() {
		return std::bind(&controller::h2, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h3() {
		return std::bind(&controller::h3, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h4() {
		return std::bind(&controller::h4, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h5() {
		return std::bind(&controller::h5, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_h6() {
		return std::bind(&controller::h6, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::param_map_t)> controller::f_header() {
		return std::bind(&controller::header, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::param_map_t)> controller::f_figure() {
		return std::bind(&controller::figure, this, std::placeholders::_1);
	}
	std::function<void(cyng::param_map_t)> controller::f_resource() {
		return std::bind(&controller::resource, this, std::placeholders::_1);
	}
	std::function<std::chrono::system_clock::time_point(cyng::param_map_t)> controller::f_now() {
		return std::bind(&controller::now, this, std::placeholders::_1);
	}
	std::function<boost::uuids::uuid(cyng::param_map_t)> controller::f_uuid() {
		return std::bind(&controller::uuid, this, std::placeholders::_1);
	}
	std::function<cyng::vector_t(cyng::vector_t)> controller::f_range() {
		return std::bind(&controller::range, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::vector_t)> controller::f_cat() {
		return std::bind(&controller::cat, this, std::placeholders::_1);
	}
	std::function<std::string(cyng::param_map_t pm)> controller::f_repeat() {
		return std::bind(&controller::repeat, this, std::placeholders::_1);
	}

	std::function<std::string(cyng::param_map_t)> controller::f_currency() {
		return std::bind(&controller::currency, this, std::placeholders::_1);
	}

	std::function<void(std::string)> controller::f_show() {
		return std::bind(&show, std::placeholders::_1);
	}

	std::filesystem::path verify_extension(std::filesystem::path p, std::string const& ext)
	{
		if (!p.has_extension())
		{
			p.replace_extension(ext);
		}
		return p;
	}

}
