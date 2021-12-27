
#include <footer.h>

#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/algorithm/reader.hpp>

namespace docscript {
	footer::footer(std::string color_scheme, std::string content, bool enabled)
		: color_scheme_(color_scheme)
		, content_(content)
		, enabled_(enabled)
	{}

	std::map<std::string, footer> read_footers(cyng::vector_t vec) {
		std::map<std::string, footer> r;
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(cyng::container_cast<cyng::param_map_t>(obj));
			r.emplace(std::piecewise_construct
				, std::forward_as_tuple(reader.get("name", ""))
				, std::forward_as_tuple(reader.get("color-scheme", "")
					, reader.get("content", "")
					, reader.get("enabled", false)));
		}
		return r;
	}

}
