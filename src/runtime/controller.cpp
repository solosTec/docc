
#include "controller.h"
//#include <tasks/ruler.h>
//#include <utils.h>

#include <cyng/task/controller.h>
//#include <cyng/task/stash.h>
#include <cyng/task/scheduler.h>
#include <cyng/vm/vm.h>
//#include <cyng/task/task.hpp>
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

	void hello() {
		std::cout << "hello, world!" << std::endl;
	}
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
		std::function<void()> f1 = std::bind(&hello);
		std::function<void(std::string)> f2 = std::bind(&show, std::placeholders::_1);
		std::function<void(std::string, std::string)> f3 = std::bind(&controller::quote, this, std::placeholders::_1, std::placeholders::_2);
		auto vm = fabric.create_proxy(tag, f1, f2, f3);
		std::size_t slot{ 0 };
		vm.set_channel_name("hello", slot++);
		vm.set_channel_name("show", slot++);
		vm.set_channel_name("quote", slot++);

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
		ctl.stop();

		auto const delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now);

		fmt::print(
			stdout,
			fg(fmt::color::forest_green),
			"***info : complete after {} milliseconds\n", delta.count());

		return EXIT_SUCCESS;
		//return EXIT_FAILURE;

	}

	void controller::quote(std::string s1, std::string s2) {
		std::cout << "QUOTE(" << s1 << ", " << s2 << ")" << std::endl;

	}
}
