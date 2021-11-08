/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_SYMBOL_H
#define DOCC_ASM_SYMBOL_H

#include <cstdint>
#include <iostream>
#include <string>
#include <functional>
#include <filesystem>

namespace docasm {

	enum class symbol_type : std::uint32_t {
		EOD,	//	end-of-data
		DIR,	//	directive
		TXT,	//	text
		LIT,	//	literal
		LBL,	//	label:
		INS,	//	instruction (op code)
		SYM,	//	special symbol like "(", ")", ",", ":"
		TST,	//	timestamp
		COL,	//	color
		BOL,	//	boolean
		NUM,	//	number (unsigned integer)
		EOL,	//	end of line (nl)
		TYP,	//	type specifier (trail)
		//INC,	//	include file

		FIL,	//	file path
		LIN,	//	line in file

		NOT,	//	nonterminal or not initialized
	};

    struct symbol 
    {
        friend std::ostream& operator<<(std::ostream& os, const symbol& sym);

		symbol_type const type_;
		std::string const value_;

		symbol();
		symbol(symbol_type, std::string&&);

		bool equals(char c) const;
		bool equals(symbol_type type, char c) const;
		bool equals(symbol_type type) const;

		auto operator<=>(symbol const&) const = default;
	};



	/**
	 * comparison
	 */
	bool operator==(symbol, symbol_type);
	bool operator!=(symbol, symbol_type);

	/**
	 * helper
	 */
	symbol make_symbol(symbol_type, std::string&&);
	symbol make_symbol(std::filesystem::path const&);
	symbol make_symbol(std::size_t);
	symbol make_symbol();	//	EOD

	/**
	 * Define an emit function
	 */
	using emit_symbol_f = std::function<void(symbol&&)>;

	/**
	 * Streaming operator
	 */
	std::ostream& operator<<(std::ostream& os, const symbol& sym);
}

#endif