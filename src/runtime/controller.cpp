
#include "controller.h"

#include <cyng/task/controller.h>
#include <cyng/task/scheduler.h>
#include <cyng/vm/vm.h>
#include <cyng/vm/mesh.h>
#include <cyng/io/parser/parser.h>
#include <cyng/io/ostream.h>
#include <cyng/io/serialize.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>

namespace docscript {

	void show(std::string str) {
		std::cout << str << std::endl;
	}

	controller::controller(std::filesystem::path out
		, int verbose)
		: vars_()
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
		auto vm = fabric.create_proxy(tag
			, f_quote()
			, f_set()
			, f_get()
			, f_meta()
			, f_paragraph()
			, f_italic()
			, f_bold()
			, f_typewriter()
			, f_label()
			, f_ref()
			, f_h1()
			, f_h2()
			, f_h3()
			, f_h4()
			, f_h5()
			, f_h6()
			, f_header()
			, f_resource()
			, f_now()
			, f_range()
			, f_cat()
			, f_repeat()
			, f_currency()
			, f_show()
		);
		std::size_t slot{ 0 };
		vm.set_channel_name("quote", slot++);
		vm.set_channel_name("set", slot++);
		vm.set_channel_name("get", slot++);
		vm.set_channel_name("meta", slot++);
		vm.set_channel_name(std::string("\xc2\xb6"), slot++);	//	paragraph
		vm.set_channel_name("i", slot++);	//	italic
		vm.set_channel_name("b", slot++);	//	bold
		vm.set_channel_name("tt", slot++);	//	typewriter
		vm.set_channel_name("label", slot++);	//	label
		vm.set_channel_name("ref", slot++);	//	ref
		vm.set_channel_name("h1", slot++);	//	h1
		vm.set_channel_name("h2", slot++);	//	h2
		vm.set_channel_name("h3", slot++);	//	h3
		vm.set_channel_name("h4", slot++);	//	h4
		vm.set_channel_name("h5", slot++);	//	h5
		vm.set_channel_name("h6", slot++);	//	h6
		vm.set_channel_name("header", slot++);	//	header
		vm.set_channel_name("resource", slot++);	//	resource
		vm.set_channel_name("now", slot++);	//	current time
		vm.set_channel_name("range", slot++);	//	build an vector
		vm.set_channel_name("cat", slot++);	//	concatenate without spaces
		vm.set_channel_name("repeat", slot++);
		vm.set_channel_name("currency", slot++);
		vm.set_channel_name("show", slot++);	//	show

		//
		//	load programe
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

		auto const delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now);

		fmt::print(
			stdout,
			fg(fmt::color::forest_green),
			"***info : complete after {} milliseconds\n", delta.count());

		return EXIT_SUCCESS;
		//return EXIT_FAILURE;

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
		return ss.str();
	}
	std::string controller::h3(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H3(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	std::string controller::h4(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H4(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	std::string controller::h5(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H5(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	std::string controller::h6(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "H6(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	std::string controller::header(cyng::param_map_t pm) {
		//std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "HEADER(" << pm << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
	}
	void controller::resource(cyng::param_map_t pm) {
		std::cout << "RESOURCE(" << pm << ")" << std::endl;
	}
	std::chrono::system_clock::time_point controller::now(cyng::param_map_t pm) {
		std::stringstream ss;
		std::cout << "NOW(" << pm << ")" << std::endl;
		return std::chrono::system_clock::now();
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
		std::cout << ss.str() << std::endl;
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
		return std::string("42 \xE2\x82\xAC");	//	euro
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
	std::function<void(cyng::param_map_t)> controller::f_resource() {
		return std::bind(&controller::resource, this, std::placeholders::_1);
	}
	std::function<std::chrono::system_clock::time_point(cyng::param_map_t)> controller::f_now() {
		return std::bind(&controller::now, this, std::placeholders::_1);
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
