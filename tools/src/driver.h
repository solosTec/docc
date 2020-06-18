/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2017 Sylko Olzscher 
 * 
 */ 

#ifndef DOCC_DRIVER_H
#define DOCC_DRIVER_H

#include <docscript/sanitizer.h>
#include <docscript/tokenizer.h>
#include <docscript/statistics.h>
#include <docscript/include.h>

#include <cyng/intrinsics/sets.h>
#include <cyng/log/severity.h>

#include <chrono>
#include <deque>

#include <cyng/compatibility/file_system.hpp>

namespace docscript
{
	/**
	 * parser context
	 */
	struct context
	{
		context();
		bool push(cyng::filesystem::path);
		void pop();
		cyng::filesystem::path top() const;

		/**
		 * @return true if no stack contains no source files
		 */
		bool empty() const;

		std::size_t size() const;

		/**
		 * current line
		 */
		std::string line_;

		/**
		 * current (tokenized) source file line
		 */
		std::size_t	curr_line_, prev_line_;

		/**
		 * stack of source files
		 */
		std::deque<cyng::filesystem::path>	source_files_;

	};

	/**
	 * forward declaration
	 */
	class reader;

	/**
	 * Driver class for docscript parser.
	 * Controls the process of reading, compiling and generating of docsript files.
	 */
	class driver
	{
		friend class reader;

	public:
		/**
	 	 * Constructor
		 *
		 * @param inc vector of include paths
		 * @param verbose verbose level. The higher the number, the more will be logged.
		 */
		driver(std::vector< std::string >const& inc, int verbose);
		driver(std::vector< cyng::filesystem::path >const& inc, int verbose);
		virtual ~driver();

		/**
		 * In a first step the compiler generates an intermediate file from all
		 * input files that contains instructions to generate the output file.
		 * In a second step a special VM executes the instructions to generate the
		 * requested output file (HTML, PDF, ...)
		 *
		 * @param master master file
		 * @param tmp temporary intermediate file. 
		 * @param out output file (html, tex, or md)
		 * @param body_only generate only HTML body (ignored for other output formats)
		 * @param meta generate a file with meta data
		 * @param index generate an index file in JSON format
		 * @param type article/report
		 */
		int run(cyng::filesystem::path const& master
			, cyng::filesystem::path const& tmp
			, cyng::filesystem::path const& out
			, bool body_only
			, bool meta
			, bool index
			, std::string type);

		/**
		 * Explicit build of an boostrap based HTML page
		 */
		int generate_bootstrap_page(cyng::filesystem::path const& master
			, cyng::filesystem::path const& tmp
			, cyng::filesystem::path const& out);

		/**
		 * @return collected meta data
		 */
		cyng::param_map_t const& get_meta() const;

	private:
		int run(cyng::filesystem::path const& inp
			, std::size_t start
			, std::size_t count
			, std::size_t depth);
		int open_and_run(incl_t inp, std::size_t);

		/**
		 * @brief finish
		 * @param body
		 * @param out output file (html, tex, or md)
		 * @param meta generate a file with meta data
		 * @param index generate an index file in JSON format
		 * @param type article/report
		 */
		void finish(cyng::filesystem::path const& body
			, cyng::filesystem::path const& out
			, bool meta
			, bool index);

		/**
		 *	build the HTML artifact
		 */
		void build(cyng::filesystem::path const& in, cyng::filesystem::path out, bool body_only);

		/**
		 *	build the bootstrap artifact
		 */
		void build_bootstrap(cyng::filesystem::path const& in, cyng::filesystem::path out);

		void print_error(cyng::logging::severity, std::string);

		template<typename ...Args>
		void print_msg(cyng::logging::severity level, Args... args)
		{
			std::cout
				<< "***"
				<< cyng::logging::to_string(level)
				<< ": "
				;

			std::size_t n{ 0 };
			((std::cout << args << (++n != sizeof...(Args) ? " " : "")), ...);

			std::cout
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

		void sanitize(docscript::token&& tok);
		void tokenize(symbol&& sym);

		void generate_iml(cyng::filesystem::path const& master
			, cyng::filesystem::path const& body
			, cyng::filesystem::path const& out
			, bool generate_meta
			, bool generate_index);

	private:
		/**
		 * Maintain a list of include directories.
		 * If the driver opens a file it searches in all given directories
		 * until the spcified file is found.
		 */
		std::vector< cyng::filesystem::path > const includes_;

		/**
		 * verbosity level. 
		 * 0 == print only errors
		 */
		int const verbose_;

		/**
		 * Frequency table. Used to calculate shannon entropy 
		 * of the text.
		 */
		frequency_t	stats_;

		/**
		 * create tokens for tokenizer
		 */
		sanitizer sanitizer_;

		/**
		 * Process the stream of input tokens and generate symbols
		 */
		tokenizer tokenizer_;

		/**
		 * The symbol stream is stored in a simple list as input for the compiler.
		 * This is memory expensive. An other approach could be to store in a file.
		 */
		std::list<symbol>		stream_;

		/**
		 * cursor (parser context)
		 */
		context ctx_;

		/**
		 * meta data
		 */
		cyng::param_map_t meta_;
	};

	/**
	 * @return last write time and file size of the specified file. If file does not exist 
	 * the file size is 0.
	 */
	std::tuple<std::chrono::system_clock::time_point, uintmax_t> read_meta_data(cyng::filesystem::path);
		
	/**
	 * Sets the specified extension if the file name doesn't contains one.
	 */
	cyng::filesystem::path verify_extension(cyng::filesystem::path p, std::string const& ext);

	/**
	 * Scan all provided directories for p
	 */
	std::pair<cyng::filesystem::path, bool> resolve_path(std::vector< cyng::filesystem::path >const& inc, cyng::filesystem::path p);

	cyng::vector_t read_iml_file(cyng::filesystem::path const& name);

}


#endif
