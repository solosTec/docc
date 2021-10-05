/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2021 Sylko Olzscher 
 * 
 */ 
#ifndef DOCC_SCRIPT_TOKEN_H
#define DOCC_SCRIPT_TOKEN_H

#include <cstdint>
#include <limits>
#include <iostream>
#include <string>
#include <functional>

namespace docscript {

    class token 
    {
        friend std::ostream& operator<<(std::ostream& os, token const& tok);

        public:

            constexpr token() 
            : c_(std::numeric_limits<std::uint32_t>::max())
            {}

            constexpr token(std::uint32_t c)
            : c_(c)
            {}

            constexpr token(token const& tok)
                : c_(tok.c_)
            {}

            constexpr bool is_eof() const
            {
                return std::numeric_limits<std::uint32_t>::max() == c_;
            }

            constexpr bool is_nl() const
            {
                return '\n' == c_;
            }

            //  static_cast<std::uint32_t>(...)
            constexpr operator std::uint32_t() const
            {
                return c_;
            }

        private:
            std::uint32_t const c_;

    };

	/**
	 * Generate an EOF token
	 */
    constexpr token make_eof()
    {
        return std::numeric_limits<std::uint32_t>::max();
    }

	/**
	 * Generate a NL token
	 */
    constexpr token make_nl()
    {
        return '\n';
    }

	/**
	 * Define an emit function
	 */
	using emit_token_f = std::function<void(token&&)>;

	/**
	 * Streaming operator
	 */
	std::ostream& operator<<(std::ostream& os, const token& tok);   

    /**
     * Works for ASCII code only.
     * 
     * @return the uppercase character 
     */
    std::uint32_t to_upper(token const& tok);
    std::uint32_t to_lower(token const& tok);
}

#endif