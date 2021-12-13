
#include <html/code_curly.h>
#include <html/formatting.h>

#include <sstream>
#include <iomanip>
#include <set>

#include <boost/algorithm/string.hpp>

namespace dom
{
	/**
	 * comment styles and strings are hard coded.
	 */
	class curly {
	public:
		enum class symbol_type
		{
			EOS,		//!<	no more symbols
			WS,			//!<	white space
			MARK,		//!<	single symbol like "{", "}", "(", ")", ...
			MACRO,		//!<	precompiler statement like #ifdef, #include, ...
			COMMENT,	//!<	// ...
			KEYWORD,	//!<	int, long, while, ...
			OPERATOR,	//!<	<, >, ==, ...
			LIBRARY,	//!<	known class
			LITERAL,	//!<	any literal string
			STRING,		//!<	"..."
			CHAR,		//!<	'.'
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

	private:
		class tokenizer {
			enum class state {
				INITIAL,
				QUOTE,
				CHAR,
				OP,	//	operator
				WS,	//	whitespace
				COMMENT,
				COMMENT_SINGLE_LINE,
				COMMENT_MULTI_LINE,
				COMMENT_MULTI_LINE_END,
				MACRO,	//	#...
				LITERAL,	//	anything
				//NUMBER_,
			} state_;

		public:
			tokenizer(std::function<void(std::size_t, symbol&&)> emit, std::initializer_list<std::string> key_words, std::initializer_list<std::string> ops, std::initializer_list<std::string> known_classes)
				: state_(state::INITIAL)
				, line_number_(1)
				, emit_(emit)
				, key_words_(key_words.begin(), key_words.end())
				, ops_(ops.begin(), ops.end())
				, known_classes_(known_classes.begin(), known_classes.end())
				, token_()
			{	}

			bool put(std::uint32_t c) {
				bool advance = true;
				switch (state_) {
				case state::INITIAL:
					std::tie(state_, advance) = state_initial(c);
					break;
				case state::QUOTE:
					std::tie(state_, advance) = state_quote(c);
					break;
				case state::CHAR:
					std::tie(state_, advance) = state_char(c);
					break;
				case state::COMMENT:
					std::tie(state_, advance) = state_comment(c);
					break;
				case state::COMMENT_SINGLE_LINE:
					std::tie(state_, advance) = state_comment_single_line(c);
					break;
				case state::COMMENT_MULTI_LINE:
					std::tie(state_, advance) = state_comment_multi_line(c);
					break;
				case state::COMMENT_MULTI_LINE_END:
					std::tie(state_, advance) = state_comment_multi_line_end(c);
					break;
				case state::MACRO:
					std::tie(state_, advance) = state_macro(c);
					break;
				case state::LITERAL:
					std::tie(state_, advance) = state_literal(c);
					break;
				case state::OP:
					std::tie(state_, advance) = state_op(c);
					break;
				default:
					break;
				}
				return advance;
			}

		private:
			std::pair<state, bool> state_initial(std::uint32_t c) {
				switch (c) {
				case '{': case '}': case '(': case ')':
					token_.push_back(c);
					emit(symbol_type::MARK);
					break;
				case '<': case '>':	//	&gt;
				case ':':
				case '+': case '-':
				case '*': 
				case '=': case '!':
				case '?': 
				case '&': case '%': case '|':
					token_.push_back(c);
					return std::make_pair(state::OP, true);
					break;

				case '"':
					return std::make_pair(state::QUOTE, true);
				case '\'':
					return std::make_pair(state::CHAR, true);
				case '/':
					return std::make_pair(state::COMMENT, true);
				//case '[':	//	annotations
				//	break;
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
				case '#': 	//	macro
					return std::make_pair(state::MACRO, true);
				default:
					token_.push_back(c);
					return std::make_pair(state::LITERAL, true);
				}
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_quote(std::uint32_t c) {
				if (c == '"') {
					emit(symbol_type::STRING);
					return std::make_pair(state::INITIAL, true);
				}
				token_.push_back(c);
				return std::make_pair(state_, true);
			}
			std::pair<state, bool> state_char(std::uint32_t c) {
				if (c == '\'') {
					emit(symbol_type::CHAR);
					return std::make_pair(state::INITIAL, true);
				}
				//	multiple chars allowed
				token_.push_back(c);
				return std::make_pair(state_, true);
			}
			std::pair<state, bool> state_comment(std::uint32_t c) {
				switch (c) {
				case '/':
					token_.push_back(c);
					token_.push_back(c);
					return std::make_pair(state::COMMENT_SINGLE_LINE, true);
				case '*':
					token_.push_back('/');
					token_.push_back(c);
					return std::make_pair(state::COMMENT_MULTI_LINE, true);
				default:
					break;
				}
				token_.push_back('/');
				token_.push_back(c);
				return std::make_pair(state::INITIAL, true);
			}

			std::pair<state, bool> state_comment_single_line(std::uint32_t c) {
				if (c == '\n') {
					++line_number_;
					//	comment complete
					emit(symbol_type::COMMENT);
					emit(symbol(symbol_type::LINE_END));
					emit(symbol(symbol_type::LINE_START));
					return std::make_pair(state::INITIAL, true);
				}
				token_.push_back(c);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_comment_multi_line(std::uint32_t c) {
				switch (c) {
				case '*':
					//	/* [*]/
					return std::make_pair(state::COMMENT_MULTI_LINE_END, true);
				case '\n':
					++line_number_;
					//	comment complete
					emit(symbol_type::COMMENT);
					emit(symbol(symbol_type::LINE_END));
					emit(symbol(symbol_type::LINE_START));
					return std::make_pair(state_, true);
				default:
					break;
				}
				
				token_.push_back(c);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_comment_multi_line_end(std::uint32_t c) {
				if (c == '/') {
					//	/* *[/]
					token_.push_back('*');
					token_.push_back(c);
					emit(symbol_type::COMMENT);
					return std::make_pair(state::INITIAL, true);
				}
				token_.push_back('*');
				token_.push_back(c);
				return std::make_pair(state::COMMENT_MULTI_LINE, true);
			}

			std::pair<state, bool> state_macro(std::uint32_t c) {
				switch (c) {
				case ' ': case '\t':
					emit(symbol_type::MACRO);
					return std::make_pair(state::INITIAL, false);
				case '\n':
					//	comment complete
					emit(symbol_type::MACRO);
					return std::make_pair(state::INITIAL, false);
				default:
					break;
				}

				token_.push_back(c);
				return std::make_pair(state_, true);
			}
			std::pair<state, bool> state_literal(std::uint32_t c) {
				switch (c) {
				case ' ': case '\t':
				case '\n':
				case ':': case '&': case '*':  
				case '<': case '>':
				case '(': case ')':
				case '{': case '}':
					emit();
					return std::make_pair(state::INITIAL, false);
				default:
					break;
				}

				token_.push_back(c);
				return std::make_pair(state_, true);
			}

			std::pair<state, bool> state_op(std::uint32_t c) {
				switch (c) {
				case '<': case '>':	//	&gt;
				case ':':
				case '+': case '-':
				case '*': case '/':
				case '=': case '!':
				case '?':
				case '&': case '%': case '|':
					token_.push_back(c);
					return std::make_pair(state_, true);
				default:
					break;
				}

				emit();
				return std::make_pair(state::INITIAL, false);
			}

			/**
			 * emit tmtoken_p_ as next symbol
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
					if (key_words_.contains(s)) {
						emit_(line_number_, symbol(symbol_type::KEYWORD, std::move(s)));
					}
					else if (ops_.contains(s)) {
						emit_(line_number_, symbol(symbol_type::OPERATOR, std::move(s)));
					}
					else if (known_classes_.contains(s)) {
						emit_(line_number_, symbol(symbol_type::LIBRARY, std::move(s)));
					}
					else {
						emit_(line_number_, symbol(symbol_type::LITERAL, std::move(s)));
					}
					token_.clear();
				}
			}

		private:
			std::size_t line_number_;
			std::function<void(std::size_t, symbol&&)> emit_;
			std::set<std::string> const	key_words_;
			std::set<std::string> const	ops_;
			std::set<std::string> const	known_classes_;
			std::u32string token_;
		};
	public:
		curly(std::ostream& os, bool numbers, std::initializer_list<std::string> key_words, std::initializer_list<std::string> ops, std::initializer_list<std::string> known_classes)
			: os_(os)
			, numbers_(numbers)
			, tokenizer_([this](std::size_t line, symbol&& sym) {
				this->next(line, std::move(sym));
			}, key_words, ops, known_classes)
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

			//	no more symbols
			next(-1, symbol(symbol_type::EOS));
		}

	private:
		void next(std::size_t line, symbol&& sym) {
			//std::cout << "#" << line << "\t" << sym.value_ << std::endl;
			switch (sym.type_) {
			case symbol_type::MARK:
				os_ << sym.value_;	// {, }, (, )
				break;
			case symbol_type::LINE_START:
				open_row(line);
				break;
			case symbol_type::LINE_END:
				close_row();
				break;
			case symbol_type::WS:
				os_ << sym.value_;	// "[WS]";
				break;
			case symbol_type::STRING:
				os_ << "&quot;<span class=\"docc-string\">";
				esc_html(os_, sym.value_);
				os_ << "</span>&quot;";
				break;
			case symbol_type::CHAR:
				os_ << "&apos;<span class=\"docc-string\">";
				esc_html(os_, sym.value_);
				os_ << "</span>&apos;";
				break;
			case symbol_type::COMMENT:
				os_ << "<span class=\"docc-comment\">";
				esc_html(os_, sym.value_);
				os_ << "</span>";
				break;
			case symbol_type::MACRO:
				os_ << "#<span class=\"docc-macro\">"; 
				esc_html(os_, sym.value_);
				os_ << "</span>";
				break;
			case symbol_type::KEYWORD:	//!<	int, long, while, ...
				os_ << "<span class=\"docc-keyword\">";
				esc_html(os_, sym.value_);
				os_ << "</span>";
				break;
			case symbol_type::OPERATOR:	//!<	<, >, ==, ...
				os_ << "<span class=\"docc-operator\">";
				esc_html(os_, sym.value_);
				os_ << "</span>";
				break;
			case symbol_type::LIBRARY:	//!<	known class
				os_ << "<span class=\"docc-class\">" << sym.value_ << "</span>";
				break;
			case symbol_type::LITERAL:	//!<	any literal string
				esc_html(os_, sym.value_);
				break;

			default:
				//os_ << "unknown(" << sym.value_ << ")!";
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

	void curly_to_html(std::ostream& os, 
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> start, 
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> end,
		bool numbers, 
		std::string const& lang) {

		if (boost::algorithm::equals(lang, "C++")) {
			curly c(os, numbers,
				{
					"alignas",
					"alignof",
					"and",
					"and_eq",
					"asm",
					"auto",
					"bool",
					"break",
					"case",
					"catch",
					"char",
					"char8_t",
					"char16_t",
					"char32_t",
					"class",
					"compl",	//	~
					"concept",
					"const",
					"const_cast",
					"consteval",
					"constexpr",
					"constinit",
					"continue",
					"co_await",
					"co_return",
					"co_yield",
					"decltype",
					"default",
					"delete",
					"do",
					"double",
					"dynamic_cast",
					"else",
					"enum",
					"explicit",
					"extern",
					"false",
					"float",
					"for",
					"friend",
					"goto",
					"if",
					"inline",
					"int",
					"long",
					"mutable",
					"namespace",
					"new",
					"noexcept",
					"not",
					"not_eq",
					"nullptr",
					"operator",
					"or",
					"or_eq",
					"private",
					"protected",
					"public",
					"register",
					"reinterpret_cast",
					"requires",
					"return",
					"short",
					"signed",
					"sizeof",
					"static",
					"static_assert",
					"switch",
					"template",
					"this",
					"thread_local",
					"throw",
					"true",
					"try",
					"typedef",
					"typeid",
					"typename",
					"union",
					"unsigned",
					"using",
					"virtual",
					"void",
					"volatile",
					"while" }
				, { "<", ">", "=", "==", "!=", "+" "-", "*", "/", "%", "+=", "-=", "*=", "/=", "&", "&&", "|", "||" }
				, {	
					"array", "deque", "set", "vector", "list", 
					"map", "multiset", "multimap", "unordered_set", "unordered_map", "unordered_multiset", "unordered_multimap",
					"stack", "queue", "priority_queue",
					"span", 
					"ostream", "istream", "ofstream", "ifstream", "cin", "cout", "cerr", "string",
					"istream_iterator", "ostream_iterator", "istreambuf_iterator", "ostreambuf_iterator",
					"complex",
					"pair", "tuple", "variant", "any", "optional", "bitset"
				});
			//	read
			c.read(start, end);
		}
		else if (boost::algorithm::equals(lang, "Java")) {
			curly c(os, numbers,
				{
					"abstract", "continue", "for", "new", "switch",
					"assert", "default", "goto", "package", "synchronized",
					"boolean", "do", "if", "private", "this",
					"break", "double", "implements", "protected", "throw",
					"byte", "else", "import", "public", "throws",
					"case", "enum", "instanceof", "return", "transient",
					"catch", "extends", "int", "short", "try",
					"char", "final", "interface", "static", "void", "var",
					"class", "finally", "long", "strictfp** volatile",
					"const", "float", "native", "super", "while"}
				, { "<", ">", "=", "==", "!=", "+" "-", "*", "/", "%", "+=", "-=", "*=", "/=", "&", "&&", "|", "||" }
				, { "Set", "List", "Queue", "Deque", "Map", "HashSet", "HashMap", "ArrayList", "ArrayDeque", "TreeSet", "TreeMap", "LinkedList", "LinkedHashSet", "LinkedHashMap"});

			//	read
			c.read(start, end);
		}
		else if (boost::algorithm::equals(lang, "JavaScript")) {
			curly c(os, numbers,
			{
				"abstract", "arguments", "await", 
				"boolean", "break", "byte",
				"case", "catch", "class", "const", "continue", 
				"debugger", "default", "delete", "do", 
				"else", "enum", "export", "extends",
				"false", "finally", "for", "function", 
				"if", "implements", "import", "in", "instanceof", "interface",
				"let", 
				"new", "null", 
				"package", "private", "protected", "public", 
				"return", 
				"super", "switch", "static", 
				"this", "throw", "try", "true",	"typeof", 
				"undefined",	//	?
				"var", "void", "volatile", 
				"while", "with",
				"yield"}
			, { "<", ">", "=", "==", "!=", "+" "-", "*", "/", "%", "+=", "-=", "*=", "/=", "&", "&&", "|", "||", "==="}
			, {"Date", "Array", "console"});

			//	read
			c.read(start, end);
		}
		else if (boost::algorithm::equals(lang, "C#")) {
			curly c(os, numbers, {}, {}, {});
			//	read
			c.read(start, end);
		}
		else if (boost::algorithm::equals(lang, "Zig")) {
			curly c(os, numbers,
			{
				"align",
				"allowzero",
				"and",
				"anyframe",
				"anytype",
				"asm",
				"async",
				"await",
				"break",
				"catch",
				"comptime",
				"const",
				"continue",
				"defer",
				"else",
				"enum",
				"errdefer",
				"error",
				"export",
				"extern",
				"false",
				"fn",
				"for",
				"if",
				"inline",
				"noalias",
				"nosuspend",
				"null",
				"or",
				"orelse",
				"packed",
				"pub",
				"resume",
				"return",
				"linksection",
				"struct",
				"suspend",
				"switch",
				"test",
				"threadlocal",
				"true",
				"try",
				"undefined",
				"union",
				"unreachable",
				"usingnamespace",
				"var",
				"volatile",
				"while"
				}
			, {}
			, {});
			//	read
			c.read(start, end);
		}
		else {
			curly c(os, numbers, {}, {}, {});
			//	read
			c.read(start, end);
		}
	}


}
