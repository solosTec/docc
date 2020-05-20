/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#include <docscript/generator/numbering.h>
#include <cyng/factory.h>
#include <iostream>

#include <boost/uuid/nil_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace docscript
{
	numbering::numbering()
		: tag_(boost::uuids::nil_uuid())
		, title_()
		, next_()
		, sub_()
	{}

	numbering::numbering(boost::uuids::uuid tag, std::string title)
		: tag_(tag)
		, title_(title)
		, next_()
		, sub_()
	{}

	numbering::~numbering()
	{}

	std::string numbering::add(std::size_t level, boost::uuids::uuid tag, std::string title)
	{
		std::vector<std::size_t> vec;
		vec.push_back(0u);
		add(vec, level, new numbering(tag, title));
		return get_numbering(vec);
	}

	void numbering::add(std::vector<std::size_t>& vec, std::size_t level, numbering* cp)
	{
		//
		//	root flag - fix the first (empty entry)
		//
		bool const root = vec.at(0) == 0u;

		//
		//	current numbering index
		//
		auto const idx = vec.size() - 1;

		//
		//	walk down vertically to current/last entry
		//
		numbering* ptr = this;
		while (ptr->next_) {
			ptr = ptr->next_.get();
			++vec.at(idx);
		}

		if (vec.size() < level) {


			if (!ptr->sub_) {
				//
				//	increase depth
				//
				vec.push_back(1u);
				if (vec.size() < level) {
					//
					//	cannot skip indentation level
					//
					std::cerr
						<< "***error cannot skip indentation level ["
						<< cp->title_
						<< ']'
						<< std::endl;
				}
				ptr->add_sub(cp);
			}
			else {
				//
				//	walk down horizontally to current/last entry
				//
				if (level - vec.size() > 1) {
					vec.push_back(1u);
				}
				else {
					vec.push_back(2u);
				}
				ptr->sub_->add(vec, level, cp);
			}
		}
		else {
			ptr->add_next(cp);
			if (root) {
				++vec.at(idx);
			}
		}
	}

	void numbering::add_next(numbering* c)
	{
		next_.reset(c);
	}
	void numbering::add_sub(numbering* c)
	{
		sub_.reset(c);
	}

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

	cyng::vector_t serialize(numbering const& n)
	{

		//
		//	skip first entry
		//
		if (n.next_) {

			return serialize(1, n.next_.get());

		}

		return cyng::vector_t();
	}

	cyng::vector_t serialize(std::size_t depth, numbering const* p)
	{
		cyng::vector_t vec;

		//std::cout << "serialize " << std::string(depth, '.' ) << ": " << p->title_ << std::endl;

		if (p->sub_) {
			auto params = cyng::param_map_factory("tag", p->tag_)("title", p->title_)("depth", depth)("sub", serialize(depth + 1, p->sub_.get()))();
			vec.push_back(params);
		}
		else {
			auto params = cyng::param_map_factory("tag", p->tag_)("title", p->title_)("depth", depth)();
			vec.push_back(params);
		}

		if (p->next_) {
			auto const v = serialize(depth, p->next_.get());
			vec.insert(vec.end(), v.begin(), v.end());
		}

		return vec;
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

	//footnote::footnote(boost::uuids::uuid tag, std::string note)
	//	: tag_(tag)
	//	, note_(note)
	//{}

	//std::string footnote::get_tag() const
	//{
	//	return boost::uuids::to_string(tag_);
	//}

	//std::string const& footnote::get_note() const
	//{
	//	return note_;
	//}

	//figure::figure(boost::uuids::uuid tag, std::string title)
	//	: tag_(tag)
	//	, title_(title)
	//{}

	//std::string figure::get_tag() const
	//{
	//	return boost::uuids::to_string(tag_);
	//}

	//std::string const& figure::get_title() const
	//{
	//	return title_;
	//}
}


