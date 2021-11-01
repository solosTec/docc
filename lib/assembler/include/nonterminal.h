/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_ASM_NONTERMINAL_H
#define DOCC_ASM_NONTERMINAL_H

/**@file non-terminals are used to build the syntax tree.
 * 
 * Production rules of docASM:
 * 
 * ; The program is divided in sections by so called "directives" currently only the 
 * ; .code directive is supported.
 * ; The next directive that will be supported is the .data section with a different syntax.
 * 
 * BODY		: PROGRAM eod
 * 
 * PROGRAM	: LINE PROGRAM
 *			| ε
 *
 * LINE		: LABEL	EOL					; LBL
 *			| STMT EOL					; INS - any instruction
 * 
 * STMT		: 'push' VALUE			
 *			| 'invoke' IDENT
 *			| 'now'
 *			| 'pid'
 *			| 'noop'
 *			| 'tid'
 *			| 'forward' UUID
 *			| 'jump' IDENT
 *
 * VALUE	: txt				
 *			| tst
 *			| col	; color
 *			| bol	; boolean
 *			| num	; number
 *			| UUID	; uuid
 * 
 * UUID		: lit typ 'uuid'
 * 
 * IDENT	: txt
 *			| lit
*/


#include <symbol.h>

namespace docscript {

	enum class nonterminal_type : std::uint32_t {
		BODY,		//	
		PROGRAM,	//	
		LINE,		//	
		VALUE,
		UUID,		//	LIT TYP 'uuid'
		IDENT,		//	any identifier
		EOL,		//	end of line
		TERMINAL
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