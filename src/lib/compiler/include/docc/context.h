/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_SCRIPT_CONTEXT_H
#define DOCC_SCRIPT_CONTEXT_H

#include <docc/parser.h>
#include <docc/method.h>

#include <cstdlib>
#include <filesystem>
#include <vector>
#include <stack>
#include <map>
#include <fstream>
#include <optional>

namespace docscript {

	class sanitizer;
	class context
	{
	private:
		struct position {
			std::filesystem::path file_;
			std::size_t line_;
			std::fstream stream_;
		};

	public:
		context(std::filesystem::path out
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
		void pop(sanitizer&);

		/**
		 * update line counter of current file
		 */
		void nl(std::size_t);

		bool get_verbosity(int) const;

		/**
		 * search for the specified filename in all include
		 * directories.
		 */
		std::pair<std::filesystem::path, bool> lookup(std::filesystem::path const&, std::string ext) const;

		/**
		 * feed the parser
		 */
		void put(symbol const& sym);

		/**
		 * @return filename and line number
		 */
		std::string get_position() const;

		/**
		 * write to the temp file
		 */
		void emit(std::string const&);

		/**
		 * @return method with the specified name
		 */
		std::optional<method> lookup_method(std::string const&) const;

		std::filesystem::path const& get_output_path() const;

	private:
		std::filesystem::path const out_file_;
		std::ofstream ostream_;	//	stream of temp file
		std::vector<std::filesystem::path> inc_;
		int const verbose_;
		std::stack<position>    position_;
		std::map<std::string, method> method_table_;
		parser parser_;
	};

	void write_bom(std::ostream&);
	void init_method_table(std::map<std::string, method>&);
	bool insert_method(std::map<std::string, method>& table, method&&);
}

#endif  //  DOCC_SCRIPT_CONTEXT_H