
#include <navbar.h>

#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/algorithm/reader.hpp>

namespace docscript {
	navbar::navbar(std::string placement, std::string color_scheme, std::filesystem::path brand)
		: placement_(placement)
		, color_scheme_(color_scheme)
		, brand_(brand)
	{}


	std::map<std::string, navbar> read_navbars(cyng::vector_t vec) {
		std::map<std::string, navbar> r;
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(cyng::container_cast<cyng::param_map_t>(obj));
			r.emplace(std::piecewise_construct
				, std::forward_as_tuple(reader.get("name", ""))
				, std::forward_as_tuple(reader.get("placement", "fixed-top")
					, reader.get("color-scheme", "light")
					, reader.get("brand", "")));
		}
		return r;
	}

}
