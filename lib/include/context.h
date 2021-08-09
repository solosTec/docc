/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_CONTEXT_H
#define DOCC_SCRIPT_CONTEXT_H

#include <parser.h>

#include <cstdlib>
#include <filesystem>
#include <vector>
#include <stack>
#include <fstream>

namespace docscript {

	class context
	{
	private:
		struct position {
			std::filesystem::path file_;
			std::size_t line_;
			std::fstream stream_;
		};

	public:
		context(std::filesystem::path const&
			, std::filesystem::path out
			, std::vector<std::filesystem::path> inc
			, int verbose);
		context(context const&) = default;
		context(context &&) = delete;

		/**
		 * start new file
		 */
		bool push(std::filesystem::path const&);

		/**
		 * @return stream iterator of file on the top of the
		 * position stack.
		 */
		std::istream_iterator<char> get_stream_iterator();

		/**
		 * @return true if EOF is reached (no more data)
		 */
		bool pop();

		/**
		 * update line counter of current file
		 */
		void nl(std::size_t);

		bool get_verbosity(int) const;

		/**
		 * search for the specified filename in all include
		 * directories.
		 */
		std::pair<std::filesystem::path, bool> lookup(std::filesystem::path const&);

		/**
		 * feed the parser
		 */
		void put(symbol const& sym);

		/**
		 * @return filename and line number
		 */
		std::string get_position() const;

	private:
		std::filesystem::path const temp_;
		std::filesystem::path const out_file_;
		std::vector<std::filesystem::path> inc_;
		int const verbose_;
		std::stack<position>    position_;
		parser parser_;
	};

}

#endif  //  DOCC_SCRIPT_CONTEXT_H