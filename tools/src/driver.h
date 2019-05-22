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

#include <boost/filesystem.hpp>

namespace docscript
{
	/**
	 * parser context
	 */
	struct context
	{
		context();
		bool push(boost::filesystem::path);
		void pop();
		boost::filesystem::path top() const;

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
		std::deque<boost::filesystem::path>	source_files_;

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
		int run(boost::filesystem::path const& master
			, boost::filesystem::path const& tmp
			, boost::filesystem::path const& out
			, bool body_only
			, bool meta
			, bool index
			, std::string type);

	private:
		int run(boost::filesystem::path const& inp, std::size_t start, std::size_t count, std::size_t depth);
		int open_and_run(incl_t inp, std::size_t);

		/**
		 * @brief finish
		 * @param body
		 * @param out output file (html, tex, or md)
		 * @param meta generate a file with meta data
		 * @param index generate an index file in JSON format
		 * @param type article/report
		 */
		void finish(boost::filesystem::path const& body
			, boost::filesystem::path const& out
			, bool meta
			, bool index);

		/**
		 *	build the artifact
		 */
		void build(boost::filesystem::path const& in, boost::filesystem::path out, bool body_only, std::chrono::milliseconds);
		void print_error(cyng::logging::severity, std::string);

	private:
		/**
		 * Maintain a list of include directories.
		 * If the driver opens a file it searches in all given directories
		 * until the spcified file is found.
		 */
		std::vector< boost::filesystem::path > const includes_;

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
	std::tuple<std::chrono::system_clock::time_point, uintmax_t> read_meta_data(boost::filesystem::path);
		
	/**
	 * Sets the specified extension if the file name doesn't contains one.
	 */
	boost::filesystem::path verify_extension(boost::filesystem::path p, std::string const& ext);
}


#endif
