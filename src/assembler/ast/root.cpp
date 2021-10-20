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
		//	-- label
		//	----------------------------------------------------------*
		//
		label label::factory(std::string const& name) {
			return label{ name };
		}

		std::ostream& operator<<(std::ostream& os, label const& c) {
			os << c.name_ << ':';
			return os;
		}

		std::size_t label::size() const {
			return 0;
		}

		void label::generate(context&, label_list_t const&) const {

		}


		//
		//	----------------------------------------------------------*
		//	-- literal
		//	----------------------------------------------------------*
		//
		literal literal::factory(std::string const& name) {
			return literal{ name };
		}

		std::ostream& operator<<(std::ostream& os, literal const& c) {
			os << c.value_;
			return os;
		}

		std::size_t literal::size() const {
			return 1;
		}
		void literal::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(value_));
		}

		//
		//	----------------------------------------------------------*
		//	-- operation
		//	----------------------------------------------------------*
		//
		operation operation::factory(cyng::op code) {
			return operation{ code };
		}

		std::ostream& operator<<(std::ostream& os, operation const& c) {
			os << c.code_;
			return os;
		}

		std::size_t operation::size() const {
			return 1;
		}
		void operation::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(code_));
		}

		//
		//	----------------------------------------------------------*
		//	-- value
		//	----------------------------------------------------------*
		//
		value value::factory(symbol const& sym) {
			switch (sym.type_) {
			case symbol_type::TST:
				return { cyng::to_timestamp(sym.value_) };
			case symbol_type::COL:
				return { cyng::color_8() };
			case symbol_type::BOL:
				return { boost::algorithm::equals(sym.value_, "true") };
			case symbol_type::NUM:
				try {
					return { static_cast<std::uint64_t>(std::stoul(sym.value_, nullptr, 10)) };
				}
				catch (...) {
					//fmt::print(
					//	stdout,
					//	fg(fmt::color::orange) | fmt::emphasis::bold,
					//	"{}: warning: invalid number [{}]\n", ctx_.get_position(), sym.value_);
					return { static_cast<std::uint64_t>(0) };
				}
				break;
			default:
				break;
			}
			return { sym.value_ };
		}

		value value::factory(boost::uuids::uuid oid) {
			return { oid };
		}

		std::ostream& operator<<(std::ostream& os, value const& v) {
			std::visit(overloaded{
				[](auto& arg) {},
				[&](std::string const& val) {
					os << std::quoted(val);
					},
				[&](std::chrono::system_clock::time_point const& val) {
					const std::time_t t_c = std::chrono::system_clock::to_time_t(std::get<2>(v.val_));
					os << std::put_time(std::localtime(&t_c), "%FT%T\n");
					},
				[&](cyng::color_8 const& val) {
					os << val;
					},
				[&](bool val) {
					os << (val ? "true" : "false");
					},
				[&](std::uint64_t val) {
					os << std::dec << val << "u64";
					},
				[&](boost::uuids::uuid val) {
					os << val;
					},
				[&](double val) {
					os << std::fixed << val;
					}
				}, v.val_);

			return os;
		}

		std::size_t value::size() const {
			return 1;
		}
		void value::generate(context& ctx, label_list_t const&) const {
			std::visit([&](auto&& arg) {
				ctx.emit(cyng::make_object(arg));
				}, val_);
		}


		//
		//	----------------------------------------------------------*
		//	-- push
		//	----------------------------------------------------------*
		//
		push push::factory() {
			return push{ std::monostate()};
		}

		std::ostream& operator<<(std::ostream& os, push const& c) {
			os << "push " << c.val_;
			return os;
		}

		push push::finish(value&& val) {
			return { val };
		}

		std::size_t push::size() const {
			return 2;
		}
		void push::generate(context& ctx, label_list_t const& ll) const {
			//ctx.emit(cyng::make_object(cyng::op::PUSH));	//	push is implicit
			val_.generate(ctx, ll);
		}

		//
		//	----------------------------------------------------------*
		//	-- invoke
		//	----------------------------------------------------------*
		//
		invoke invoke::factory() {
			return invoke{ std::string() };
		}

		std::ostream& operator<<(std::ostream& os, invoke const& c) {
			os << "invoke " << c.name_;
			return os;
		}

		invoke invoke::finish(std::string const& name) {
			return { name };
		}

		std::size_t invoke::size() const {
			return 2;
		}
		void invoke::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(name_));
			ctx.emit(cyng::make_object(cyng::op::INVOKE));
		}

		//
		//	----------------------------------------------------------*
		//	-- invoke_r
		//	----------------------------------------------------------*
		//
		invoke_r invoke_r::factory() {
			return invoke_r{ std::string() };
		}

		std::ostream& operator<<(std::ostream& os, invoke_r const& c) {
			os << "invoke_r " << c.name_;
			return os;
		}

		invoke_r invoke_r::finish(std::string const& name) {
			return { name };
		}

		std::size_t invoke_r::size() const {
			return 3;
		}
		void invoke_r::generate(context& ctx, label_list_t const&) const {
			ctx.emit(cyng::make_object(name_));
			ctx.emit(cyng::make_object(cyng::op::RESOLVE));
			ctx.emit(cyng::make_object(cyng::op::INVOKE_R));
		}

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
