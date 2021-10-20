
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

namespace docscript {

	void show(std::string str) {
		std::cout << str << std::endl;
	}

	controller::controller(std::filesystem::path out
		, int verbose)
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
		std::function<std::string(cyng::vector_t)> f1 = std::bind(&controller::quote, this, std::placeholders::_1);
		std::function<void(cyng::param_map_t)> f2 = std::bind(&controller::set, this, std::placeholders::_1);
		std::function<std::string(cyng::vector_t)> f3 = std::bind(&controller::paragraph, this, std::placeholders::_1);
		std::function<std::string(cyng::vector_t)> f4 = std::bind(&controller::italic, this, std::placeholders::_1);
		std::function<std::string(cyng::vector_t)> f5 = std::bind(&controller::bold, this, std::placeholders::_1);
		std::function<void(cyng::vector_t)> f6 = std::bind(&controller::label, this, std::placeholders::_1);
		std::function<std::string(cyng::vector_t)> f7 = std::bind(&controller::ref, this, std::placeholders::_1);
		std::function<std::string(cyng::vector_t)> f8 = std::bind(&controller::h1, this, std::placeholders::_1);
		std::function<void(std::string)> f_show = std::bind(&show, std::placeholders::_1);
		auto vm = fabric.create_proxy(tag, f1, f2, f3, f4, f5, f6, f7, f8);
		std::size_t slot{ 0 };
		vm.set_channel_name("quote", slot++);
		vm.set_channel_name("set", slot++);
		vm.set_channel_name(std::string("\xc2\xb6"), slot++);	//	paragraph
		vm.set_channel_name("i", slot++);	//	italic
		vm.set_channel_name("b", slot++);	//	bold
		vm.set_channel_name("label", slot++);	//	label
		vm.set_channel_name("ref", slot++);	//	ref
		vm.set_channel_name("h1", slot++);	//	h1
		vm.set_channel_name("show", slot++);

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
		ss << "QUOTE(" << vec << ")";
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
		//std::cout << ss.str() << std::endl;
		return ss.str();
	}

	void controller::set(cyng::param_map_t pm) {
		std::cout << "SET(" << pm << ")" << std::endl;
	}

	std::string controller::paragraph(cyng::vector_t vec) {
		std::reverse(std::begin(vec), std::end(vec));
		std::stringstream ss;
		ss << "PARAGRAPH(" << vec << ")";
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
		ss << "H1(" << vec << ")";
		std::cout << ss.str() << std::endl;
		return ss.str();
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
