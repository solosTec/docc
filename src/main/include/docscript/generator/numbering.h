/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2019 Sylko Olzscher 
 * 
 */ 

#ifndef DOCSCRIPT_GENERATOR_NUMBERING_H
#define DOCSCRIPT_GENERATOR_NUMBERING_H

#include <cyng/object.h>
#include <cyng/intrinsics/sets.h>

#include <memory>
#include <string>
//#include <vector>
#include <boost/uuid/uuid.hpp>

namespace docscript
{
	/**
	 * content table as hierarchical structure.
	 * Each node has an optional sibbling (next_) and
	 * one child (sub_) optionally.
	 */
	class numbering
	{
		friend cyng::vector_t serialize(numbering const&);
		friend cyng::vector_t serialize(std::size_t depth, numbering const* p);

	public:
		numbering();
		numbering(boost::uuids::uuid, std::string);
		numbering(numbering&&) = default;
		numbering& operator=(numbering&&) = default;
		virtual ~numbering();

		std::string add(std::size_t level, boost::uuids::uuid, std::string);

	private:
		void add(std::vector<std::size_t>&, std::size_t level, numbering*);
		void add_next(numbering*);
		void add_sub(numbering*);

	private:
		boost::uuids::uuid const tag_;
		std::string const title_;

		/**
		 * @brief next_ We have to work with pointers since
		 * std::optional<> requires complete types and no recursive
		 * declarations are possible.
		 */
		std::unique_ptr<numbering> next_;
		std::unique_ptr<numbering> sub_;
	};

	std::string get_numbering(std::vector<std::size_t> const&);

	/**
	 * Convert a tree of numbering classes into a string
	 * like 3.2.5 ...
	 */
	cyng::vector_t serialize(numbering const&);
	cyng::vector_t serialize(std::size_t, numbering const* n);

	/**
	 * Simple list of footnotes
	 */
	class footnote
	{
	public:
		footnote(boost::uuids::uuid, std::string);

		std::string get_tag() const;
		std::string const& get_note() const;
	private:
		boost::uuids::uuid const tag_;
		std::string const note_;
	};

	using footnotes_t = std::list<footnote>;

	/**
	 * Simple list of figures
	 */
	class figure
	{
	public:
		figure(boost::uuids::uuid, std::string);

		std::string get_tag() const;
		std::string const& get_title() const;
	private:
		boost::uuids::uuid const tag_;
		std::string const title_;
	};

	using figures_t = std::list<footnote>;

}

#endif
