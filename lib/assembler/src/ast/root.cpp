#include <ast/root.h>
#include <context.h>

#include <cyng/io/ostream.h>
#include <cyng/parse/timestamp.h>
#include <cyng/obj/factory.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

#include <utility>

#include <boost/assert.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace docscript {
	namespace ast {

		template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
		// explicit deduction guide (not needed as of C++20)
		template<class... Ts> overloaded(Ts...)->overloaded<Ts...>;



		//
		//	----------------------------------------------------------*
		//	-- forward
		//	----------------------------------------------------------*
		//
		forward forward::factory() {
			return forward{ boost::uuids::uuid()};
		}

		std::ostream& operator<<(std::ostream& os, forward const& c) {
			os << "forward " << c.tag_;
			return os;
		}

		forward forward::finish(boost::uuids::uuid tag) {
			return forward{ tag };
		}

		std::size_t forward::size() const {
			return 2;
		}
		void forward::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(cyng::op::FORWARD));
			ctx.emit(cyng::make_object(tag_));
		}

		//
		//	----------------------------------------------------------*
		//	-- jump
		//	----------------------------------------------------------*
		//

		jump jump::factory(cyng::op code) {
			return { code, std::string()};
		}

		std::ostream& operator<<(std::ostream& os, jump const& c) {
			os << "jump " << c.label_;
			return os;
		}

		/**
		 * Generate a complete forward operation
		 */
		jump jump::finish(std::string const& name) {
			return { this->code_, name };
		}

		std::size_t jump::size() const {
			return 2;
		}
		void jump::generate(context& ctx, label_list_t const& ll) const {
			ctx.emit(cyng::make_object(code_));
			//
			//	get address
			//
			auto const pos = ll.find(label_);
			if (pos != ll.end()) {
				ctx.emit(cyng::make_object(pos->second));
				fmt::print(
					stdout,
					fg(fmt::color::dim_gray),
					"label \"{}\" at @{}\n", label_, pos->second);
			}
			else {
				fmt::print(
					stdout,
					fg(fmt::color::crimson) | fmt::emphasis::bold,
					"{}: error: label \"{}\" not found\n", ctx.get_position(), label_);
				ctx.emit(cyng::make_object(0ul));
			}
		}

		//
		//	----------------------------------------------------------*
		//	-- program
		//	----------------------------------------------------------*
		//
		program::program(context& ctx)
			: ctx_(ctx)
			, semantic_stack_()
			, asts_()
			, uuidgen_()
		{}

		void program::finalize_statement() {

			if (semantic_stack_.size() == 1) {
				asts_.push_back(semantic_stack_.top());
				semantic_stack_.pop();
			}
			else if (!semantic_stack_.empty()) {
				switch (semantic_stack_.top().index()) {
				case 8: {	//	value
					BOOST_ASSERT(semantic_stack_.size() > 1);
					auto const val = std::get<8>(semantic_stack_.top());
					semantic_stack_.pop();
					switch (semantic_stack_.top().index()) {
					case 5: {	//	forward expects uuid
						auto f = std::get<5>(semantic_stack_.top()).finish(std::get<boost::uuids::uuid>(val.val_));
						asts_.push_back(f);
						semantic_stack_.pop();
					}
						  break;
					default:
						break;
					}
				}
					break;
				default:
					break;
				}
			}
		}

		void program::finalize_statement(symbol const& sym) {
			//label,		//	[0]
			//operation,	//	[1]
			//push,		//	[2]
			//invoke,	//	[3]
			//invoke_r,	//	[4]
			//forward,	//	[5]
			//jump		//	[6]
			//literal,	//	[7]
			//value		//	[8]

			std::visit(overloaded{ 
				[](auto &arg) {},
				[&](push& top) {
					auto p = top.finish(value::factory(sym));
					semantic_stack_.pop();
					semantic_stack_.push(p);
				},
				[&](invoke& top) {
					auto p = top.finish(sym.value_);
					semantic_stack_.pop();
					semantic_stack_.push(p);
				},
				[&](invoke_r& top) {
					auto p = top.finish(sym.value_);
					semantic_stack_.pop();
					semantic_stack_.push(p);
				},
				[&](forward& ) {
				},
				[&](jump& top) {
					auto j = top.finish(sym.value_);
					semantic_stack_.pop();
					semantic_stack_.push(j);
				},
				[&](literal& top) {
					semantic_stack_.pop();
					BOOST_ASSERT(sym.type_ == symbol_type::TYP);
					if (boost::algorithm::equals(sym.value_, "uuid")) {
						semantic_stack_.push(value::factory(uuidgen_(top.value_)));
					}
					else {
						fmt::print(
							stdout,
							fg(fmt::color::crimson) | fmt::emphasis::bold,
							"{}: error: unknown data type [{}]\n", ctx_.get_position(), sym.value_);
					}
				}
			}
			, semantic_stack_.top());

			//switch (semantic_stack_.top().index()) {
			//case 2: {	//	push
			//	auto p = std::get<2>(semantic_stack_.top()).finish(value::factory(sym));
			//	semantic_stack_.pop();
			//	semantic_stack_.push(p);
			//}
			//	break;
			//case 3: {	//	invoke
			//	auto p = std::get<3>(semantic_stack_.top()).finish(sym.value_);
			//	semantic_stack_.pop();
			//	semantic_stack_.push(p);
			//}
			//	  break;
			//case 4: {	//	invoke_r
			//	auto p = std::get<3>(semantic_stack_.top()).finish(sym.value_);
			//	semantic_stack_.pop();
			//	semantic_stack_.push(p);
			//}
			//	  break;
			//case 5: {	//	forward
			//	//auto p = std::get<4>(semantic_stack_.top()).finish(value::factory(sym));

			//}
			//	  break;
			//case 6: {	//	jump
			//	auto j = std::get<6>(semantic_stack_.top()).finish(sym.value_);
			//	semantic_stack_.pop();
			//	semantic_stack_.push(j);
			//}
			//	  break;
			//case 7: {	//	literal
			//	auto const lit = std::get<7>(semantic_stack_.top());
			//	semantic_stack_.pop();
			//	BOOST_ASSERT(sym.type_ == symbol_type::TYP);
			//	if (boost::algorithm::equals(sym.value_, "uuid")) {
			//		semantic_stack_.push(value::factory(uuidgen_(lit.value_)));
			//	}
			//	else {
			//		fmt::print(
			//			stdout,
			//			fg(fmt::color::crimson) | fmt::emphasis::bold,
			//			"{}: error: unknown data type [{}]\n", ctx_.get_position(), sym.value_);
			//	}
			//}
			//	  break;
			//default:
			//	break;
			//}
		}

		void program::init_label(std::string const& name) {
			BOOST_ASSERT(semantic_stack_.empty());

			semantic_stack_.push(label::factory(name));
		}

		void program::init_op(cyng::op code) {
			BOOST_ASSERT(semantic_stack_.empty());

			switch (code) {
			case cyng::op::PUSH://!<  	push, mem[--sp] = mem[pc]
				semantic_stack_.push(push::factory());
				break;

			case cyng::op::INVOKE:		//!< 	call a library function
				semantic_stack_.push(invoke::factory());
				break;
			case cyng::op::INVOKE_R:			//!< 	call a library function
				semantic_stack_.push(invoke_r::factory());
				break;
			case cyng::op::FORWARD:			//!<	forward function call to other VM
				semantic_stack_.push(forward::factory());
				break;

			case cyng::op::JA:
			case cyng::op::JE:	//!< 	jump if error register is set
			case cyng::op::JNE:	//!< 	jump if error register is not set
				semantic_stack_.push(jump::factory(code));
				break;

			default:
				semantic_stack_.push(operation::factory(code));
				break;
			}
		}

		void program::init_literal(std::string const& value) {
			semantic_stack_.push(literal::factory(value));
		}

		ast::label_list_t program::build_label_list() {
			ast::label_list_t ll;
			std::size_t pos{ 0 };
			for (auto const& s : asts_) {
				if (s.index() == 0) {
					auto const name = std::get<0>(s).name_;
					if (ll.find(name) != ll.end()) {
						fmt::print(
							stdout,
							fg(fmt::color::crimson) | fmt::emphasis::bold,
							"{}: error: duplicate label \"{}\"\n", ctx_.get_position(), name);
					}
					else {
						ll.emplace(name, pos);
						fmt::print(
							stdout,
							fg(fmt::color::dim_gray),
							"{}: label \"{}\" at [{}] \n", ctx_.get_position(), name, pos);
					}
				}
				else {
					std::visit([&](auto&& arg) {
						//std::cout << arg << std::endl;
						pos += arg.size();
						}, s);
				}
			}
			return ll;
		}
		void program::generate(ast::label_list_t const& ll) {

			for (auto const& s : asts_) {

				std::visit([&](auto&& arg) {
#ifdef _DEBUG
					std::cout << arg << std::endl;
#endif
					arg.generate(ctx_, ll);
					}, s);

			}
		}

	}
}
