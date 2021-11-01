/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_NONTERMINAL_H
#define DOCC_SCRIPT_NONTERMINAL_H

/**@file non-terminals are used to build the syntax tree.
 * 
 * Production rules of docScript:
 * 
 * ; The document (BODY) consists of a list of TERMs or is optionally empty
 * 
 * BODY		: LIST eod
 * 
 * LIST		: TERM LIST
 *			| ε
 *
 * TERM		: fun PARAMS			{ fun }
 *			| par VECTOR			{ par }					;	paragraph
 * 
 * PARAMS	: VALUE					{ txt, tst, fun, '"' }	; single value method (SVM)
 *			| '(' VECTOR ')'		{ '(' }					; parser lookup in function table required
 *			| '(' MAP ')'			{ '(' }					; to choose VECTOR or MAP
 * 
 * VECTOR	: VALUE VECTOR			{ txt, tst, fun, '"' }
 *			|  ε
 *	
 * MAP		: txt ':' VALUE TAIL	{ txt }
 * 
 * TAIL		: ',' MAP				{ ',' }
 * TAIL		| ε
 * 
 * VALUE	: txt					{ txt }
 *			| tst
 *			| fun PARAMS
 *			| '"' VECTOR '"'		; quote
 * 
 * 
 */

 
/**
 * see https://bnfplayground.pauliankline.com/
 * <body> ::= <list> <eod>
 * <eod> ::= "-1"
 * <list> ::= <term> <list> | E
 * <term> ::= <method> | <section>
 * <method> ::= "fun" <value> | "fun" "(" <vector> ")" | "fun" "(" <map> ")"
 * <section> ::= "par" <vector>
 * <value> ::= <word> | <method>
 * <word> ::= "txt" | "tst"
 * <vector> ::= <value> <vector> | E
 * <map> ::= <pair> | "," <pair>
 * <pair> ::= <key> <value>
 * <key> ::= "txt" ":"
 */

#include <symbol.h>

namespace docscript {

	enum class nonterminal_type : std::uint32_t {
		BODY,	//	
		LIST,	//	
		TERM,	//	
		VECTOR,	//	
		MAP,	//	
		TAIL,
		VALUE,
		SVM,	//	same as value for Single Value Methods
		PARAMS,
		TERMINAL,	//	any symbol
	};

	/**
	 * Streaming operator
	 */
	std::ostream& operator<<(std::ostream& os, nonterminal_type nt);

	struct nonterminal 
	{
		symbol const sym_;
		nonterminal_type const nttype_;

		nonterminal(symbol sym);
		nonterminal(nonterminal_type);
		nonterminal(symbol sym, nonterminal_type);

		/**
		 * conversion operator
		 */
		explicit operator nonterminal_type() const;
	};

	std::ostream& operator<<(std::ostream& os, nonterminal nt);

}

#endif