/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 


#include "reader.h"
#include "driver.h"
#include <docscript/pre_compiler.h>

#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>

namespace docscript
{
	
	reader::reader(driver& d, cyng::filesystem::path const& source, std::size_t start, std::size_t count)
		: driver_(d)
		, source_(source)
		, start_(start)
		, count_(count)
		, curr_line_(0)
	{}
		
	bool reader::run(std::size_t depth)
	{
		std::fstream f(source_.string(), std::ios::in);
		if (f.is_open())
		{
			std::stringstream ss;
			ss
				<< "parse file ["
				<< source_
				<< "]"
				;
			driver_.print_error(cyng::logging::severity::LEVEL_TRACE, ss.str());

			//
			//	update source file stack and detect recursive
			//	includes
			//
			if (!driver_.ctx_.push(source_)) {

				ss.str("");
				ss
					<< "recursive includes ["
					<< source_
					<< "]"
					;
				driver_.print_error(cyng::logging::severity::LEVEL_FATAL, ss.str());
				return false;
			}

			//
			//	new file
			//
			driver_.tokenizer_.emit_current_file(source_.string());

			std::string line;
			while (std::getline(f, line))
			{
				if (curr_line_ == 0 && line.size() > 3)
				{
					//	test UTF-8 BOM
					if (line.at(0) == (char)0xef && line.at(1) == (char)0xbb && line.at(2) == (char)0xbf)
					{
						ss.str("");
						ss
							<< "***info: "
							<< source_.filename()
							<< " contains UTF-8 signature (BOM)"
							;
						driver_.print_error(cyng::logging::severity::LEVEL_TRACE, ss.str());

						//	remove first three characters
						line.erase(0, 3);

					}
				}

				//
				//	update line counter and store current input (line)
				//
				driver_.ctx_.curr_line_ = ++curr_line_;
				driver_.ctx_.line_ = line;

				if ((curr_line_ > start_) && (curr_line_ < (start_ + count_))) {
					if (boost::algorithm::starts_with(line, ";"))
					{
						//	skip comments
						//	The compiler doesn't see any comments.
						;
					}
					else if (boost::algorithm::starts_with(line, ".include"))
					{
						//
						//	The include command has to be handled before
						//	the compiler is running. So this is implemented
						//	as preprocessor function.
						//
						auto const r = parse_include(line);
						driver_.open_and_run(r, depth + 1);

						//
						//	reset current file
						//
						driver_.tokenizer_.emit_current_file(source_.string());
					}
					else
					{
						driver_.tokenizer_.emit_current_line(curr_line_);

						//	virtual "new line" at the beginning
						tokenize("\n");
						tokenize(line);
					}
				}
			}

			//
			//	emit last character
			//
			driver_.sanitizer_.flush(driver_.ctx_.size() == 1);

			//
			//	update source file stack
			//
			driver_.ctx_.pop();
			return true;
		}
			
		std::stringstream ss;
		ss
			<< "could not open ["
			<< source_
			<< "]"
			;
		driver_.print_error(cyng::logging::severity::LEVEL_ERROR, ss.str());

		return false;
	}

	void reader::tokenize(std::string const& str)
	{
		auto start = std::begin(str);
		auto stop = std::end(str);
		driver_.sanitizer_.read(boost::u8_to_u32_iterator<std::string::const_iterator>(start), boost::u8_to_u32_iterator<std::string::const_iterator>(stop));
	}

	incl_t reader::parse_include(std::string const& line)
	{
		pre_compiler pc(line);
		return pc.parse_include();
	}
}
