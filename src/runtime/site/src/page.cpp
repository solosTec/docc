
#include <page.h>

#include <cyng/obj/container_cast.hpp>
#include <cyng/obj/algorithm/reader.hpp>

namespace docscript {
	page::page(std::string title, std::string menu, std::string footer, bool enabled, std::filesystem::path source)
		: title_(title)
		, navbar_(menu)
		, footer_(footer)
		, enabled_(enabled)
		, source_()
	{}

	std::map<std::string, page> read_pages(cyng::vector_t vec) {
		std::map<std::string, page> r;
		for (auto const& obj : vec) {
			auto const reader = cyng::make_reader(cyng::container_cast<cyng::param_map_t>(obj));
			r.emplace(std::piecewise_construct
				, std::forward_as_tuple(reader.get("name", ""))
				, std::forward_as_tuple(reader.get("title", "")
					, reader.get("navbar", "")
					, reader.get("footer", "")
					, reader.get("enabled", false)
					, reader.get("source", "")));
		}
		return r;
	}

}
