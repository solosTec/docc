
#include "controller.h"

#include <docc/utils.h>
#include <docc/reader.h>

#include <fmt/core.h>
#include <fmt/color.h>

namespace docscript {
	controller::controller(std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, int verbose)
	: ctx_(verify_extension(out, "docs"), inc, verbose)

	{}

	int controller::run(std::filesystem::path&& inp, std::size_t pool_size) {

		auto const r = ctx_.lookup(inp, "docscript");
		if (r.second) {

			auto const now = std::chrono::high_resolution_clock::now();

			if (ctx_.get_verbosity(1)) {

				fmt::print(
					stdout,
					fg(fmt::color::gray),
					"***info : input file [{}]\n", r.first.generic_string());
			}

			reader inp(ctx_);
			inp.read(r.first);

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
