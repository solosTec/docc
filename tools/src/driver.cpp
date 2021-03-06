/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include "driver.h"
#include "reader.h"
#include <docscript/symbol.h>
#include <docscript/parser.h>
#include <docscript/generator/gen_html.h>
#include <docscript/generator/gen_md.h>
#include <docscript/generator/gen_asciidoc.h>
#include <docscript/generator/gen_LaTeX.h>
#include <docscript/generator/gen_bootstrap.h>

#include <cyng/io/serializer.h>
#include <cyng/io/parser/parser.h>
#include <cyng/io/io_chrono.hpp>
#include <cyng/traits.h>
#include <cyng/factory.h>
#include <cyng/vm/manip.h>
#include <cyng/vm/generator.h>

#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>

namespace docscript
{
	
	driver::driver(std::vector< std::string >const& inc, int verbose)
	: includes_(inc.begin(), inc.end())
		, verbose_(verbose)
		, stats_()
		, stream_()
		, ctx_()
		, meta_()

		//
		//	sanitizer => tokenizer 
		//
		, sanitizer_(std::bind(&driver::sanitize, this, std::placeholders::_1), std::bind(&driver::print_error, this, std::placeholders::_1, std::placeholders::_2))
		, tokenizer_(std::bind(&driver::tokenize, this, std::placeholders::_1), std::bind(&driver::print_error, this, std::placeholders::_1, std::placeholders::_2))

	{}
		
	driver::driver(std::vector< cyng::filesystem::path >const& inc, int verbose)
		: includes_(inc.begin(), inc.end())
		, verbose_(verbose)
		, stats_()
		, stream_()
		, ctx_()
		, meta_()
		, sanitizer_(std::bind(&driver::sanitize, this, std::placeholders::_1), std::bind(&driver::print_error, this, std::placeholders::_1, std::placeholders::_2))
		, tokenizer_(std::bind(&driver::tokenize, this, std::placeholders::_1), std::bind(&driver::print_error, this, std::placeholders::_1, std::placeholders::_2))
	{}

	driver::~driver()
	{}

	void driver::tokenize(symbol&& sym)
	{
		if (verbose_ > 5)
		{
			std::cout
				<< "SYMBOL  "
				<< sym
				<< std::endl;
		}

		//
		//	save in stream
		//
		stream_.emplace_back(std::move(sym));
	}

	void driver::sanitize(docscript::token&& tok)
	{
		if (!tok.eof_)
		{
			//	update frequency
			stats_[tok.value_] += tok.count_;
		}

		while (!tokenizer_.next(tok))
		{
#ifdef _DEBUG
			if (verbose_ > 9)
			{
				std::cout
					<< "***info: repeat"
					<< tok
					<< std::endl;
			}
#endif
		}
	}

	cyng::param_map_t const& driver::get_meta() const
	{
		return meta_;
	}

	int driver::run(cyng::filesystem::path const& master
		, cyng::filesystem::path const& body
		, cyng::filesystem::path const& out
		, bool generate_body_only
		, bool generate_meta
		, bool generate_index
		, std::string type)
	{
		if (!out.empty())
		{
			//
			// update meta data
			//
			meta_["og:type"] = cyng::make_object(type);

			//
			//	generate IML code
			//
			generate_iml(master, body, out, generate_meta, generate_index);

			print_msg(cyng::logging::severity::LEVEL_INFO, "output file is", out);
			build(body, out, generate_body_only);

			//
			//	remove temporary files
			//
			cyng::error_code ec;
			cyng::filesystem::remove(body, ec);

			return ec.value();
		}

		print_msg(cyng::logging::severity::LEVEL_ERROR, "no output file specified");
		return EXIT_FAILURE;
	}

	int driver::generate_bootstrap_page(cyng::filesystem::path const& master
		, cyng::filesystem::path const& body
		, cyng::filesystem::path const& out)
	{
		if (!out.empty())
		{
			//
			//	generate IML code
			//
			generate_iml(master, body, out, false, false);

			//
			//	generate HTML bootstrap page
			//
			print_msg(cyng::logging::severity::LEVEL_INFO, "output file is", out);
			build_bootstrap(body, out);

			//
			//	remove temporary files
			//
			cyng::error_code ec;
			cyng::filesystem::remove(body, ec);

			return ec.value();
		}

		print_msg(cyng::logging::severity::LEVEL_ERROR, "no output file specified");
		return EXIT_FAILURE;

	}

	void driver::generate_iml(cyng::filesystem::path const& master
		, cyng::filesystem::path const& body
		, cyng::filesystem::path const& out
		, bool generate_meta
		, bool generate_index)
	{

		//
		//	get a timestamp to measure performance
		//
		auto const now = std::chrono::system_clock::now();

		//
		//	read and tokenize file recursive
		//
		int const r = open_and_run(std::make_tuple(master, 0u, std::numeric_limits<std::size_t>::max()), 0);
		finish(body, out, generate_meta, generate_index);

		//
		//	calculate duration of reading and compilation
		//
		auto const delta = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - now);
		if (verbose_ > 0)
		{
			print_msg(cyng::logging::severity::LEVEL_TRACE, "compilation took ", cyng::to_str(delta));
		}
	}


	int driver::run(cyng::filesystem::path const& inp, std::size_t start, std::size_t count, std::size_t depth)
	{	
		reader r(*this, inp, start, count);
		return r.run(depth)
			? EXIT_SUCCESS
			: EXIT_FAILURE
			;
	}

	int driver::open_and_run(incl_t inp, std::size_t depth)
	{
		BOOST_ASSERT_MSG(depth < 64, "maximal nesting depth exceeded");
		const auto p = verify_extension(std::get<0>(inp), "docscript");

		//
		//	open file and read it line by line
		//
		auto const r = resolve_path(includes_, p);
		if (r.second) {
			if (depth == 0)
			{
				//
				//	get meta data of master file
				//
				std::chrono::system_clock::time_point last_write_time;
				uintmax_t file_size;

				std::tie(last_write_time, file_size) = read_meta_data(r.first);

				meta_["last-write-time"] = cyng::make_object(last_write_time);
				meta_["released"] = cyng::make_object(last_write_time);
				meta_["file-size"] = cyng::make_object(file_size);
				meta_["total-file-size"] = cyng::make_object(file_size);
				meta_["file-name"] = cyng::make_object(p.filename().string());
				meta_["title"] = cyng::make_object(p.stem().string());
				meta_["language"] = cyng::make_object("en");
			}
			else {

				//
				//	update total file size
				//
				std::chrono::system_clock::time_point last_write_time;
				uintmax_t file_size;
				std::tie(last_write_time, file_size) = read_meta_data(r.first);
				file_size += cyng::value_cast(meta_.at("total-file-size"), file_size);
				meta_["total-file-size"] = cyng::make_object(file_size);

			}
			return run(r.first
				, std::get<1>(inp)	//	start
				, std::get<2>(inp)	//	count
				, depth);
		}

		//
		//	print error message
		//
		print_msg(cyng::logging::severity::LEVEL_FATAL, "file [", p, "] not found");
		return EXIT_FAILURE;

	}

	void driver::finish(cyng::filesystem::path const& body
		, cyng::filesystem::path const& out
		, bool meta
		, bool index)
	{
		//
		//	create intermediate output file for compiler
		//
		std::ofstream file(body.string(), std::ios::out | std::ios::trunc | std::ios::binary);
		if (!file.is_open())
		{
			print_msg(cyng::logging::severity::LEVEL_ERROR, "cannot open file [", body, "]");
		}
		else
		{
			
			if (verbose_ > 1)
			{
				print_msg(cyng::logging::severity::LEVEL_TRACE, "start parser with", stream_.size(), "input symbols");
			}

			//
			//	calculated entropy
			//
			auto const entropy = calculate_entropy(stats_);
			auto const size = calculate_size(stats_);	//	symbol count

			meta_["text-entropy"] = cyng::make_object(entropy);
			meta_["token-count"] = cyng::make_object(size);
			if (verbose_ > 2)
			{
				//
				//	automatic generated meta data
				//
				std::cout
					<< "***info: last write time: "
					<< cyng::io::to_str(meta_.at("last-write-time"))
					<< std::endl
					<< "***info: file size of master file: "
					<< cyng::io::to_str(meta_.at("file-size"))
					<< " bytes"
					<< std::endl
					<< "***info: entropy is "
					<< entropy
					<< " (calculated over "
					<< size
					<< " input token)"
					<< std::endl
					;
			}

			//
			//	serialize meta data
			//
			auto meta_prg = cyng::generate_invoke("init.meta.data", meta_);
			for (auto obj : meta_prg) {
				cyng::io::serialize_binary(file, obj);
			}

			//
			//	create parser
			//
			parser p(stream_, verbose_);

			//
			//	generate parse tree (AST)
			//
			p.parse();

			//
			//	generate code
			//
			auto const prg = p.get_ast().generate(out, meta, index);

			//
			//	serialize as program not as data (reverse on stack)
			//
			std::size_t counter{0};
			for (auto obj : prg) {
#ifdef _DEBUG
//				std::cout
//					<< '#'
//					<< counter
//					<< '\t'
//					<< cyng::io::to_str(obj)
//					<< ((obj.get_class().tag() == cyng::TC_CODE) ? "\n" : " ")
//					;
#endif
				cyng::io::serialize_binary(file, obj);
				++counter;
			};
		}
	}

	void driver::build(cyng::filesystem::path const& in
		, cyng::filesystem::path out
		, bool body_only)
	{
		//
		//	read intermediate file
		//
		auto prg = read_iml_file(in);

		//
		//	startup VM/generator
		//
		auto const extension = out.extension().string();
		if (boost::algorithm::iequals(extension, ".html")) {
			gen_html g(this->includes_, body_only);
			g.run(std::move(prg));
			auto const m = g.get_meta();
			for (auto const& i : m) {
				meta_[i.first] = i.second;
			}
		}
		else if (boost::algorithm::iequals(extension, ".md")) {
			gen_md g(this->includes_);
			g.run(std::move(prg));
			auto const m = g.get_meta();
			for (auto const& i : m) {
				meta_[i.first] = i.second;
			}
		}
		else if (boost::algorithm::iequals(extension, ".asciidoc") 
			|| boost::algorithm::iequals(extension, ".adoc") 
			|| boost::algorithm::iequals(extension, ".asc")) {
			gen_asciidoc g(this->includes_);
			g.run(std::move(prg));
			auto const m = g.get_meta();
			for (auto const& i : m) {
				meta_[i.first] = i.second;
			}
		}
		else if (boost::algorithm::iequals(extension, ".tex")) {
			gen_latex g(this->includes_);
			g.run(std::move(prg));
			auto const m = g.get_meta();
			for (auto const& i : m) {
				meta_[i.first] = i.second;
			}
		}
		else {
			std::cerr
				<< "***"
				<< cyng::logging::to_string(cyng::logging::severity::LEVEL_FATAL)
				<< ": unknown output format: ["
				<< extension
				<< "]"
				;
		}
	}

	void driver::build_bootstrap(cyng::filesystem::path const& in, cyng::filesystem::path out)
	{
		//
		//	read intermediate file
		//
		auto prg = read_iml_file(in);

		//
		//	startup VM/generator
		//
		gen_bootstrap g(this->includes_);
		g.run(std::move(prg));
		auto const m = g.get_meta();
		for (auto const& i : m) {
			meta_[i.first] = i.second;
		}
	}


	void driver::print_error(cyng::logging::severity level, std::string msg)
	{
		std::cout
			<< "***"
			<< cyng::logging::to_string(level)
			<< ": "
			<< msg
			<< " #"
			<< ctx_.curr_line_
			;

		if (!ctx_.empty()) {
			std::cout
				<< '@'
				<< ctx_.top()
				;
		}
		std::cout
			<< std::endl
			;
	}

	context::context()
		: line_()
		, curr_line_(0u)
		, prev_line_(std::numeric_limits<std::size_t>::max())
		, source_files_()
	{}

	bool context::push(cyng::filesystem::path p)
	{
		auto const size = std::count(source_files_.begin(), source_files_.end(), p);
		if (size == 0u) {
			source_files_.push_back(p);
		}
		return size == 0u;
	}

	void context::pop()
	{
		source_files_.pop_back();
	}

	cyng::filesystem::path context::top() const
	{
		return source_files_.back();
	}

	bool context::empty() const
	{
		return source_files_.empty();
	}

	std::size_t context::size() const
	{
		return source_files_.size();
	}

	std::tuple<std::chrono::system_clock::time_point, uintmax_t> read_meta_data(cyng::filesystem::path p)
	{
		return (cyng::filesystem::exists(p) && cyng::filesystem::is_regular(p))
			? std::make_tuple(cyng::filesystem::get_write_time(p), cyng::filesystem::file_size(p))
			: std::make_tuple(std::chrono::system_clock::now(), uintmax_t(0u))
			;
	}

	cyng::filesystem::path verify_extension(cyng::filesystem::path p, std::string const& ext)
	{
		if (!p.has_extension())
		{
			p.replace_extension(ext);
		}
		return p;	
	}

	std::pair<cyng::filesystem::path, bool> resolve_path(std::vector< cyng::filesystem::path >const& inc, cyng::filesystem::path p)
	{
		for (auto const& dir : inc)
		{
			if (cyng::filesystem::exists(dir / p))	return std::make_pair((dir / p), true);
		}

		//
		//	not found - try harder
		//
		for (auto const& dir : inc)
		{
			//	ignore path
			if (cyng::filesystem::exists(dir / p.filename()))	return std::make_pair((dir / p.filename()), true);
		}

		return std::make_pair(p, false);
	}

	cyng::vector_t read_iml_file(cyng::filesystem::path const& name)
	{
		cyng::vector_t prg;

		std::ifstream file(name.string(), std::ios::binary);
		if (file.is_open())
		{
			//
			//	do not skip 
			//
			file.unsetf(std::ios::skipws);

			//
			//	parse temporary file
			//
			cyng::parser np([&prg](cyng::vector_t&& vec) {
				prg = std::move(vec);
				});
			np.read(std::istream_iterator<char>(file), std::istream_iterator<char>());
		}
		return prg;
	}

}	
