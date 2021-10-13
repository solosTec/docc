
#include "controller.h"
#include <tasks/ruler.h>
#include <utils.h>

#include <cyng/task/stash.h>
#include <cyng/task/scheduler.h>
#include <cyng/task/task.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

namespace docscript {
	controller::controller(std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, int verbose)
	: ctx_(verify_extension(out, "docs"), inc, verbose)

	{}

	int controller::run(std::filesystem::path&& inp, std::size_t pool_size) {

		auto const r = ctx_.lookup(inp);
		if (r.second) {

			auto const now = std::chrono::high_resolution_clock::now();

			if (ctx_.get_verbosity(1)) {

				fmt::print(
					stdout,
					fg(fmt::color::gray),
					"***info : input file [{}]\n", r.first.generic_string());
			}

			//
			//	Create an I/O controller with specified size
			//	of the thread pool.
			//
			cyng::controller ctl(pool_size);
			cyng::stash channels(ctl.get_ctx());

			//
			//  start task
			//
			auto channel = ctl.create_named_channel_with_ref<ruler>("ruler", ctl, channels, ctx_);
			BOOST_ASSERT(channel->is_open());
			channel->dispatch("read", r.first);
			channels.lock(channel);

			//
			//	wait for pending requests
			//
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ctl.stop();

			auto const delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now);

			fmt::print(
				stdout,
				fg(fmt::color::forest_green),
				"***info : complete after {} milliseconds\n", delta.count());

			return EXIT_SUCCESS;
		}

		//
		//  print error message
		//
		fmt::print(
			stderr,
			fg(fmt::color::crimson) | fmt::emphasis::bold,
			"***error: [{}] not found\n", inp.string());

		return EXIT_FAILURE;

	}

}
