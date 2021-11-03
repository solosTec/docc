/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Sylko Olzscher
 *
 */
#ifndef DOCC_RUNTIME_NUMBERING_H
#define DOCC_RUNTIME_NUMBERING_H

#include <cyng/obj/object.h>
#include <cyng/obj/intrinsics/container.h>

#include <memory>
#include <string>

#include <boost/uuid/uuid.hpp>

namespace docruntime
{
	/**
	 * content table as hierarchical structure.
	 * Each node has an optional sibbling (next_) and
	 * one child (sub_) optionally.
	 */
	class numbering
	{
		friend cyng::vector_t serialize(numbering const&);
		friend cyng::vector_t serialize(std::size_t depth, numbering const* p, std::vector<std::size_t>);

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
		std::unique_ptr<numbering> next_;	//	sibling
		std::unique_ptr<numbering> sub_;	//	child
	};

	/**
	 * Takes a vector with hierarchical information
	 * and generates the numbering.
	 */
	std::string get_numbering(std::vector<std::size_t> const&);

	/**
	 * Convert a tree of numbering classes into a string
	 * like 3.2.5 ...
	 */
	cyng::vector_t serialize(numbering const&);
	cyng::vector_t serialize(std::size_t, numbering const* n, std::vector<std::size_t>);

	/**
	 * element like table, figure, footnote with an UUID
	 * and text
	 */
	class element
	{
	public:
		element(boost::uuids::uuid, std::string);

		std::string get_tag() const;
		std::string const& get_text() const;
	private:
		boost::uuids::uuid const tag_;
		std::string const text_;
	};

	/**
	 * Simple list of footnotes
	 */
	using footnotes_t = std::list<element>;

	/**
	 * Simple list of figures
	 */
	using figures_t = std::list<element>;

	/**
	 * Simple list of tables
	 */
	using tables_t = std::list<element>;

}

#endif
