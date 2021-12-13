
#include <html/code_cmake.h>
#include <html/formatting.h>

#include <sstream>
#include <iomanip>
#include <set>
#include <stack>

#include <boost/algorithm/string.hpp>

namespace dom
{
	class parser {
		enum class symbol_type
		{
			EOS,		//!<	no more symbols
			WS,			//!<	white space
			COMMENT,	//!<	// #...eol
			//MARK,		//!<	single symbol like "{", "}", "(", ")", ...
			//MACRO,		//!<	precompiler statement like #ifdef, #include, ...
			//KEYWORD,	//!<	int, long, while, ...
			//OPERATOR,	//!<	<, >, ==, ...
			//LIBRARY,	//!<	known class
			LITERAL,	//!<	any literal string
			//STRING,		//!<	"..."
			//CHAR,		//!<	'.'
			LINE_START,
			LINE_END,
		};

		struct symbol
		{
			symbol(symbol_type t, std::string&& s)
				: type_(t)
				, value_(std::move(s))
			{}
			symbol(symbol_type t)
				: type_(t)
				, value_()
			{}
			symbol_type const type_;
			std::string const value_;

		};


		class tokenizer {
			enum class state {
				INITIAL,
				QUOTE,
				COMMENT,	//	#...eol
				VARIABLE,
				LITERAL,	//	anything
			};
			std::stack<state>	state_;

		public:
			tokenizer(std::function<void(std::size_t, symbol&&)> emit)
				: state_()
				, line_number_(1)
				, emit_(emit)
				, token_()
			{
				state_.push(state::INITIAL);
			}

			bool put(std::uint32_t c) {
				bool advance = true;
				switch (get_state()) {
				case state::INITIAL:
					std::tie(state_.top(), advance) = 
						state_initial(c);
					break;
				case state::COMMENT:
					std::tie(state_.top(), advance) = 
						state_comment(c);
					break;
				case state::VARIABLE:
					std::tie(state_.top(), advance) = 
						state_variable(c);
					break;
				case state::LITERAL:
					std::tie(state_.top(), advance) =
						state_literal(c);
					break;
				default:
					break;
				}
				return advance;
			}

		private:
			state get_state() const {
				return state_.empty()
					? state::INITIAL
					: state_.top()
					;
			}

			std::pair<state, bool> state_initial(std::uint32_t c) {
				switch (c) {
				case '\n':
					++line_number_;
					emit(symbol(symbol_type::LINE_END));
					emit(symbol(symbol_type::LINE_START));
					break;
				case '\r':
					;	//	ignore
					break;
				case ' ': case '\t':	//	white space
					token_.push_back(c);
					emit(symbol_type::WS);
					break;
				case '#':
					return std::make_pair(state::COMMENT, true);
				case '$':
					return std::make_pair(state::VARIABLE, true);
				default:
					token_.push_back(c);
					return std::make_pair(state::LITERAL, true);
				}
				return std::make_pair(get_state(), true);
			}
			std::pair<state, bool> state_comment(std::uint32_t c) {
				if (c == '\n') {
					++line_number_;
					//	comment complete
					emit(symbol_type::COMMENT);
					emit(symbol(symbol_type::LINE_END));
					emit(symbol(symbol_type::LINE_START));
					return std::make_pair(state::INITIAL, true);
				}
				token_.push_back(c);
				return std::make_pair(get_state(), true);
			}

			std::pair<state, bool> state_variable(std::uint32_t c) {
				return std::make_pair(get_state(), true);
			}

			std::pair<state, bool> state_literal(std::uint32_t c) {
				if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')) {
					token_.push_back(c);
					return std::make_pair(get_state(), true);
				}
				emit();
				return std::make_pair(state::INITIAL, false);
			}

			/**
			 * emit token as next symbol
			 */
			void emit(symbol_type st)
			{
				if (!token_.empty()) {
					cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> start(token_.begin());
					cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> end(token_.end());
					emit_(line_number_, symbol(st, std::string(start, end)));
					token_.clear();
				}
			}
			void emit(symbol&& s) const
			{
				emit_(line_number_, std::move(s));
			}
			void emit()
			{
				if (!token_.empty()) {
					cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> start(token_.begin());
					cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> end(token_.end());
					std::string s(start, end);
					//	ToDo:
					//if (key_words_.contains(s)) {
					//	emit_(line_number_, symbol(symbol_type::KEYWORD, std::move(s)));
					//}
					//else if (ops_.contains(s)) {
					//	emit_(line_number_, symbol(symbol_type::OPERATOR, std::move(s)));
					//}
					//else if (known_classes_.contains(s)) {
					//	emit_(line_number_, symbol(symbol_type::LIBRARY, std::move(s)));
					//}
					//else {
					emit_(line_number_, symbol(symbol_type::LITERAL, std::move(s)));
					//}
					token_.clear();
				}
			}

		private:
			std::size_t line_number_;
			std::function<void(std::size_t, symbol&&)> emit_;
			std::u32string token_;
		};
	public:
		parser(std::ostream& os, bool numbers) 
			: os_(os)
			, numbers_(numbers)
			, tokenizer_([this](std::size_t line, symbol&& sym) {
				this->next(line, std::move(sym));
			})
		{}
		void read(cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> start,
			cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> end) {

			os_ << "<table style=\"tab-size: 2;\" class=\"docc-code\" >" << std::endl << "<tbody style=\"white-space: pre;\">" << std::endl;
			open_row(1);
			while (start != end) {
				//std::cout << '[' << (char) * start << ']' << std::endl;
				{
					std::size_t counter = 4;	//	max 4 repeats
					while (!tokenizer_.put(*start) && counter-- > 0) {
						;
					}
				}
				++start;
			}

			close_row();
			os_ << "</tbody>" << std::endl << "</table>" << std::endl;

		}

	private:
		void next(std::size_t line, symbol&& sym) {

			switch (sym.type_) {
			case symbol_type::LINE_START:
				open_row(line);
				break;
			case symbol_type::LINE_END:
				close_row();
				break;
			case symbol_type::WS:
				os_ << sym.value_;	// "[WS]";
				break;
			case symbol_type::COMMENT:
				os_ << "<span class=\"docc-comment\">#";
				esc_html(os_, sym.value_);
				os_ << "</span>";
				break;
			default:
				os_ << sym.value_;
				break;
			}

		}
		void open_row(std::size_t line) {
			os_ << "<tr>";
			if (numbers_) {
				os_ << "<td class=\"docc-num\" data-line-number=\"" << line << "\"></td>";
			}
			os_ << "<td class=\"docc-code\">";
		}
		void close_row() {
			os_ << "</td>" << "</tr>" << std::endl;
		}

	private:
		std::ostream& os_;
		bool const numbers_;
		tokenizer tokenizer_;
	};

	void cmake_to_html(std::ostream& os,
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> start,
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> end,
		bool numbers) {

		parser p(os, numbers);
		p.read(start, end);
	}

}
