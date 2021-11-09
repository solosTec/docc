#include <asm/ast/jump.h>
#include <asm/context.h>

#include <cyng/obj/factory.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

namespace docasm {
	namespace ast {


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


	}
}
