
#include <toc.h>

#include <cyng/obj/factory.hpp>
#include <cyng/obj/container_factory.hpp>

#include <iostream>

#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace docruntime
{

	std::string get_numbering(std::vector<std::size_t> const& vec)
	{
		std::string r;
		for (auto number : vec) {
			if (!r.empty()) {
				r.append(1, '.');
			}
			r.append(std::to_string(number));
		}
		return r;
	}


	element::element(boost::uuids::uuid tag, std::string note)
		: tag_(tag)
		, text_(note)
	{}

	std::string element::get_tag() const
	{
		return boost::uuids::to_string(tag_);
	}

	std::string const& element::get_text() const
	{
		return text_;
	}

	toc::toc()
		: headings_()
	{}
	toc::toc(boost::uuids::uuid tag, std::string title)
		: headings_({ entry(tag, title) })
	{}

	bool toc::add(std::size_t level, boost::uuids::uuid tag, std::string title) {
		if (level == 0) {
			headings_.emplace_back(tag, title);
		}
		else {
			if (headings_.empty()) return false;

			if (!headings_.back().sub_) {
				BOOST_ASSERT_MSG(level == 1, "missing toc level");
				headings_.back().sub_ = std::make_shared<toc>(tag, title);
			}
			else {
				headings_.back().sub_->add(level - 1, tag, title);
			}
		}
		return true;
	}

	toc::entry::entry(boost::uuids::uuid tag, std::string s)
		: element(tag, s)
		, sub_()
	{}

	namespace {	//	static linkage

		std::vector<std::size_t> inc(std::vector<std::size_t> vec) {
			vec.push_back(1);
			return vec;
		}
	}

	cyng::vector_t to_vector(toc const& t) {
		std::vector<std::size_t> index{ 1 };
		return to_vector(t, index);
	}

	cyng::vector_t to_vector(toc const& t, std::vector<std::size_t> index) {
		cyng::vector_t vec;
		for (auto const& e : t.headings_) {

			if (e.sub_) {
				auto params = cyng::param_map_factory("tag", e.get_tag())
					("title", e.get_text())
					("number", get_numbering(index))
					("depth", index.size())
					("sub", to_vector(*e.sub_, inc(index)))
					();
				vec.push_back(params);
			}
			else {
				auto params = cyng::param_map_factory("tag", e.get_tag())
					("title", e.get_text())
					("number", get_numbering(index))
					("depth", index.size())
					();
				vec.push_back(params);
			}
			++index.back();
		}
		return vec;

	}


	std::ostream& operator<<(std::ostream& os, toc const& t) {
		std::vector<std::size_t> index{ 1 };
		serialize(os, t, index);
		return os;
	}

	void serialize(std::ostream& os, toc const& t, std::vector<std::size_t> index) {
		for (auto const& e : t.headings_) {
			os << get_numbering(index) << ' ' << e.get_text() << std::endl;
			if (e.sub_) {
				serialize(os, *e.sub_, inc(index));
			}
			++index.back();
		}
	}

}

