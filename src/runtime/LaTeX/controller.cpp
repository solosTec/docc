
#include "controller.h"
#include "generator.h"

#include <rt/currency.h>

#include <docc/reader.h>
#include <docc/utils.h>

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

	controller::controller(std::filesystem::path out
		, std::vector<std::filesystem::path> inc
		, std::filesystem::path const& tmp_asm
		, std::filesystem::path const& tmp_tex
		, int verbose)
	: ofs_(out.string(), std::ios::trunc)
		, tmp_tex_(tmp_tex.string(), std::ios::trunc)
		, tmp_tex_path_(tmp_tex)
		, ctx_(docscript::verify_extension(tmp_asm, "docs"), inc, verbose)
		, assembler_(std::filesystem::path(ctx_.get_output_path()).replace_extension("cyng")
			, inc
			, verbose)
	{}

	int controller::run(std::filesystem::path&& inp
		, std::size_t pool_size
		, boost::uuids::uuid tag) {

		//
		//	check output file
		//
		if (!ofs_.is_open() || !tmp_tex_.is_open()) {
			fmt::print(stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : cannot open output file\n");
			return EXIT_FAILURE;
		}

		auto const now = std::chrono::high_resolution_clock::now();

		//
		//	check input file
		//
		auto const r = ctx_.lookup(inp, "docscript");
		if (!r.second) {
			fmt::print(stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***error: input file [{}] not found\n", inp.string());
			return EXIT_FAILURE;
		}
		if (ctx_.get_verbosity(2)) {
			fmt::print(
				stdout,
				fg(fmt::color::gray),
				"***info : input file [{}]\n", r.first.string());
		}


		//
		//	start compiler and generate an assembler file
		//
		docscript::reader compiler(ctx_);
		compiler.read(r.first);

		if (ctx_.get_verbosity(2)) {

			fmt::print(stdout, fg(fmt::color::forest_green),
				"***info : intermediate file {} complete\n",
				ctx_.get_output_path());
		}

		//
		//	generate program from assembler
		//
		assembler_.read(std::filesystem::path(ctx_.get_output_path()));

		if (ctx_.get_verbosity(2)) {

			fmt::print(stdout, fg(fmt::color::forest_green),
				"***info : program {} is loaded\n",
				assembler_.get_output_path().string());
		}

		//
		//	load and execute program
		//
		std::ifstream ifs(assembler_.get_output_path().string(),
			std::ios::binary);
		if (ifs.is_open()) {
			ifs.unsetf(std::ios::skipws);

			//
			//	Create an scheduler with specified size
			//	of the thread pool.
			//
			cyng::controller ctl(pool_size);
			cyng::mesh fabric(ctl);

			generator gen(ifs, tmp_tex_, fabric, tag, ctx_);
			gen.run();

			//
			//	wait for pending requests
			//
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			ctl.cancel();
			ctl.stop();
			// ctl.shutdown();

			tmp_tex_.close();

			emit_header(gen.get_meta());
			ofs_ << "% body\n";
			ofs_ << "\\begin{document}\n";
			ofs_ << "\\maketitle\n";

			tmp_tex_.open(tmp_tex_path_.string(), std::ios::in);
			ofs_ << tmp_tex_.rdbuf();
			tmp_tex_.close();

			ofs_ << "\\end{document}\n";
			ofs_ << "% end\n";

		}
		else {
			fmt::print(stdout,
				fg(fmt::color::dark_orange) | fmt::emphasis::bold,
				"***info : input file [{}] not found\n", inp.string());
			return EXIT_FAILURE;
		}


		auto const delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - now);

		fmt::print(
			stdout,
			fg(fmt::color::forest_green),
			"***info : complete after {} milliseconds\n", delta.count());

		return EXIT_SUCCESS;
	}

	void controller::emit_header(cyng::param_map_t& meta) {
		ofs_ << "% header\n";
		ofs_ << "\\documentclass[12pt]{report}\n";
		ofs_ << "\\usepackage[utf8]{inputenc}\n";

		for (auto const& param : meta) {
			if (boost::algorithm::equals(param.first, "title")) {
				ofs_ << "\\title{" << param.second << "}\n";
			}
			else if (boost::algorithm::equals(param.first, "author")) {
				ofs_ << "\\author{" << param.second << "}\n";
			}
		}

		ofs_ << "\\date{\\today}\n";

	}
}
