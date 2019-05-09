/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#include "docscript_to_html.h"
#include <docscript/sanitizer.h>
#include <docscript/tokenizer.h>
#include <docscript/symbol.h>
#include <docscript/parser.h>

namespace docscript
{
	docscript_to_html::docscript_to_html(bool linenumbers, boost::uuids::uuid tag)
		: linenumbers_(linenumbers)
		, tag_(tag)
	{}

	void docscript_to_html::convert(std::ostream& os, std::string const& inp)
	{
		std::list<symbol>		stream;

		tokenizer _tokenizer([&](symbol&& sym) {

			std::cout
				<< "SYMBOL  "
				<< sym
				<< std::endl;
			//
			//	save in stream
			//
			stream.emplace_back(std::move(sym));

			}, std::bind(&docscript_to_html::print_error, this, std::placeholders::_1, std::placeholders::_2));


		/**
		 * create tokens for tokenizer
		 */
		sanitizer san([&](token&& tok) {

			while (!_tokenizer.next(tok)) {
				;
			}

			}, std::bind(&docscript_to_html::print_error, this, std::placeholders::_1, std::placeholders::_2));

		//
		//	read input
		//
		auto const start = std::begin(inp);
		auto const stop = std::end(inp);
		san.read(boost::u8_to_u32_iterator<std::string::const_iterator>(start), boost::u8_to_u32_iterator<std::string::const_iterator>(stop));
		san.flush(true);

		//
		//	create parser
		//
		parser p(stream);

		//
		//	generate parse tree (AST)
		//
		p.parse();

		//
		//	convert parse tree to html
		//
		auto const& ast  = p.get_ast();
		ast.generate_html(os, linenumbers_);
	}

	void docscript_to_html::print_error(cyng::logging::severity level, std::string msg)
	{
		std::cerr << msg << std::endl;
	}

}


