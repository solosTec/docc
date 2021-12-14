
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
			PUNCTUATION,	//	,;
			ARG_QUOTED_START,
			ARG_QUOTED,
			ARG_QUOTED_END,
			ARG_UNQUOTED,
			LITERAL,		//!<	any literal string
			SCOPE_START,	//!<	$ENV{...}, $CACHE{...}
			SCOPE_END,		//!<	...}

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
				SCOPE,		//	${...}, $ENV{...}, $CACHE{...}
				VARIABLE,		//	variable reference
				IDENTIFIER,	//	[A-Za-z_][A-Za-z0-9_]*
				ARG_QUOTED,
				ARG_QUOTED_ESC,
				ARG_UNQUOTED,
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
				switch (get_state()) {
				case state::INITIAL:
					return state_initial(c);
				case state::COMMENT:
					return state_comment(c);
				case state::SCOPE:
					return state_scope(c);
				case state::VARIABLE:
					return state_variable(c);
				case state::IDENTIFIER:
					return state_identifier(c);
				case state::ARG_QUOTED:
					return state_quoted(c);
				case state::ARG_QUOTED_ESC:
					return state_quoted_esc(c);
				case state::ARG_UNQUOTED:
					return state_unquoted(c);
				default:
					break;
				}
				BOOST_ASSERT_MSG(false, "illegal state");
				return true;
			}

		private:
			state get_state() const {
				return state_.empty()
					? state::INITIAL
					: state_.top()
					;
			}

			bool state_initial(std::uint32_t c) {
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
					state_.push(state::COMMENT);
					return true;
				case '$':
					state_.push(state::SCOPE);
					return true;
				case '"':
					token_.push_back(c);
					emit(symbol_type::ARG_QUOTED_START);
					state_.push(state::ARG_QUOTED);
					return true;
				case '(':	case ')':
				case ';':	case ',':
					token_.push_back(c);
					emit(symbol_type::PUNCTUATION);
					return true;
				default:
					token_.push_back(c);
					if ((c >= 'A' && c <= 'Z') 
						|| (c >= 'a' && c <= 'z') 
						|| (c == '_')) {

						state_.push(state::IDENTIFIER);
						return true;
					}
					else {
						//	argument - anything else
						state_.push(state::ARG_UNQUOTED);
					}

					return true;
				}
				return true;
			}
			bool state_comment(std::uint32_t c) {
				if (c == '\n') {
					++line_number_;
					//	comment complete
					emit(symbol_type::COMMENT);
					emit(symbol(symbol_type::LINE_END));
					emit(symbol(symbol_type::LINE_START));
					BOOST_ASSERT(!state_.empty());
					state_.pop();
					return true;
				}
				token_.push_back(c);
				return true;
			}

			bool state_scope(std::uint32_t c) {
				if (c == '{') {
					emit(symbol_type::SCOPE_START);
					BOOST_ASSERT(!state_.empty());
					state_.pop();
					state_.push(state::VARIABLE);
					return true;
				}
				BOOST_ASSERT(!state_.empty());
				state_.pop();
				token_.push_back('$');
				token_.push_back(c);	//	generator expression
				return true;
			}

			bool state_variable(std::uint32_t c) {
				switch (c)
				{
				case '}':
					emit(symbol_type::LITERAL);
					BOOST_ASSERT(!state_.empty());
					state_.pop();
					token_.push_back(c);
					emit(symbol_type::SCOPE_END);
					return true;
				case '$':
					state_.push(state::SCOPE);
					return true;
				default:
					break;
				}
				token_.push_back(c);
				return true;
			}

			bool state_quoted(std::uint32_t c) {
				//	any character except '\' or '"'
				switch (c) {
				case '"':
					emit(symbol_type::ARG_QUOTED);
					BOOST_ASSERT(!state_.empty());
					state_.pop();
					token_.push_back(c);
					emit(symbol_type::ARG_QUOTED_END);
					return true;
				case '\n':
					emit(symbol_type::ARG_QUOTED);
					BOOST_ASSERT(!state_.empty());
					state_.pop();
					token_.push_back('"');
					emit(symbol_type::ARG_QUOTED_END);
					return false;
				case '\\':
					state_.push(state::ARG_QUOTED_ESC);
					return true;
				case '$':
					emit(symbol_type::ARG_QUOTED);
					//	nested
					state_.push(state::SCOPE);
					return true;
				default:
					break;
				}
				token_.push_back(c);
				return true;
			}

			bool state_quoted_esc(std::uint32_t c) {
				switch (c) {
				case '\n':
					//	quoted_continuation
					break;
				case 'n': token_.push_back('\n'); break;
				case 'r': token_.push_back('\r'); break;
				case 't': token_.push_back('\t'); break;
				default:
					token_.push_back(c);
					break;
				}
				BOOST_ASSERT(!state_.empty());
				state_.pop();
				return true;
			}
			
			bool state_unquoted(std::uint32_t c) {
				//	any character except whitespace or one of '()#"\'
				switch (c) {
				case ' ': case '\t': 
				case '(': case ')':
				case '#': 
				case '"':
				case '\\':
				case '\n':
					emit(symbol_type::ARG_UNQUOTED);
					BOOST_ASSERT(!state_.empty());
					state_.pop();
					return false;
				default:
					break;
				}
				token_.push_back(c);
				return true;
			}

			bool state_identifier(std::uint32_t c) {
				if ((c >= 'A' && c <= 'Z') 
					|| (c >= 'a' && c <= 'z') 
					|| (c >= '0' && c <= '9')
					|| (c == '_')) {
					token_.push_back(c);
					return true;
				}
				emit(symbol_type::LITERAL);
				BOOST_ASSERT(!state_.empty());
				state_.pop();
				return false;
			}

			/**
			 * emit token as next symbol
			 */
			void emit(symbol_type st)
			{
				//if (!token_.empty()) {
					cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> start(token_.begin());
					cyng::utf8::u32_to_u8_iterator<std::u32string::const_iterator> end(token_.end());
					emit_(line_number_, symbol(st, std::string(start, end)));
					token_.clear();
				//}
			}
			void emit(symbol&& s) const
			{
				emit_(line_number_, std::move(s));
			}

		private:
			std::size_t line_number_;
			std::function<void(std::size_t, symbol&&)> emit_;
			std::u32string token_;
		};
	public:
		parser(std::ostream& os, bool numbers, std::initializer_list<std::string> cmd_list, std::initializer_list<std::string> var_list, std::initializer_list<std::string> prop_list)
			: os_(os)
			, numbers_(numbers)
			, cmd_list_(cmd_list.begin(), cmd_list.end())
			, var_list_(var_list.begin(), var_list.end())
			, prop_list_(prop_list.begin(), prop_list.end())
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
			case symbol_type::ARG_QUOTED_START:
				os_ << "<span class=\"docc-string\">&quot;";
				break;
			case symbol_type::ARG_QUOTED:
				esc_html(os_, sym.value_);
				break;
			case symbol_type::ARG_QUOTED_END:
				os_ << "&quot;</span>";
				break;
			case symbol_type::SCOPE_START:
				os_ << "<span class=\"docc-ref \">$";
				esc_html(os_, sym.value_);
				os_ << "{";
				break;
			case symbol_type::SCOPE_END:
				os_ << "}</span>";
				break;

			case symbol_type::ARG_UNQUOTED:
			case symbol_type::LITERAL:
				if (cmd_list_.contains(sym.value_)) {
					os_ << "<span class=\"docc-keyword\">";
					esc_html(os_, sym.value_);
					os_ << "</span>";
				}
				else if (var_list_.contains(sym.value_)) {
					os_ << "<span class=\"docc-class\">";
					esc_html(os_, sym.value_);
					os_ << "</span>";
				}
				else if (prop_list_.contains(sym.value_)) {
					os_ << "<span class=\"docc-operator\">";
					esc_html(os_, sym.value_);
					os_ << "</span>";
				}
				else {
					if (boost::algorithm::starts_with(sym.value_, "CMAKE_") 
						|| boost::algorithm::starts_with(sym.value_, "CPACK_")
						|| boost::algorithm::starts_with(sym.value_, "CTEST_")) 
					{
						//	variable
						os_ << "<span class=\"docc-class\">";
						esc_html(os_, sym.value_);
						os_ << "</span>";
					}
					else {
						os_ << sym.value_;
					}
				}
				break;
			case symbol_type::PUNCTUATION:
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

		//	cmake --help-command-list
		const std::set<std::string>	cmd_list_;

		//	cmake --help-variable-list
		const std::set<std::string>	var_list_;

		//	cmake --help-property-list
		const std::set<std::string>	prop_list_;
	};

	//const std::set<std::string>	parser::cmd_list_
	//const std::set<std::string>	parser::var_list_ 
	//const std::set<std::string>	parser::prop_list_ = 

	void cmake_to_html(std::ostream& os,
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> start,
		cyng::utf8::u8_to_u32_iterator<std::istream_iterator<char>> end,
		bool numbers) {

		parser p(os
			, numbers
			, {
				"add_compile_definitions",
				"add_compile_options",
				"add_custom_command",
				"add_custom_target",
				"add_definitions",
				"add_dependencies",
				"add_executable",
				"add_library",
				"add_link_options",
				"add_subdirectory",
				"add_test",
				"aux_source_directory",
				"break",
				"build_command",
				"build_name",
				"cmake_host_system_information",
				"cmake_minimum_required",
				"cmake_parse_arguments",
				"cmake_policy",
				"configure_file",
				"continue",
				"create_test_sourcelist",
				"ctest_build",
				"ctest_configure",
				"ctest_coverage",
				"ctest_empty_binary_directory",
				"ctest_memcheck",
				"ctest_read_custom_files",
				"ctest_run_script",
				"ctest_sleep",
				"ctest_start",
				"ctest_submit",
				"ctest_test",
				"ctest_update",
				"ctest_upload",
				"define_property",
				"else",
				"elseif",
				"enable_language",
				"enable_testing",
				"endforeach",
				"endfunction",
				"endif",
				"endmacro",
				"endwhile",
				"exec_program",
				"execute_process",
				"export",
				"export_library_dependencies",
				"file",
				"find_file",
				"find_library",
				"find_package",
				"find_path",
				"find_program",
				"fltk_wrap_ui",
				"foreach",
				"function",
				"get_cmake_property",
				"get_directory_property",
				"get_filename_component",
				"get_property",
				"get_source_file_property",
				"get_target_property",
				"get_test_property",
				"if",
				"include",
				"include_directories",
				"include_external_msproject",
				"include_guard",
				"include_regular_expression",
				"install",
				"install_files",
				"install_programs",
				"install_targets",
				"link_directories",
				"link_libraries",
				"list",
				"load_cache",
				"load_command",
				"macro",
				"make_directory",
				"mark_as_advanced",
				"math",
				"message",
				"option",
				"output_required_files",
				"project",
				"qt_wrap_cpp",
				"qt_wrap_ui",
				"remove",
				"remove_definitions",
				"return",
				"separate_arguments",
				"set",
				"set_directory_properties",
				"set_property",
				"set_source_files_properties",
				"set_target_properties",
				"set_tests_properties",
				"site_name",
				"source_group",
				"string",
				"subdir_depends",
				"subdirs",
				"target_compile_definitions",
				"target_compile_features",
				"target_compile_options",
				"target_include_directories",
				"target_link_directories",
				"target_link_libraries",
				"target_link_options",
				"target_precompile_headers",
				"target_sources",
				"try_compile",
				"try_run",
				"unset",
				"use_mangled_mesa",
				"utility_source",
				"variable_requires",
				"variable_watch",
				"while",
				"write_file"
			}
			, 
			{
				//"<PROJECT-NAME>_BINARY_DIR
				//"<PROJECT-NAME>_DESCRIPTION
				//"<PROJECT-NAME>_HOMEPAGE_URL
				//"<PROJECT-NAME>_SOURCE_DIR
				//"<PROJECT-NAME>_VERSION
				//"<PROJECT-NAME>_VERSION_MAJOR
				//"<PROJECT-NAME>_VERSION_MINOR
				//"<PROJECT-NAME>_VERSION_PATCH
				//"<PROJECT-NAME>_VERSION_TWEAK
				//"<PackageName>_ROOT
				"ANDROID",
				"APPLE",
				"BORLAND",
				"BUILD_SHARED_LIBS",
				"CACHE",
				//"CMAKE_<CONFIG>_POSTFIX
				//"CMAKE_<LANG>_ANDROID_TOOLCHAIN_MACHINE
				//"CMAKE_<LANG>_ANDROID_TOOLCHAIN_PREFIX
				//"CMAKE_<LANG>_ANDROID_TOOLCHAIN_SUFFIX
				//"CMAKE_<LANG>_ARCHIVE_APPEND
				//"CMAKE_<LANG>_ARCHIVE_CREATE
				//"CMAKE_<LANG>_ARCHIVE_FINISH
				//"CMAKE_<LANG>_CLANG_TIDY
				//"CMAKE_<LANG>_COMPILER
				//"CMAKE_<LANG>_COMPILER_ABI
				//"CMAKE_<LANG>_COMPILER_AR
				//"CMAKE_<LANG>_COMPILER_ARCHITECTURE_ID
				//"CMAKE_<LANG>_COMPILER_EXTERNAL_TOOLCHAIN
				//"CMAKE_<LANG>_COMPILER_ID
				//"CMAKE_<LANG>_COMPILER_LAUNCHER
				//"CMAKE_<LANG>_COMPILER_LOADED
				//"CMAKE_<LANG>_COMPILER_PREDEFINES_COMMAND
				//"CMAKE_<LANG>_COMPILER_RANLIB
				//"CMAKE_<LANG>_COMPILER_TARGET
				//"CMAKE_<LANG>_COMPILER_VERSION
				//"CMAKE_<LANG>_COMPILER_VERSION_INTERNAL
				//"CMAKE_<LANG>_COMPILE_OBJECT
				//"CMAKE_<LANG>_CPPCHECK
				//"CMAKE_<LANG>_CPPLINT
				//"CMAKE_<LANG>_CREATE_SHARED_LIBRARY
				//"CMAKE_<LANG>_CREATE_SHARED_MODULE
				//"CMAKE_<LANG>_CREATE_STATIC_LIBRARY
				//"CMAKE_<LANG>_FLAGS
				//"CMAKE_<LANG>_FLAGS_<CONFIG>
				//"CMAKE_<LANG>_FLAGS_<CONFIG>_INIT
				//"CMAKE_<LANG>_FLAGS_DEBUG
				//"CMAKE_<LANG>_FLAGS_DEBUG_INIT
				//"CMAKE_<LANG>_FLAGS_INIT
				//"CMAKE_<LANG>_FLAGS_MINSIZEREL
				//"CMAKE_<LANG>_FLAGS_MINSIZEREL_INIT
				//"CMAKE_<LANG>_FLAGS_RELEASE
				//"CMAKE_<LANG>_FLAGS_RELEASE_INIT
				//"CMAKE_<LANG>_FLAGS_RELWITHDEBINFO
				//"CMAKE_<LANG>_FLAGS_RELWITHDEBINFO_INIT
				//"CMAKE_<LANG>_IGNORE_EXTENSIONS
				//"CMAKE_<LANG>_IMPLICIT_INCLUDE_DIRECTORIES
				//"CMAKE_<LANG>_IMPLICIT_LINK_DIRECTORIES
				//"CMAKE_<LANG>_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES
				//"CMAKE_<LANG>_IMPLICIT_LINK_LIBRARIES
				//"CMAKE_<LANG>_INCLUDE_WHAT_YOU_USE
				//"CMAKE_<LANG>_LIBRARY_ARCHITECTURE
				//"CMAKE_<LANG>_LINKER_PREFERENCE
				//"CMAKE_<LANG>_LINKER_PREFERENCE_PROPAGATES
				//"CMAKE_<LANG>_LINKER_WRAPPER_FLAG
				//"CMAKE_<LANG>_LINKER_WRAPPER_FLAG_SEP
				//"CMAKE_<LANG>_LINK_EXECUTABLE
				//"CMAKE_<LANG>_LINK_LIBRARY_FILE_FLAG
				//"CMAKE_<LANG>_LINK_LIBRARY_FLAG
				//"CMAKE_<LANG>_LINK_LIBRARY_SUFFIX
				//"CMAKE_<LANG>_OUTPUT_EXTENSION
				//"CMAKE_<LANG>_PLATFORM_ID
				//"CMAKE_<LANG>_SIMULATE_ID
				//"CMAKE_<LANG>_SIMULATE_VERSION
				//"CMAKE_<LANG>_SIZEOF_DATA_PTR
				//"CMAKE_<LANG>_SOURCE_FILE_EXTENSIONS
				//"CMAKE_<LANG>_STANDARD_INCLUDE_DIRECTORIES
				//"CMAKE_<LANG>_STANDARD_LIBRARIES
				//"CMAKE_<LANG>_VISIBILITY_PRESET
				//"CMAKE_ABSOLUTE_DESTINATION_FILES",
				//"CMAKE_ANDROID_ANT_ADDITIONAL_OPTIONS",
				//"CMAKE_ANDROID_API",
				//"CMAKE_ANDROID_API_MIN",
				//"CMAKE_ANDROID_ARCH",
				//"CMAKE_ANDROID_ARCH_ABI",
				//"CMAKE_ANDROID_ARM_MODE",
				//"CMAKE_ANDROID_ARM_NEON",
				//"CMAKE_ANDROID_ASSETS_DIRECTORIES",
				//"CMAKE_ANDROID_GUI",
				//"CMAKE_ANDROID_JAR_DEPENDENCIES",
				//"CMAKE_ANDROID_JAR_DIRECTORIES",
				//"CMAKE_ANDROID_JAVA_SOURCE_DIR",
				//"CMAKE_ANDROID_NATIVE_LIB_DEPENDENCIES",
				//"CMAKE_ANDROID_NATIVE_LIB_DIRECTORIES",
				//"CMAKE_ANDROID_NDK",
				//"CMAKE_ANDROID_NDK_DEPRECATED_HEADERS",
				//"CMAKE_ANDROID_NDK_TOOLCHAIN_HOST_TAG",
				//"CMAKE_ANDROID_NDK_TOOLCHAIN_VERSION",
				//"CMAKE_ANDROID_PROCESS_MAX",
				//"CMAKE_ANDROID_PROGUARD",
				//"CMAKE_ANDROID_PROGUARD_CONFIG_PATH",
				//"CMAKE_ANDROID_SECURE_PROPS_PATH",
				//"CMAKE_ANDROID_SKIP_ANT_STEP",
				//"CMAKE_ANDROID_STANDALONE_TOOLCHAIN",
				//"CMAKE_ANDROID_STL_TYPE",
				//"CMAKE_APPBUNDLE_PATH",
				//"CMAKE_AR",
				//"CMAKE_ARCHIVE_OUTPUT_DIRECTORY",
				//"CMAKE_ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>
				//"CMAKE_ARGC",
				//"CMAKE_ARGV0",
				//"CMAKE_AUTOGEN_ORIGIN_DEPENDS",
				//"CMAKE_AUTOGEN_PARALLEL",
				//"CMAKE_AUTOGEN_VERBOSE",
				//"CMAKE_AUTOMOC",
				//"CMAKE_AUTOMOC_COMPILER_PREDEFINES",
				//"CMAKE_AUTOMOC_DEPEND_FILTERS",
				//"CMAKE_AUTOMOC_MACRO_NAMES",
				//"CMAKE_AUTOMOC_MOC_OPTIONS",
				//"CMAKE_AUTOMOC_PATH_PREFIX",
				//"CMAKE_AUTOMOC_RELAXED_MODE",
				//"CMAKE_AUTORCC",
				//"CMAKE_AUTORCC_OPTIONS",
				//"CMAKE_AUTOUIC",
				//"CMAKE_AUTOUIC_OPTIONS",
				//"CMAKE_AUTOUIC_SEARCH_PATHS",
				//"CMAKE_BACKWARDS_COMPATIBILITY",
				//"CMAKE_BINARY_DIR",
				//"CMAKE_BUILD_RPATH",
				//"CMAKE_BUILD_RPATH_USE_ORIGIN",
				//"CMAKE_BUILD_TOOL",
				//"CMAKE_BUILD_TYPE",
				//"CMAKE_BUILD_WITH_INSTALL_NAME_DIR",
				//"CMAKE_BUILD_WITH_INSTALL_RPATH",
				//"CMAKE_CACHEFILE_DIR",
				//"CMAKE_CACHE_MAJOR_VERSION",
				//"CMAKE_CACHE_MINOR_VERSION",
				//"CMAKE_CACHE_PATCH_VERSION",
				//"CMAKE_CFG_INTDIR",
				//"CMAKE_CL_64",
				//"CMAKE_CODEBLOCKS_COMPILER_ID",
				//"CMAKE_CODEBLOCKS_EXCLUDE_EXTERNAL_FILES",
				//"CMAKE_CODELITE_USE_TARGETS",
				//"CMAKE_COLOR_MAKEFILE",
				//"CMAKE_COMMAND",
				//"CMAKE_COMPILER_2005",
				//"CMAKE_COMPILER_IS_GNUCC",
				//"CMAKE_COMPILER_IS_GNUCXX",
				//"CMAKE_COMPILER_IS_GNUG77",
				//"CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY",
				//"CMAKE_COMPILE_PDB_OUTPUT_DIRECTORY_<CONFIG>
				//"CMAKE_CONFIGURATION_TYPES",
				//"CMAKE_CPACK_COMMAND",
				//"CMAKE_CROSSCOMPILING",
				//"CMAKE_CROSSCOMPILING_EMULATOR",
				//"CMAKE_CTEST_COMMAND",
				//"CMAKE_CUDA_EXTENSIONS",
				//"CMAKE_CUDA_HOST_COMPILER",
				//"CMAKE_CUDA_RESOLVE_DEVICE_SYMBOLS",
				//"CMAKE_CUDA_SEPARABLE_COMPILATION",
				//"CMAKE_CUDA_STANDARD",
				//"CMAKE_CUDA_STANDARD_REQUIRED",
				//"CMAKE_CUDA_TOOLKIT_INCLUDE_DIRECTORIES",
				//"CMAKE_CURRENT_BINARY_DIR",
				//"CMAKE_CURRENT_LIST_DIR",
				//"CMAKE_CURRENT_LIST_FILE",
				//"CMAKE_CURRENT_LIST_LINE",
				//"CMAKE_CURRENT_SOURCE_DIR",
				//"CMAKE_CXX_COMPILE_FEATURES",
				//"CMAKE_CXX_EXTENSIONS",
				//"CMAKE_CXX_STANDARD",
				//"CMAKE_CXX_STANDARD_REQUIRED",
				//"CMAKE_C_COMPILE_FEATURES",
				//"CMAKE_C_EXTENSIONS",
				//"CMAKE_C_STANDARD",
				//"CMAKE_C_STANDARD_REQUIRED",
				//"CMAKE_DEBUG_POSTFIX",
				//"CMAKE_DEBUG_TARGET_PROPERTIES",
				//"CMAKE_DEPENDS_IN_PROJECT_ONLY",
				//"CMAKE_DIRECTORY_LABELS",
				//"CMAKE_DISABLE_FIND_PACKAGE_<PackageName>
				//"CMAKE_DISABLE_PRECOMPILE_HEADERS",
				//"CMAKE_DL_LIBS",
				//"CMAKE_DOTNET_TARGET_FRAMEWORK_VERSION",
				//"CMAKE_ECLIPSE_GENERATE_LINKED_RESOURCES",
				//"CMAKE_ECLIPSE_GENERATE_SOURCE_PROJECT",
				//"CMAKE_ECLIPSE_MAKE_ARGUMENTS",
				//"CMAKE_ECLIPSE_RESOURCE_ENCODING",
				//"CMAKE_ECLIPSE_VERSION",
				//"CMAKE_EDIT_COMMAND",
				//"CMAKE_ENABLE_EXPORTS",
				//"CMAKE_ERROR_DEPRECATED",
				//"CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION",
				//"CMAKE_EXECUTABLE_SUFFIX",
				//"CMAKE_EXECUTE_PROCESS_COMMAND_ECHO",
				//"CMAKE_EXE_LINKER_FLAGS",
				//"CMAKE_EXE_LINKER_FLAGS_<CONFIG>
				//"CMAKE_EXE_LINKER_FLAGS_<CONFIG>_INIT
				//"CMAKE_EXE_LINKER_FLAGS_INIT",
				//"CMAKE_EXPORT_COMPILE_COMMANDS",
				//"CMAKE_EXPORT_NO_PACKAGE_REGISTRY",
				//"CMAKE_EXPORT_PACKAGE_REGISTRY",
				//"CMAKE_EXTRA_GENERATOR",
				//"CMAKE_EXTRA_SHARED_LIBRARY_SUFFIXES",
				//"CMAKE_FIND_APPBUNDLE",
				//"CMAKE_FIND_FRAMEWORK",
				//"CMAKE_FIND_LIBRARY_CUSTOM_LIB_SUFFIX",
				//"CMAKE_FIND_LIBRARY_PREFIXES",
				//"CMAKE_FIND_LIBRARY_SUFFIXES",
				//"CMAKE_FIND_NO_INSTALL_PREFIX",
				//"CMAKE_FIND_PACKAGE_NAME",
				//"CMAKE_FIND_PACKAGE_NO_PACKAGE_REGISTRY",
				//"CMAKE_FIND_PACKAGE_NO_SYSTEM_PACKAGE_REGISTRY",
				//"CMAKE_FIND_PACKAGE_PREFER_CONFIG",
				//"CMAKE_FIND_PACKAGE_RESOLVE_SYMLINKS",
				//"CMAKE_FIND_PACKAGE_SORT_DIRECTION",
				//"CMAKE_FIND_PACKAGE_SORT_ORDER",
				//"CMAKE_FIND_PACKAGE_WARN_NO_MODULE",
				//"CMAKE_FIND_ROOT_PATH",
				//"CMAKE_FIND_ROOT_PATH_MODE_INCLUDE",
				//"CMAKE_FIND_ROOT_PATH_MODE_LIBRARY",
				//"CMAKE_FIND_ROOT_PATH_MODE_PACKAGE",
				//"CMAKE_FIND_ROOT_PATH_MODE_PROGRAM",
				//"CMAKE_FIND_USE_CMAKE_ENVIRONMENT_PATH",
				//"CMAKE_FIND_USE_CMAKE_PATH",
				//"CMAKE_FIND_USE_CMAKE_SYSTEM_PATH",
				//"CMAKE_FIND_USE_PACKAGE_REGISTRY",
				//"CMAKE_FIND_USE_PACKAGE_ROOT_PATH",
				//"CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH",
				//"CMAKE_FIND_USE_SYSTEM_PACKAGE_REGISTRY",
				//"CMAKE_FOLDER",
				//"CMAKE_FRAMEWORK",
				//"CMAKE_FRAMEWORK_PATH",
				//"CMAKE_Fortran_FORMAT",
				//"CMAKE_Fortran_MODDIR_DEFAULT",
				//"CMAKE_Fortran_MODDIR_FLAG",
				//"CMAKE_Fortran_MODOUT_FLAG",
				//"CMAKE_Fortran_MODULE_DIRECTORY",
				//"CMAKE_GENERATOR",
				//"CMAKE_GENERATOR_INSTANCE",
				//"CMAKE_GENERATOR_PLATFORM",
				//"CMAKE_GENERATOR_TOOLSET",
				//"CMAKE_GHS_NO_SOURCE_GROUP_FILE",
				//"CMAKE_GLOBAL_AUTOGEN_TARGET",
				//"CMAKE_GLOBAL_AUTOGEN_TARGET_NAME",
				//"CMAKE_GLOBAL_AUTORCC_TARGET",
				//"CMAKE_GLOBAL_AUTORCC_TARGET_NAME",
				//"CMAKE_GNUtoMS",
				//"CMAKE_HOME_DIRECTORY",
				//"CMAKE_HOST_APPLE",
				//"CMAKE_HOST_SOLARIS",
				//"CMAKE_HOST_SYSTEM",
				//"CMAKE_HOST_SYSTEM_NAME",
				//"CMAKE_HOST_SYSTEM_PROCESSOR",
				//"CMAKE_HOST_SYSTEM_VERSION",
				//"CMAKE_HOST_UNIX",
				//"CMAKE_HOST_WIN32",
				//"CMAKE_IGNORE_PATH",
				//"CMAKE_IMPORT_LIBRARY_PREFIX",
				//"CMAKE_IMPORT_LIBRARY_SUFFIX",
				//"CMAKE_INCLUDE_CURRENT_DIR",
				//"CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE",
				//"CMAKE_INCLUDE_DIRECTORIES_BEFORE",
				//"CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE",
				//"CMAKE_INCLUDE_PATH",
				//"CMAKE_INSTALL_DEFAULT_COMPONENT_NAME",
				//"CMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS",
				//"CMAKE_INSTALL_MESSAGE",
				//"CMAKE_INSTALL_NAME_DIR",
				//"CMAKE_INSTALL_PREFIX",
				//"CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT",
				//"CMAKE_INSTALL_REMOVE_ENVIRONMENT_RPATH",
				//"CMAKE_INSTALL_RPATH",
				//"CMAKE_INSTALL_RPATH_USE_LINK_PATH",
				//"CMAKE_INTERNAL_PLATFORM_ABI",
				//"CMAKE_INTERPROCEDURAL_OPTIMIZATION",
				//"CMAKE_INTERPROCEDURAL_OPTIMIZATION_<CONFIG>",
				//"CMAKE_IOS_INSTALL_COMBINED",
				//"CMAKE_JOB_POOLS",
				//"CMAKE_JOB_POOL_COMPILE",
				//"CMAKE_JOB_POOL_LINK",
				//"CMAKE_LIBRARY_ARCHITECTURE",
				//"CMAKE_LIBRARY_ARCHITECTURE_REGEX",
				//"CMAKE_LIBRARY_OUTPUT_DIRECTORY",
				//"CMAKE_LIBRARY_OUTPUT_DIRECTORY_<CONFIG>",
				//"CMAKE_LIBRARY_PATH",
				//"CMAKE_LIBRARY_PATH_FLAG",
				//"CMAKE_LINK_DEF_FILE_FLAG",
				//"CMAKE_LINK_DEPENDS_NO_SHARED",
				//"CMAKE_LINK_DIRECTORIES_BEFORE",
				//"CMAKE_LINK_INTERFACE_LIBRARIES",
				//"CMAKE_LINK_LIBRARY_FILE_FLAG",
				//"CMAKE_LINK_LIBRARY_FLAG",
				//"CMAKE_LINK_LIBRARY_SUFFIX",
				//"CMAKE_LINK_SEARCH_END_STATIC",
				//"CMAKE_LINK_SEARCH_START_STATIC",
				//"CMAKE_LINK_WHAT_YOU_USE",
				//"CMAKE_MACOSX_BUNDLE",
				//"CMAKE_MACOSX_RPATH",
				//"CMAKE_MAJOR_VERSION",
				//"CMAKE_MAKE_PROGRAM",
				//"CMAKE_MAP_IMPORTED_CONFIG_<CONFIG>",
				//"CMAKE_MATCH_<n>",
				//"CMAKE_MATCH_COUNT",
				//"CMAKE_MAXIMUM_RECURSION_DEPTH",
				//"CMAKE_MESSAGE_INDENT",
				//"CMAKE_MFC_FLAG",
				//"CMAKE_MINIMUM_REQUIRED_VERSION",
				//"CMAKE_MINOR_VERSION",
				//"CMAKE_MODULE_LINKER_FLAGS",
				//"CMAKE_MODULE_LINKER_FLAGS_<CONFIG>",
				//"CMAKE_MODULE_LINKER_FLAGS_<CONFIG>_INIT",
				//"CMAKE_MODULE_LINKER_FLAGS_INIT",
				//"CMAKE_MODULE_PATH",
				//"CMAKE_MSVCIDE_RUN_PATH",
				//"CMAKE_MSVC_RUNTIME_LIBRARY",
				//"CMAKE_NETRC",
				//"CMAKE_NETRC_FILE",
				//"CMAKE_NINJA_OUTPUT_PATH_PREFIX",
				//"CMAKE_NOT_USING_CONFIG_FLAGS",
				//"CMAKE_NO_BUILTIN_CHRPATH",
				//"CMAKE_NO_SYSTEM_FROM_IMPORTED",
				//"CMAKE_OBJCXX_EXTENSIONS",
				//"CMAKE_OBJCXX_STANDARD",
				//"CMAKE_OBJCXX_STANDARD_REQUIRED",
				//"CMAKE_OBJC_EXTENSIONS",
				//"CMAKE_OBJC_STANDARD",
				//"CMAKE_OBJC_STANDARD_REQUIRED",
				//"CMAKE_OBJECT_PATH_MAX",
				//"CMAKE_OSX_ARCHITECTURES",
				//"CMAKE_OSX_DEPLOYMENT_TARGET",
				//"CMAKE_OSX_SYSROOT",
				//"CMAKE_PARENT_LIST_FILE",
				//"CMAKE_PATCH_VERSION",
				//"CMAKE_PDB_OUTPUT_DIRECTORY",
				//"CMAKE_PDB_OUTPUT_DIRECTORY_<CONFIG>",
				//"CMAKE_POLICY_DEFAULT_CMP<NNNN>",
				//"CMAKE_POLICY_WARNING_CMP<NNNN>",
				//"CMAKE_POSITION_INDEPENDENT_CODE",
				//"CMAKE_PREFIX_PATH",
				//"CMAKE_PROGRAM_PATH",
				//"CMAKE_PROJECT_<PROJECT-NAME>_INCLUDE",
				//"CMAKE_PROJECT_DESCRIPTION",
				//"CMAKE_PROJECT_HOMEPAGE_URL",
				//"CMAKE_PROJECT_INCLUDE",
				//"CMAKE_PROJECT_INCLUDE_BEFORE",
				//"CMAKE_PROJECT_NAME",
				//"CMAKE_PROJECT_VERSION",
				//"CMAKE_PROJECT_VERSION_MAJOR",
				//"CMAKE_PROJECT_VERSION_MINOR",
				//"CMAKE_PROJECT_VERSION_PATCH",
				//"CMAKE_PROJECT_VERSION_TWEAK",
				//"CMAKE_RANLIB",
				//"CMAKE_ROOT",
				//"CMAKE_RULE_MESSAGES",
				//"CMAKE_RUNTIME_OUTPUT_DIRECTORY",
				//"CMAKE_RUNTIME_OUTPUT_DIRECTORY_<CONFIG>",
				//"CMAKE_SCRIPT_MODE_FILE",
				//"CMAKE_SHARED_LIBRARY_PREFIX",
				//"CMAKE_SHARED_LIBRARY_SUFFIX",
				//"CMAKE_SHARED_LINKER_FLAGS",
				//"CMAKE_SHARED_LINKER_FLAGS_<CONFIG>",
				//"CMAKE_SHARED_LINKER_FLAGS_<CONFIG>_INIT",
				//"CMAKE_SHARED_LINKER_FLAGS_INIT",
				//"CMAKE_SHARED_MODULE_PREFIX",
				//"CMAKE_SHARED_MODULE_SUFFIX",
				//"CMAKE_SIZEOF_VOID_P",
				//"CMAKE_SKIP_BUILD_RPATH",
				//"CMAKE_SKIP_INSTALL_ALL_DEPENDENCY",
				//"CMAKE_SKIP_INSTALL_RPATH",
				//"CMAKE_SKIP_INSTALL_RULES",
				//"CMAKE_SKIP_RPATH",
				//"CMAKE_SOURCE_DIR",
				//"CMAKE_STAGING_PREFIX",
				//"CMAKE_STATIC_LIBRARY_PREFIX",
				//"CMAKE_STATIC_LIBRARY_SUFFIX",
				//"CMAKE_STATIC_LINKER_FLAGS",
				//"CMAKE_STATIC_LINKER_FLAGS_<CONFIG>",
				//"CMAKE_STATIC_LINKER_FLAGS_<CONFIG>_INIT",
				//"CMAKE_STATIC_LINKER_FLAGS_INIT",
				//"CMAKE_SUBLIME_TEXT_2_ENV_SETTINGS",
				//"CMAKE_SUBLIME_TEXT_2_EXCLUDE_BUILD_TREE",
				//"CMAKE_SUPPRESS_REGENERATION",
				//"CMAKE_SYSROOT",
				//"CMAKE_SYSROOT_COMPILE",
				//"CMAKE_SYSROOT_LINK",
				//"CMAKE_SYSTEM",
				//"CMAKE_SYSTEM_APPBUNDLE_PATH",
				//"CMAKE_SYSTEM_FRAMEWORK_PATH",
				//"CMAKE_SYSTEM_IGNORE_PATH",
				//"CMAKE_SYSTEM_INCLUDE_PATH",
				//"CMAKE_SYSTEM_LIBRARY_PATH",
				//"CMAKE_SYSTEM_NAME",
				//"CMAKE_SYSTEM_PREFIX_PATH",
				//"CMAKE_SYSTEM_PROCESSOR",
				//"CMAKE_SYSTEM_PROGRAM_PATH",
				//"CMAKE_SYSTEM_VERSION",
				//"CMAKE_Swift_LANGUAGE_VERSION",
				//"CMAKE_Swift_MODULE_DIRECTORY",
				//"CMAKE_Swift_NUM_THREADS",
				//"CMAKE_TOOLCHAIN_FILE",
				//"CMAKE_TRY_COMPILE_CONFIGURATION",
				//"CMAKE_TRY_COMPILE_PLATFORM_VARIABLES",
				//"CMAKE_TRY_COMPILE_TARGET_TYPE",
				//"CMAKE_TWEAK_VERSION",
				//"CMAKE_UNITY_BUILD",
				//"CMAKE_UNITY_BUILD_BATCH_SIZE",
				//"CMAKE_USER_MAKE_RULES_OVERRIDE",
				//"CMAKE_USER_MAKE_RULES_OVERRIDE_<LANG>",
				//"CMAKE_USE_RELATIVE_PATHS",
				//"CMAKE_VERBOSE_MAKEFILE",
				//"CMAKE_VERSION",
				//"CMAKE_VISIBILITY_INLINES_HIDDEN",
				//"CMAKE_VS_DEVENV_COMMAND",
				//"CMAKE_VS_GLOBALS",
				//"CMAKE_VS_INCLUDE_INSTALL_TO_DEFAULT_BUILD",
				//"CMAKE_VS_INCLUDE_PACKAGE_TO_DEFAULT_BUILD",
				//"CMAKE_VS_INTEL_Fortran_PROJECT_VERSION",
				//"CMAKE_VS_JUST_MY_CODE_DEBUGGING",
				//"CMAKE_VS_MSBUILD_COMMAND",
				//"CMAKE_VS_NsightTegra_VERSION",
				//"CMAKE_VS_PLATFORM_NAME",
				//"CMAKE_VS_PLATFORM_NAME_DEFAULT",
				//"CMAKE_VS_PLATFORM_TOOLSET",
				//"CMAKE_VS_PLATFORM_TOOLSET_CUDA",
				//"CMAKE_VS_PLATFORM_TOOLSET_CUDA_CUSTOM_DIR",
				//"CMAKE_VS_PLATFORM_TOOLSET_HOST_ARCHITECTURE",
				//"CMAKE_VS_PLATFORM_TOOLSET_VERSION",
				//"CMAKE_VS_SDK_EXCLUDE_DIRECTORIES",
				//"CMAKE_VS_SDK_EXECUTABLE_DIRECTORIES",
				//"CMAKE_VS_SDK_INCLUDE_DIRECTORIES",
				//"CMAKE_VS_SDK_LIBRARY_DIRECTORIES",
				//"CMAKE_VS_SDK_LIBRARY_WINRT_DIRECTORIES",
				//"CMAKE_VS_SDK_REFERENCE_DIRECTORIES",
				//"CMAKE_VS_SDK_SOURCE_DIRECTORIES",
				//"CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION",
				//"CMAKE_VS_WINRT_BY_DEFAULT",
				//"CMAKE_WARN_DEPRECATED",
				//"CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION",
				//"CMAKE_WIN32_EXECUTABLE",
				//"CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS",
				//"CMAKE_XCODE_ATTRIBUTE_<an - attribute>",
				//"CMAKE_XCODE_GENERATE_SCHEME",
				//"CMAKE_XCODE_GENERATE_TOP_LEVEL_PROJECT_ONLY",
				//"CMAKE_XCODE_PLATFORM_TOOLSET",
				//"CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER",
				//"CMAKE_XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN",
				//"CMAKE_XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING",
				//"CMAKE_XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER",
				//"CMAKE_XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS",
				//"CMAKE_XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE",
				//"CMAKE_XCODE_SCHEME_GUARD_MALLOC",
				//"CMAKE_XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP",
				//"CMAKE_XCODE_SCHEME_MALLOC_GUARD_EDGES",
				//"CMAKE_XCODE_SCHEME_MALLOC_SCRIBBLE",
				//"CMAKE_XCODE_SCHEME_MALLOC_STACK",
				//"CMAKE_XCODE_SCHEME_THREAD_SANITIZER",
				//"CMAKE_XCODE_SCHEME_THREAD_SANITIZER_STOP",
				//"CMAKE_XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER",
				//"CMAKE_XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP",
				//"CMAKE_XCODE_SCHEME_ZOMBIE_OBJECTS",

				//"CPACK_ABSOLUTE_DESTINATION_FILES",
				//"CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY",
				//"CPACK_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION",
				//"CPACK_INCLUDE_TOPLEVEL_DIRECTORY",
				//"CPACK_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS",
				//"CPACK_PACKAGING_INSTALL_PREFIX",
				//"CPACK_SET_DESTDIR",
				//"CPACK_WARN_ON_ABSOLUTE_INSTALL_DESTINATION",

				//"CTEST_BINARY_DIRECTORY",
				//"CTEST_BUILD_COMMAND",
				//"CTEST_BUILD_NAME",
				//"CTEST_BZR_COMMAND",
				//"CTEST_BZR_UPDATE_OPTIONS",
				//"CTEST_CHANGE_ID",
				//"CTEST_CHECKOUT_COMMAND",
				//"CTEST_CONFIGURATION_TYPE",
				//"CTEST_CONFIGURE_COMMAND",
				//"CTEST_COVERAGE_COMMAND",
				//"CTEST_COVERAGE_EXTRA_FLAGS",
				//"CTEST_CURL_OPTIONS",
				//"CTEST_CUSTOM_COVERAGE_EXCLUDE",
				//"CTEST_CUSTOM_ERROR_EXCEPTION",
				//"CTEST_CUSTOM_ERROR_MATCH",
				//"CTEST_CUSTOM_ERROR_POST_CONTEXT",
				//"CTEST_CUSTOM_ERROR_PRE_CONTEXT",
				//"CTEST_CUSTOM_MAXIMUM_FAILED_TEST_OUTPUT_SIZE",
				//"CTEST_CUSTOM_MAXIMUM_NUMBER_OF_ERRORS",
				//"CTEST_CUSTOM_MAXIMUM_NUMBER_OF_WARNINGS",
				//"CTEST_CUSTOM_MAXIMUM_PASSED_TEST_OUTPUT_SIZE",
				//"CTEST_CUSTOM_MEMCHECK_IGNORE",
				//"CTEST_CUSTOM_POST_MEMCHECK",
				//"CTEST_CUSTOM_POST_TEST",
				//"CTEST_CUSTOM_PRE_MEMCHECK",
				//"CTEST_CUSTOM_PRE_TEST",
				//"CTEST_CUSTOM_TESTS_IGNORE",
				//"CTEST_CUSTOM_WARNING_EXCEPTION",
				//"CTEST_CUSTOM_WARNING_MATCH",
				//"CTEST_CVS_CHECKOUT",
				//"CTEST_CVS_COMMAND",
				//"CTEST_CVS_UPDATE_OPTIONS",
				//"CTEST_DROP_LOCATION",
				//"CTEST_DROP_METHOD",
				//"CTEST_DROP_SITE",
				//"CTEST_DROP_SITE_CDASH",
				//"CTEST_DROP_SITE_PASSWORD",
				//"CTEST_DROP_SITE_USER",
				//"CTEST_EXTRA_COVERAGE_GLOB",
				//"CTEST_GIT_COMMAND",
				//"CTEST_GIT_INIT_SUBMODULES",
				//"CTEST_GIT_UPDATE_CUSTOM",
				//"CTEST_GIT_UPDATE_OPTIONS",
				//"CTEST_HG_COMMAND",
				//"CTEST_HG_UPDATE_OPTIONS",
				//"CTEST_LABELS_FOR_SUBPROJECTS",
				//"CTEST_MEMORYCHECK_COMMAND",
				//"CTEST_MEMORYCHECK_COMMAND_OPTIONS",
				//"CTEST_MEMORYCHECK_SANITIZER_OPTIONS",
				//"CTEST_MEMORYCHECK_SUPPRESSIONS_FILE",
				//"CTEST_MEMORYCHECK_TYPE",
				//"CTEST_NIGHTLY_START_TIME",
				//"CTEST_P4_CLIENT",
				//"CTEST_P4_COMMAND",
				//"CTEST_P4_OPTIONS",
				//"CTEST_P4_UPDATE_OPTIONS",
				//"CTEST_RUN_CURRENT_SCRIPT",
				//"CTEST_SCP_COMMAND",
				//"CTEST_SITE",
				//"CTEST_SOURCE_DIRECTORY",
				//"CTEST_SUBMIT_URL",
				//"CTEST_SVN_COMMAND",
				//"CTEST_SVN_OPTIONS",
				//"CTEST_SVN_UPDATE_OPTIONS",
				//"CTEST_TEST_LOAD",
				//"CTEST_TEST_TIMEOUT",
				//"CTEST_TRIGGER_SITE",
				//"CTEST_UPDATE_COMMAND",
				//"CTEST_UPDATE_OPTIONS",
				//"CTEST_UPDATE_VERSION_ONLY",
				//"CTEST_UPDATE_VERSION_OVERRIDE",
				//"CTEST_USE_LAUNCHERS",

				"CYGWIN",
				"ENV",
				"EXECUTABLE_OUTPUT_PATH",
				"GHS-MULTI",
				"IOS",
				"LIBRARY_OUTPUT_PATH",
				"MINGW",
				"MSVC",
				"MSVC10",
				"MSVC11",
				"MSVC12",
				"MSVC14",
				"MSVC60",
				"MSVC70",
				"MSVC71",
				"MSVC80",
				"MSVC90",
				"MSVC_IDE",
				"MSVC_TOOLSET_VERSION",
				"MSVC_VERSION",
				"MSYS",
				"PROJECT_BINARY_DIR",
				"PROJECT_DESCRIPTION",
				"PROJECT_HOMEPAGE_URL",
				"PROJECT_NAME",
				"PROJECT_SOURCE_DIR",
				"PROJECT_VERSION",
				"PROJECT_VERSION_MAJOR",
				"PROJECT_VERSION_MINOR",
				"PROJECT_VERSION_PATCH",
				"PROJECT_VERSION_TWEAK",
				"UNIX",
				"WIN32",
				"WINCE",
				"WINDOWS_PHONE",
				"WINDOWS_STORE",
				"XCODE",
				"XCODE_VERSION"
			}
			, 
			{
				//<CONFIG>_OUTPUT_NAME
				//<CONFIG>_POSTFIX
				//<LANG>_CLANG_TIDY
				//<LANG>_COMPILER_LAUNCHER
				//<LANG>_CPPCHECK
				//<LANG>_CPPLINT
				//<LANG>_INCLUDE_WHAT_YOU_USE
				//<LANG>_VISIBILITY_PRESET
				"ABSTRACT",
				"ADDITIONAL_CLEAN_FILES",
				"ADDITIONAL_CLEAN_FILES",
				"ADDITIONAL_MAKE_CLEAN_FILES",
				"ADVANCED",
				"ALIASED_TARGET",
				"ALLOW_DUPLICATE_CUSTOM_TARGETS",
				"ANDROID_ANT_ADDITIONAL_OPTIONS",
				"ANDROID_API",
				"ANDROID_API_MIN",
				"ANDROID_ARCH",
				"ANDROID_ASSETS_DIRECTORIES",
				"ANDROID_GUI",
				"ANDROID_JAR_DEPENDENCIES",
				"ANDROID_JAR_DIRECTORIES",
				"ANDROID_JAVA_SOURCE_DIR",
				"ANDROID_NATIVE_LIB_DEPENDENCIES",
				"ANDROID_NATIVE_LIB_DIRECTORIES",
				"ANDROID_PROCESS_MAX",
				"ANDROID_PROGUARD",
				"ANDROID_PROGUARD_CONFIG_PATH",
				"ANDROID_SECURE_PROPS_PATH",
				"ANDROID_SKIP_ANT_STEP",
				"ANDROID_STL_TYPE",
				"ARCHIVE_OUTPUT_DIRECTORY",
				//"ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>",
				"ARCHIVE_OUTPUT_NAME",
				//"ARCHIVE_OUTPUT_NAME_<CONFIG>",
				"ATTACHED_FILES",
				"ATTACHED_FILES_ON_FAIL",
				"AUTOGEN_BUILD_DIR",
				"AUTOGEN_ORIGIN_DEPENDS",
				"AUTOGEN_PARALLEL",
				"AUTOGEN_SOURCE_GROUP",
				"AUTOGEN_TARGETS_FOLDER",
				"AUTOGEN_TARGET_DEPENDS",
				"AUTOMOC",
				"AUTOMOC_COMPILER_PREDEFINES",
				"AUTOMOC_DEPEND_FILTERS",
				"AUTOMOC_EXECUTABLE",
				"AUTOMOC_MACRO_NAMES",
				"AUTOMOC_MOC_OPTIONS",
				"AUTOMOC_PATH_PREFIX",
				"AUTOMOC_SOURCE_GROUP",
				"AUTOMOC_TARGETS_FOLDER",
				"AUTORCC",
				"AUTORCC_EXECUTABLE",
				"AUTORCC_OPTIONS",
				"AUTORCC_OPTIONS",
				"AUTORCC_SOURCE_GROUP",
				"AUTOUIC",
				"AUTOUIC_EXECUTABLE",
				"AUTOUIC_OPTIONS",
				"AUTOUIC_OPTIONS",
				"AUTOUIC_SEARCH_PATHS",
				"BINARY_DIR",
				"BINARY_DIR",
				"BUILDSYSTEM_TARGETS",
				"BUILD_RPATH",
				"BUILD_RPATH_USE_ORIGIN",
				"BUILD_WITH_INSTALL_NAME_DIR",
				"BUILD_WITH_INSTALL_RPATH",
				"BUNDLE",
				"BUNDLE_EXTENSION",
				"CACHE_VARIABLES",
				"CLEAN_NO_CUSTOM",
				"CMAKE_CONFIGURE_DEPENDS",
				"CMAKE_CXX_KNOWN_FEATURES",
				"CMAKE_C_KNOWN_FEATURES",
				"CMAKE_ROLE",
				"COMMON_LANGUAGE_RUNTIME",
				"COMPATIBLE_INTERFACE_BOOL",
				"COMPATIBLE_INTERFACE_NUMBER_MAX",
				"COMPATIBLE_INTERFACE_NUMBER_MIN",
				"COMPATIBLE_INTERFACE_STRING",
				"COMPILE_DEFINITIONS",
				"COMPILE_DEFINITIONS",
				"COMPILE_DEFINITIONS",
				//"COMPILE_DEFINITIONS_<CONFIG>",
				//"COMPILE_DEFINITIONS_<CONFIG>",
				//"COMPILE_DEFINITIONS_<CONFIG>",
				"COMPILE_FEATURES",
				"COMPILE_FLAGS",
				"COMPILE_FLAGS",
				"COMPILE_OPTIONS",
				"COMPILE_OPTIONS",
				"COMPILE_OPTIONS",
				"COMPILE_PDB_NAME",
				//"COMPILE_PDB_NAME_<CONFIG>",
				"COMPILE_PDB_OUTPUT_DIRECTORY",
				//"COMPILE_PDB_OUTPUT_DIRECTORY_<CONFIG>",
				"COST",
				"CPACK_DESKTOP_SHORTCUTS",
				"CPACK_NEVER_OVERWRITE",
				"CPACK_PERMANENT",
				"CPACK_STARTUP_SHORTCUTS",
				"CPACK_START_MENU_SHORTCUTS",
				"CPACK_WIX_ACL",
				"CROSSCOMPILING_EMULATOR",
				"CUDA_EXTENSIONS",
				"CUDA_PTX_COMPILATION",
				"CUDA_RESOLVE_DEVICE_SYMBOLS",
				"CUDA_SEPARABLE_COMPILATION",
				"CUDA_STANDARD",
				"CUDA_STANDARD_REQUIRED",
				"CXX_EXTENSIONS",
				"CXX_STANDARD",
				"CXX_STANDARD_REQUIRED",
				"C_EXTENSIONS",
				"C_STANDARD",
				"C_STANDARD_REQUIRED",
				"DEBUG_CONFIGURATIONS",
				"DEBUG_POSTFIX",
				"DEFINE_SYMBOL",
				"DEFINITIONS",
				"DEPENDS",
				"DEPLOYMENT_ADDITIONAL_FILES",
				"DEPLOYMENT_REMOTE_DIRECTORY",
				"DISABLED",
				"DISABLED_FEATURES",
				"DISABLE_PRECOMPILE_HEADERS",
				"DOTNET_TARGET_FRAMEWORK_VERSION",
				"ECLIPSE_EXTRA_CPROJECT_CONTENTS",
				"ECLIPSE_EXTRA_NATURES",
				"ENABLED_FEATURES",
				"ENABLED_LANGUAGES",
				"ENABLE_EXPORTS",
				"ENVIRONMENT",
				"EXCLUDE_FROM_ALL",
				"EXCLUDE_FROM_ALL",
				"EXCLUDE_FROM_DEFAULT_BUILD",
				"EXCLUDE_FROM_DEFAULT_BUILD_<CONFIG>",
				"EXPORT_NAME",
				"EXPORT_PROPERTIES",
				"EXTERNAL_OBJECT",
				"EchoString",
				"FAIL_REGULAR_EXPRESSION",
				"FIND_LIBRARY_USE_LIB32_PATHS",
				"FIND_LIBRARY_USE_LIB64_PATHS",
				"FIND_LIBRARY_USE_LIBX32_PATHS",
				"FIND_LIBRARY_USE_OPENBSD_VERSIONING",
				"FIXTURES_CLEANUP",
				"FIXTURES_REQUIRED",
				"FIXTURES_SETUP",
				"FOLDER",
				"FRAMEWORK",
				"FRAMEWORK_VERSION",
				"Fortran_FORMAT",
				"Fortran_FORMAT",
				"Fortran_MODULE_DIRECTORY",
				"GENERATED",
				"GENERATOR_FILE_NAME",
				"GENERATOR_IS_MULTI_CONFIG",
				"GHS_INTEGRITY_APP",
				"GHS_NO_SOURCE_GROUP_FILE",
				"GLOBAL_DEPENDS_DEBUG_MODE",
				"GLOBAL_DEPENDS_NO_CYCLES",
				"GNUtoMS",
				"HAS_CXX",
				"HEADER_FILE_ONLY",
				"HELPSTRING",
				"IMPLICIT_DEPENDS_INCLUDE_TRANSFORM",
				"IMPLICIT_DEPENDS_INCLUDE_TRANSFORM",
				"IMPORTED",
				"IMPORTED_COMMON_LANGUAGE_RUNTIME",
				"IMPORTED_CONFIGURATIONS",
				"IMPORTED_GLOBAL",
				"IMPORTED_IMPLIB",
				//"IMPORTED_IMPLIB_<CONFIG>",
				"IMPORTED_LIBNAME",
				//"IMPORTED_LIBNAME_<CONFIG>",
				"IMPORTED_LINK_DEPENDENT_LIBRARIES",
				//"IMPORTED_LINK_DEPENDENT_LIBRARIES_<CONFIG>",
				"IMPORTED_LINK_INTERFACE_LANGUAGES",
				//"IMPORTED_LINK_INTERFACE_LANGUAGES_<CONFIG>",
				"IMPORTED_LINK_INTERFACE_LIBRARIES",
				//"IMPORTED_LINK_INTERFACE_LIBRARIES_<CONFIG>",
				"IMPORTED_LINK_INTERFACE_MULTIPLICITY",
				//"IMPORTED_LINK_INTERFACE_MULTIPLICITY_<CONFIG>",
				"IMPORTED_LOCATION",
				//"IMPORTED_LOCATION_<CONFIG>",
				"IMPORTED_NO_SONAME",
				//"IMPORTED_NO_SONAME_<CONFIG>",
				"IMPORTED_OBJECTS",
				//"IMPORTED_OBJECTS_<CONFIG>",
				"IMPORTED_SONAME",
				//"IMPORTED_SONAME_<CONFIG>",
				"IMPORT_PREFIX",
				"IMPORT_SUFFIX",
				"INCLUDE_DIRECTORIES",
				"INCLUDE_DIRECTORIES",
				"INCLUDE_DIRECTORIES",
				"INCLUDE_REGULAR_EXPRESSION",
				"INSTALL_NAME_DIR",
				"INSTALL_REMOVE_ENVIRONMENT_RPATH",
				"INSTALL_RPATH",
				"INSTALL_RPATH_USE_LINK_PATH",
				"INTERFACE_AUTOUIC_OPTIONS",
				"INTERFACE_COMPILE_DEFINITIONS",
				"INTERFACE_COMPILE_FEATURES",
				"INTERFACE_COMPILE_OPTIONS",
				"INTERFACE_INCLUDE_DIRECTORIES",
				"INTERFACE_LINK_DEPENDS",
				"INTERFACE_LINK_DIRECTORIES",
				"INTERFACE_LINK_LIBRARIES",
				"INTERFACE_LINK_OPTIONS",
				"INTERFACE_POSITION_INDEPENDENT_CODE",
				"INTERFACE_PRECOMPILE_HEADERS",
				"INTERFACE_SOURCES",
				"INTERFACE_SYSTEM_INCLUDE_DIRECTORIES",
				"INTERPROCEDURAL_OPTIMIZATION",
				"INTERPROCEDURAL_OPTIMIZATION",
				//"INTERPROCEDURAL_OPTIMIZATION_<CONFIG>",
				//"INTERPROCEDURAL_OPTIMIZATION_<CONFIG>",
				"IN_TRY_COMPILE",
				"IOS_INSTALL_COMBINED",
				"JOB_POOLS",
				"JOB_POOL_COMPILE",
				"JOB_POOL_LINK",
				"KEEP_EXTENSION",
				"LABELS",
				"LABELS",
				"LABELS",
				"LABELS",
				"LANGUAGE",
				"LIBRARY_OUTPUT_DIRECTORY",
				//"LIBRARY_OUTPUT_DIRECTORY_<CONFIG>",
				"LIBRARY_OUTPUT_NAME",
				//"LIBRARY_OUTPUT_NAME_<CONFIG>",
				"LINKER_LANGUAGE",
				"LINK_DEPENDS",
				"LINK_DEPENDS_NO_SHARED",
				"LINK_DIRECTORIES",
				"LINK_DIRECTORIES",
				"LINK_FLAGS",
				"LINK_FLAGS_<CONFIG>",
				"LINK_INTERFACE_LIBRARIES",
				//"LINK_INTERFACE_LIBRARIES_<CONFIG>",
				"LINK_INTERFACE_MULTIPLICITY",
				//"LINK_INTERFACE_MULTIPLICITY_<CONFIG>",
				"LINK_LIBRARIES",
				"LINK_OPTIONS",
				"LINK_OPTIONS",
				"LINK_SEARCH_END_STATIC",
				"LINK_SEARCH_START_STATIC",
				"LINK_WHAT_YOU_USE",
				"LISTFILE_STACK",
				"LOCATION",
				"LOCATION",
				//"LOCATION_<CONFIG>",
				"MACOSX_BUNDLE",
				"MACOSX_BUNDLE_INFO_PLIST",
				"MACOSX_FRAMEWORK_INFO_PLIST",
				"MACOSX_PACKAGE_LOCATION",
				"MACOSX_RPATH",
				"MACROS",
				"MANUALLY_ADDED_DEPENDENCIES",
				//"MAP_IMPORTED_CONFIG_<CONFIG>",
				"MEASUREMENT",
				"MODIFIED",
				"MSVC_RUNTIME_LIBRARY",
				"NAME",
				"NO_SONAME",
				"NO_SYSTEM_FROM_IMPORTED",
				"OBJCXX_EXTENSIONS",
				"OBJCXX_STANDARD",
				"OBJCXX_STANDARD_REQUIRED",
				"OBJC_EXTENSIONS",
				"OBJC_STANDARD",
				"OBJC_STANDARD_REQUIRED",
				"OBJECT_DEPENDS",
				"OBJECT_OUTPUTS",
				"OSX_ARCHITECTURES",
				//"OSX_ARCHITECTURES_<CONFIG>",
				"OUTPUT_NAME",
				//"OUTPUT_NAME_<CONFIG>",
				"PACKAGES_FOUND",
				"PACKAGES_NOT_FOUND",
				"PARENT_DIRECTORY",
				"PASS_REGULAR_EXPRESSION",
				"PDB_NAME",
				//"PDB_NAME_<CONFIG>",
				"PDB_OUTPUT_DIRECTORY",
				//"PDB_OUTPUT_DIRECTORY_<CONFIG>",
				"POSITION_INDEPENDENT_CODE",
				"POST_INSTALL_SCRIPT",
				"PRECOMPILE_HEADERS",
				"PRECOMPILE_HEADERS_REUSE_FROM",
				"PREDEFINED_TARGETS_FOLDER",
				"PREFIX",
				"PRE_INSTALL_SCRIPT",
				"PRIVATE_HEADER",
				"PROCESSORS",
				"PROCESSOR_AFFINITY",
				"PROJECT_LABEL",
				"PUBLIC_HEADER",
				"REPORT_UNDEFINED_PROPERTIES",
				"REQUIRED_FILES",
				"RESOURCE",
				"RESOURCE_GROUPS",
				"RESOURCE_LOCK",
				"RULE_LAUNCH_COMPILE",
				"RULE_LAUNCH_COMPILE",
				"RULE_LAUNCH_COMPILE",
				"RULE_LAUNCH_CUSTOM",
				"RULE_LAUNCH_CUSTOM",
				"RULE_LAUNCH_CUSTOM",
				"RULE_LAUNCH_LINK",
				"RULE_LAUNCH_LINK",
				"RULE_LAUNCH_LINK",
				"RULE_MESSAGES",
				"RUNTIME_OUTPUT_DIRECTORY",
				//"RUNTIME_OUTPUT_DIRECTORY_<CONFIG>",
				"RUNTIME_OUTPUT_NAME",
				//"RUNTIME_OUTPUT_NAME_<CONFIG>",
				"RUN_SERIAL",
				"SKIP_AUTOGEN",
				"SKIP_AUTOMOC",
				"SKIP_AUTORCC",
				"SKIP_AUTOUIC",
				"SKIP_BUILD_RPATH",
				"SKIP_PRECOMPILE_HEADERS",
				"SKIP_REGULAR_EXPRESSION",
				"SKIP_RETURN_CODE",
				"SKIP_UNITY_BUILD_INCLUSION",
				"SOURCES",
				"SOURCE_DIR",
				"SOURCE_DIR",
				"SOVERSION",
				"STATIC_LIBRARY_FLAGS",
				//"STATIC_LIBRARY_FLAGS_<CONFIG>",
				"STATIC_LIBRARY_OPTIONS",
				"STRINGS",
				"SUBDIRECTORIES",
				"SUFFIX",
				"SYMBOLIC",
				"Swift_DEPENDENCIES_FILE",
				"Swift_DEPENDENCIES_FILE",
				"Swift_DIAGNOSTICS_FILE",
				"Swift_LANGUAGE_VERSION",
				"Swift_MODULE_DIRECTORY",
				"Swift_MODULE_NAME",
				"TARGET_ARCHIVES_MAY_BE_SHARED_LIBS",
				"TARGET_MESSAGES",
				"TARGET_SUPPORTS_SHARED_LIBS",
				"TESTS",
				"TEST_INCLUDE_FILE",
				"TEST_INCLUDE_FILES",
				"TIMEOUT",
				"TIMEOUT_AFTER_MATCH",
				"TYPE",
				"TYPE",
				"UNITY_BUILD",
				"UNITY_BUILD_BATCH_SIZE",
				"UNITY_BUILD_CODE_AFTER_INCLUDE",
				"UNITY_BUILD_CODE_BEFORE_INCLUDE",
				"USE_FOLDERS",
				"VALUE",
				"VARIABLES",
				"VERSION",
				"VISIBILITY_INLINES_HIDDEN",
				"VS_CONFIGURATION_TYPE",
				"VS_COPY_TO_OUT_DIR",
				//"VS_CSHARP_<tagname>",
				"VS_DEBUGGER_COMMAND",
				"VS_DEBUGGER_COMMAND_ARGUMENTS",
				"VS_DEBUGGER_ENVIRONMENT",
				"VS_DEBUGGER_WORKING_DIRECTORY",
				"VS_DEPLOYMENT_CONTENT",
				"VS_DEPLOYMENT_LOCATION",
				"VS_DESKTOP_EXTENSIONS_VERSION",
				//"VS_DOTNET_REFERENCEPROP_<refname>_TAG_<tagname>",
				"VS_DOTNET_REFERENCES",
				"VS_DOTNET_REFERENCES_COPY_LOCAL",
				//"VS_DOTNET_REFERENCE_<refname>",
				"VS_DOTNET_TARGET_FRAMEWORK_VERSION",
				"VS_DPI_AWARE",
				//"VS_GLOBAL_<variable>",
				"VS_GLOBAL_KEYWORD",
				"VS_GLOBAL_PROJECT_TYPES",
				"VS_GLOBAL_ROOTNAMESPACE",
				//"VS_GLOBAL_SECTION_POST_<section>",
				//"VS_GLOBAL_SECTION_PRE_<section>",
				"VS_INCLUDE_IN_VSIX",
				"VS_IOT_EXTENSIONS_VERSION",
				"VS_IOT_STARTUP_TASK",
				"VS_JUST_MY_CODE_DEBUGGING",
				"VS_KEYWORD",
				"VS_MOBILE_EXTENSIONS_VERSION",
				"VS_NO_SOLUTION_DEPLOY",
				"VS_PACKAGE_REFERENCES",
				"VS_PROJECT_IMPORT",
				"VS_RESOURCE_GENERATOR",
				"VS_SCC_AUXPATH",
				"VS_SCC_LOCALPATH",
				"VS_SCC_PROJECTNAME",
				"VS_SCC_PROVIDER",
				"VS_SDK_REFERENCES",
				"VS_SHADER_DISABLE_OPTIMIZATIONS",
				"VS_SHADER_ENABLE_DEBUG",
				"VS_SHADER_ENTRYPOINT",
				"VS_SHADER_FLAGS",
				"VS_SHADER_MODEL",
				"VS_SHADER_OBJECT_FILE_NAME",
				"VS_SHADER_OUTPUT_HEADER_FILE",
				"VS_SHADER_TYPE",
				"VS_SHADER_VARIABLE_NAME",
				"VS_STARTUP_PROJECT",
				"VS_TOOL_OVERRIDE",
				"VS_USER_PROPS",
				"VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION",
				"VS_WINRT_COMPONENT",
				"VS_WINRT_EXTENSIONS",
				"VS_WINRT_REFERENCES",
				"VS_XAML_TYPE",
				"WILL_FAIL",
				"WIN32_EXECUTABLE",
				"WINDOWS_EXPORT_ALL_SYMBOLS",
				"WORKING_DIRECTORY",
				"WRAP_EXCLUDE",
				//"XCODE_ATTRIBUTE_<an-attribute>",
				"XCODE_EMIT_EFFECTIVE_PLATFORM_NAME",
				"XCODE_EXPLICIT_FILE_TYPE",
				"XCODE_EXPLICIT_FILE_TYPE",
				"XCODE_FILE_ATTRIBUTES",
				"XCODE_GENERATE_SCHEME",
				"XCODE_LAST_KNOWN_FILE_TYPE",
				"XCODE_PRODUCT_TYPE",
				"XCODE_SCHEME_ADDRESS_SANITIZER",
				"XCODE_SCHEME_ADDRESS_SANITIZER_USE_AFTER_RETURN",
				"XCODE_SCHEME_ARGUMENTS",
				"XCODE_SCHEME_DEBUG_AS_ROOT",
				"XCODE_SCHEME_DEBUG_DOCUMENT_VERSIONING",
				"XCODE_SCHEME_DISABLE_MAIN_THREAD_CHECKER",
				"XCODE_SCHEME_DYNAMIC_LIBRARY_LOADS",
				"XCODE_SCHEME_DYNAMIC_LINKER_API_USAGE",
				"XCODE_SCHEME_ENVIRONMENT",
				"XCODE_SCHEME_EXECUTABLE",
				"XCODE_SCHEME_GUARD_MALLOC",
				"XCODE_SCHEME_MAIN_THREAD_CHECKER_STOP",
				"XCODE_SCHEME_MALLOC_GUARD_EDGES",
				"XCODE_SCHEME_MALLOC_SCRIBBLE",
				"XCODE_SCHEME_MALLOC_STACK",
				"XCODE_SCHEME_THREAD_SANITIZER",
				"XCODE_SCHEME_THREAD_SANITIZER_STOP",
				"XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER",
				"XCODE_SCHEME_UNDEFINED_BEHAVIOUR_SANITIZER_STOP",
				"XCODE_SCHEME_ZOMBIE_OBJECTS",
				"XCTEST"
			});

		//	parse
		p.read(start, end);
	}

}
