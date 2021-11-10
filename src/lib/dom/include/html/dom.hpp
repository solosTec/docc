/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Sylko Olzscher
 *
 */

#ifndef HTML_DOM_H
#define HTML_DOM_H

#include <list>
#include <iostream>
#include <sstream>

#include <boost/flyweight.hpp>

namespace dom
{
	enum class node_types {
		ELEMENT = 1,
		ATTRIBUTE,
		TEXT,
		ENTITY = 6,
		COMMENT = 8,
		DOCUMENT
	};

	/**
	 * cleanup HTML text
	 */
	std::string patch_attribute_name(std::string value);

	/**
	 * base class
	 */
	class node 
	{
	public:
		node();

		virtual void serialize(std::ostream&) const = 0;
		virtual node_types get_node_type() const = 0;

		std::string	operator()(std::size_t intend) const;

	protected:
		virtual void serialize(std::ostream&, std::size_t depth) const = 0;
	};

	/**
	 * text node
	 */
	class element;
	class text : public node
	{
		friend class element;
	public:
		text();
		text(std::string);

		template<typename ...Args>
		text(Args... args)
		{
			std::size_t n{ 0 };
			std::stringstream ss;

			((ss 
				<< args 
				<< (++n != sizeof...(Args) ? " " : "")), ...);

			value_ = ss.str();
		}

		virtual void serialize(std::ostream&) const override;
		virtual node_types get_node_type() const override;

		/**
		 * @return true if text is not empty
		 */
		explicit operator bool() const;

	protected:
		virtual void serialize(std::ostream&, std::size_t depth) const override;

	private:
		std::string value_;
	};

	/**
	 * attribute node
	 */
	class attribute : public node
	{
		struct dom_attribute {};
	public:
		attribute();
		attribute(std::string);

		template<typename ...Args>
		attribute(std::string name, Args... args)
			: name_(patch_attribute_name(name))
			, value_()
		{
			std::size_t n{ 0 };
			std::stringstream ss;

			((ss
				<< args
				<< (++n != sizeof...(Args) ? " " : "")), ...);

			value_ = ss.str();
		}


		virtual node_types get_node_type() const override;
		virtual void serialize(std::ostream&) const;

	protected:
		virtual void serialize(std::ostream&, std::size_t depth) const override;

	private:
		boost::flyweight<std::string, boost::flyweights::tag<dom_attribute>> const name_;
		std::string value_;
	};


	/**
	 * element node
	 */
	class element : public node
	{
		struct dom_element {};
	public:
		element();
		element(std::string tag);
		element(std::string tag, text);

		template<typename ...Args>
		element(std::string tag, Args... args)
			: tag_(std::move(tag))
			, children_()
			, text_()
		{
			assign(std::forward<Args>(args)...);
		}

		virtual node_types get_node_type() const override;
		virtual void serialize(std::ostream&) const override;

		/**
		 * append attribute
		 */
		element& operator+=(attribute&&);

		/**
		 * append child
		 */
		element& operator+=(element&&);

	protected:
		virtual void serialize(std::ostream&, std::size_t depth) const override;

	private:
		template<class T, class ...Args>
		void assign(T&& val, Args&&...args)
		{
			assign_impl(std::forward<T>(val));
			assign(std::forward<Args>(args)...);
		}

		template<class T>
		void assign(T&& val)
		{
			assign_impl(std::forward<T>(val));
		}

		template<class T>
		void assign_impl(T&& val)
		{
			text_ = text(std::forward<T>(val));
		}

		void assign_impl(attribute&& attr);
		void assign_impl(element&& e);
		void assign_impl(text&& t);


	private:
		boost::flyweight<std::string, boost::flyweights::tag<dom_element>> const tag_;
		std::list<attribute> attr_;
		std::list<element>	children_;
		text text_;
	};



#define DEFINE_ELEMENT(X) template<class ...Args> \
	element X(Args &&...args)	\
	{return element(#X, std::forward<Args>(args)...);}

#define DEFINE_ATTRIBUTE(X) \
	template<class T> \
	attribute X##_(T &&val){return attribute(#X, std::forward<T>(val));}


	inline namespace elem
	{
		DEFINE_ELEMENT(a)
		DEFINE_ELEMENT(abbr)
		DEFINE_ELEMENT(acronym)
		DEFINE_ELEMENT(address)
		DEFINE_ELEMENT(applet)
		DEFINE_ELEMENT(area)
		DEFINE_ELEMENT(article)
		DEFINE_ELEMENT(aside)
		DEFINE_ELEMENT(audio)
		DEFINE_ELEMENT(autocomplete)
		DEFINE_ELEMENT(autofocus)
		DEFINE_ELEMENT(b)
		DEFINE_ELEMENT(base)
		DEFINE_ELEMENT(basefont)
		DEFINE_ELEMENT(bdi)
		DEFINE_ELEMENT(bdo)
		DEFINE_ELEMENT(big)
		DEFINE_ELEMENT(blockquote)
		DEFINE_ELEMENT(body)
		DEFINE_ELEMENT(br)
		DEFINE_ELEMENT(button)
		DEFINE_ELEMENT(canvas)
		DEFINE_ELEMENT(caption)
		DEFINE_ELEMENT(center)
		DEFINE_ELEMENT(cite)
		DEFINE_ELEMENT(code)
		DEFINE_ELEMENT(col)
		DEFINE_ELEMENT(colgroup)
		DEFINE_ELEMENT(command)
		DEFINE_ELEMENT(datalist)
		DEFINE_ELEMENT(dd)
		DEFINE_ELEMENT(del)
		DEFINE_ELEMENT(details)
		DEFINE_ELEMENT(dfn)
		DEFINE_ELEMENT(dialog)
		DEFINE_ELEMENT(dir)
		DEFINE_ELEMENT(div)
		DEFINE_ELEMENT(dl)
		DEFINE_ELEMENT(dt)
		DEFINE_ELEMENT(em)
		DEFINE_ELEMENT(embed)
		DEFINE_ELEMENT(fieldset)
		DEFINE_ELEMENT(figcaption)
		DEFINE_ELEMENT(figure)
		DEFINE_ELEMENT(font)
		DEFINE_ELEMENT(footer)
		DEFINE_ELEMENT(form)
		DEFINE_ELEMENT(frame)
		DEFINE_ELEMENT(frameset)
		DEFINE_ELEMENT(h1)
		DEFINE_ELEMENT(h2)
		DEFINE_ELEMENT(h3)
		DEFINE_ELEMENT(h4)
		DEFINE_ELEMENT(h5)
		DEFINE_ELEMENT(h6)
		DEFINE_ELEMENT(head)
		DEFINE_ELEMENT(header)
		DEFINE_ELEMENT(hr)
		DEFINE_ELEMENT(html)
		DEFINE_ELEMENT(i)
		DEFINE_ELEMENT(iframe)
		DEFINE_ELEMENT(img)
		DEFINE_ELEMENT(input)
		DEFINE_ELEMENT(ins)
		DEFINE_ELEMENT(isindex)
		DEFINE_ELEMENT(kbd)
		DEFINE_ELEMENT(keygen)
		DEFINE_ELEMENT(label)
		DEFINE_ELEMENT(legend)
		DEFINE_ELEMENT(li)
		DEFINE_ELEMENT(link)
		DEFINE_ELEMENT(map)
		DEFINE_ELEMENT(mark)
		DEFINE_ELEMENT(menu)
		DEFINE_ELEMENT(menuitem)
		DEFINE_ELEMENT(meta)
		DEFINE_ELEMENT(meter)
		DEFINE_ELEMENT(nav)
		DEFINE_ELEMENT(noframes)
		DEFINE_ELEMENT(noscript)
		DEFINE_ELEMENT(object)
		DEFINE_ELEMENT(ol)
		DEFINE_ELEMENT(optgroup)
		DEFINE_ELEMENT(option)
		DEFINE_ELEMENT(output)
		DEFINE_ELEMENT(p)
		DEFINE_ELEMENT(param)
		DEFINE_ELEMENT(pre)
		DEFINE_ELEMENT(progress)
		DEFINE_ELEMENT(q)
		DEFINE_ELEMENT(rp)
		DEFINE_ELEMENT(rt)
		DEFINE_ELEMENT(ruby)
		DEFINE_ELEMENT(s)
		DEFINE_ELEMENT(samp)
		DEFINE_ELEMENT(script)
		DEFINE_ELEMENT(section)
		DEFINE_ELEMENT(select)
		//DEFINE_ELEMENT(small)
		DEFINE_ELEMENT(source)
		DEFINE_ELEMENT(span)
		DEFINE_ELEMENT(strike)
		DEFINE_ELEMENT(strong)
		DEFINE_ELEMENT(style)
		DEFINE_ELEMENT(sub)
		DEFINE_ELEMENT(summary)
		DEFINE_ELEMENT(sup)
		DEFINE_ELEMENT(table)
		DEFINE_ELEMENT(tbody)
		DEFINE_ELEMENT(td)
		DEFINE_ELEMENT(textarea)
		DEFINE_ELEMENT(tfoot)
		DEFINE_ELEMENT(th)
		DEFINE_ELEMENT(thead)
		DEFINE_ELEMENT(time)
		DEFINE_ELEMENT(title)
		DEFINE_ELEMENT(tr)
		DEFINE_ELEMENT(track)
		DEFINE_ELEMENT(tt)
		DEFINE_ELEMENT(u)
		DEFINE_ELEMENT(ul)
		DEFINE_ELEMENT(var)
		DEFINE_ELEMENT(video)
		DEFINE_ELEMENT(wbr)
		DEFINE_ELEMENT(xmp)

	}

	inline namespace attr
	{
		DEFINE_ATTRIBUTE(abbr)
		DEFINE_ATTRIBUTE(accept)
		DEFINE_ATTRIBUTE(accept_charset)
		DEFINE_ATTRIBUTE(accesskey)
		DEFINE_ATTRIBUTE(action)
		DEFINE_ATTRIBUTE(align)
		DEFINE_ATTRIBUTE(alink)
		DEFINE_ATTRIBUTE(alt)
		DEFINE_ATTRIBUTE(archive)
		DEFINE_ATTRIBUTE(aria_hidden)
		DEFINE_ATTRIBUTE(aria_labelledby)
		DEFINE_ATTRIBUTE(async)
		DEFINE_ATTRIBUTE(autocomplete)
		DEFINE_ATTRIBUTE(autofocus)
		DEFINE_ATTRIBUTE(autoplay)
		DEFINE_ATTRIBUTE(axis)
		DEFINE_ATTRIBUTE(background)
		DEFINE_ATTRIBUTE(bgcolor)
		DEFINE_ATTRIBUTE(border)
		DEFINE_ATTRIBUTE(cellpadding)
		DEFINE_ATTRIBUTE(cellspacing)
		DEFINE_ATTRIBUTE(challenge)
		DEFINE_ATTRIBUTE(char)
		DEFINE_ATTRIBUTE(charoff)
		DEFINE_ATTRIBUTE(charset)
		DEFINE_ATTRIBUTE(checked)
		DEFINE_ATTRIBUTE(cite)
		DEFINE_ATTRIBUTE(class)
		DEFINE_ATTRIBUTE(classid)
		DEFINE_ATTRIBUTE(code)
		DEFINE_ATTRIBUTE(codebase)
		DEFINE_ATTRIBUTE(codetype)
		DEFINE_ATTRIBUTE(color)
		DEFINE_ATTRIBUTE(cols)
		DEFINE_ATTRIBUTE(colspan)
		DEFINE_ATTRIBUTE(compact)
		DEFINE_ATTRIBUTE(content)
		DEFINE_ATTRIBUTE(contenteditable)
		DEFINE_ATTRIBUTE(contextmenu)
		DEFINE_ATTRIBUTE(controls)
		DEFINE_ATTRIBUTE(coords)
		DEFINE_ATTRIBUTE(data)
		DEFINE_ATTRIBUTE(data_target)	//	bootstrap 4
		DEFINE_ATTRIBUTE(data_toggle)	//	bootstrap 4
		DEFINE_ATTRIBUTE(datetime)
		DEFINE_ATTRIBUTE(declare)
		DEFINE_ATTRIBUTE(default)
		DEFINE_ATTRIBUTE(defer)
		DEFINE_ATTRIBUTE(dir)
		DEFINE_ATTRIBUTE(disabled)
		//DEFINE_ATTRIBUTE(display)
		DEFINE_ATTRIBUTE(download)
		DEFINE_ATTRIBUTE(draggable)
		DEFINE_ATTRIBUTE(dropzone)
		DEFINE_ATTRIBUTE(enctype)
		DEFINE_ATTRIBUTE(face)
		DEFINE_ATTRIBUTE(for)
		DEFINE_ATTRIBUTE(form)
		DEFINE_ATTRIBUTE(formaction)
		DEFINE_ATTRIBUTE(formenctype)
		DEFINE_ATTRIBUTE(formmethod)
		DEFINE_ATTRIBUTE(formnovalidate)
		DEFINE_ATTRIBUTE(formtarget)
		DEFINE_ATTRIBUTE(frame)
		DEFINE_ATTRIBUTE(frameborder)
		DEFINE_ATTRIBUTE(headers)
		DEFINE_ATTRIBUTE(height)
		DEFINE_ATTRIBUTE(hidden)
		DEFINE_ATTRIBUTE(high)
		DEFINE_ATTRIBUTE(href)
		DEFINE_ATTRIBUTE(hreflang)
		DEFINE_ATTRIBUTE(hspace)
		DEFINE_ATTRIBUTE(http_equiv)
		DEFINE_ATTRIBUTE(icon)
		DEFINE_ATTRIBUTE(id)
		DEFINE_ATTRIBUTE(ismap)
		DEFINE_ATTRIBUTE(keytype)
		DEFINE_ATTRIBUTE(kind)
		DEFINE_ATTRIBUTE(label)
		DEFINE_ATTRIBUTE(lang)
		DEFINE_ATTRIBUTE(language)
		DEFINE_ATTRIBUTE(link)
		DEFINE_ATTRIBUTE(list)
		DEFINE_ATTRIBUTE(loading)
		DEFINE_ATTRIBUTE(longdesc)
		DEFINE_ATTRIBUTE(loop)
		DEFINE_ATTRIBUTE(low)
		DEFINE_ATTRIBUTE(manifest)
		//DEFINE_ATTRIBUTE(margin_height)
		//DEFINE_ATTRIBUTE(margin_width)
		//DEFINE_ATTRIBUTE(margin_right)
		//DEFINE_ATTRIBUTE(margin_left)
		DEFINE_ATTRIBUTE(max)
		DEFINE_ATTRIBUTE(maxlength)
		DEFINE_ATTRIBUTE(media)
		DEFINE_ATTRIBUTE(method)
		DEFINE_ATTRIBUTE(min)
		DEFINE_ATTRIBUTE(multiple)
		DEFINE_ATTRIBUTE(muted)
		DEFINE_ATTRIBUTE(name)
		DEFINE_ATTRIBUTE(nohref)
		DEFINE_ATTRIBUTE(noresize)
		DEFINE_ATTRIBUTE(noshade)
		DEFINE_ATTRIBUTE(novalidate)
		DEFINE_ATTRIBUTE(nowrap)
		DEFINE_ATTRIBUTE(object)
		DEFINE_ATTRIBUTE(onabort)
		DEFINE_ATTRIBUTE(onafterprint)
		DEFINE_ATTRIBUTE(onbeforeprint)
		DEFINE_ATTRIBUTE(onbeforeunload)
		DEFINE_ATTRIBUTE(onblur)
		DEFINE_ATTRIBUTE(oncanplay)
		DEFINE_ATTRIBUTE(oncanplaythrough)
		DEFINE_ATTRIBUTE(onchange)
		DEFINE_ATTRIBUTE(onclick)
		DEFINE_ATTRIBUTE(oncontextmenu)
		DEFINE_ATTRIBUTE(ondblclick)
		DEFINE_ATTRIBUTE(ondrag)
		DEFINE_ATTRIBUTE(ondragend)
		DEFINE_ATTRIBUTE(ondragenter)
		DEFINE_ATTRIBUTE(ondragleave)
		DEFINE_ATTRIBUTE(ondragover)
		DEFINE_ATTRIBUTE(ondragstart)
		DEFINE_ATTRIBUTE(ondrop)
		DEFINE_ATTRIBUTE(ondurationchange)
		DEFINE_ATTRIBUTE(onemptied)
		DEFINE_ATTRIBUTE(onended)
		DEFINE_ATTRIBUTE(onerror)
		DEFINE_ATTRIBUTE(onfocus)
		DEFINE_ATTRIBUTE(onformchange)
		DEFINE_ATTRIBUTE(onforminput)
		DEFINE_ATTRIBUTE(onhaschange)
		DEFINE_ATTRIBUTE(oninput)
		DEFINE_ATTRIBUTE(oninvalid)
		DEFINE_ATTRIBUTE(onkeydown)
		DEFINE_ATTRIBUTE(onkeypress)
		DEFINE_ATTRIBUTE(onkeyup)
		DEFINE_ATTRIBUTE(onload)
		DEFINE_ATTRIBUTE(onloadeddata)
		DEFINE_ATTRIBUTE(onloadedmetadata)
		DEFINE_ATTRIBUTE(onloadstart)
		DEFINE_ATTRIBUTE(onmessage)
		DEFINE_ATTRIBUTE(onmousedown)
		DEFINE_ATTRIBUTE(onmousemove)
		DEFINE_ATTRIBUTE(onmouseout)
		DEFINE_ATTRIBUTE(onmouseover)
		DEFINE_ATTRIBUTE(onmouseup)
		DEFINE_ATTRIBUTE(onmousewheel)
		DEFINE_ATTRIBUTE(onoffline)
		DEFINE_ATTRIBUTE(ononline)
		DEFINE_ATTRIBUTE(onpagehide)
		DEFINE_ATTRIBUTE(onpageshow)
		DEFINE_ATTRIBUTE(onpause)
		DEFINE_ATTRIBUTE(onplay)
		DEFINE_ATTRIBUTE(onplaying)
		DEFINE_ATTRIBUTE(onpopstate)
		DEFINE_ATTRIBUTE(onprogress)
		DEFINE_ATTRIBUTE(onratechange)
		DEFINE_ATTRIBUTE(onreadystatechange)
		DEFINE_ATTRIBUTE(onredo)
		DEFINE_ATTRIBUTE(onreset)
		DEFINE_ATTRIBUTE(onresize)
		DEFINE_ATTRIBUTE(onscroll)
		DEFINE_ATTRIBUTE(onseeked)
		DEFINE_ATTRIBUTE(onseeking)
		DEFINE_ATTRIBUTE(onselect)
		DEFINE_ATTRIBUTE(onstalled)
		DEFINE_ATTRIBUTE(onstorage)
		DEFINE_ATTRIBUTE(onsubmit)
		DEFINE_ATTRIBUTE(onsuspend)
		DEFINE_ATTRIBUTE(ontimeupdate)
		DEFINE_ATTRIBUTE(onundo)
		DEFINE_ATTRIBUTE(onunload)
		DEFINE_ATTRIBUTE(onvolumechange)
		DEFINE_ATTRIBUTE(onwaiting)
		DEFINE_ATTRIBUTE(open)
		DEFINE_ATTRIBUTE(optimum)
		DEFINE_ATTRIBUTE(pattern)
		DEFINE_ATTRIBUTE(placeholder)
		DEFINE_ATTRIBUTE(poster)
		DEFINE_ATTRIBUTE(preload)
		DEFINE_ATTRIBUTE(profile)
		DEFINE_ATTRIBUTE(pubdate)
		DEFINE_ATTRIBUTE(radiogroup)
		DEFINE_ATTRIBUTE(readonly)
		DEFINE_ATTRIBUTE(rel)
		DEFINE_ATTRIBUTE(required)
		DEFINE_ATTRIBUTE(rev)
		DEFINE_ATTRIBUTE(reversed)
		DEFINE_ATTRIBUTE(role)	//	bootstrap 4
		DEFINE_ATTRIBUTE(rows)
		DEFINE_ATTRIBUTE(rowspan)
		DEFINE_ATTRIBUTE(rules)
		DEFINE_ATTRIBUTE(sandbox)
		DEFINE_ATTRIBUTE(scheme)
		DEFINE_ATTRIBUTE(scope)
		DEFINE_ATTRIBUTE(scrolling)
		DEFINE_ATTRIBUTE(seamless)
		DEFINE_ATTRIBUTE(selected)
		DEFINE_ATTRIBUTE(shape)
		DEFINE_ATTRIBUTE(size)
		DEFINE_ATTRIBUTE(span)
		DEFINE_ATTRIBUTE(spellcheck)
		DEFINE_ATTRIBUTE(src)
		DEFINE_ATTRIBUTE(srcdoc)
		DEFINE_ATTRIBUTE(srclang)
		DEFINE_ATTRIBUTE(standby)
		DEFINE_ATTRIBUTE(start)
		DEFINE_ATTRIBUTE(step)
		DEFINE_ATTRIBUTE(style)
		DEFINE_ATTRIBUTE(summary)
		DEFINE_ATTRIBUTE(tabindex)
		DEFINE_ATTRIBUTE(target)
		DEFINE_ATTRIBUTE(text)
		DEFINE_ATTRIBUTE(title)
		DEFINE_ATTRIBUTE(translate)
		DEFINE_ATTRIBUTE(type)
		DEFINE_ATTRIBUTE(usemap)
		DEFINE_ATTRIBUTE(valign)
		DEFINE_ATTRIBUTE(value)
		DEFINE_ATTRIBUTE(valuetype)
		DEFINE_ATTRIBUTE(vlink)
		DEFINE_ATTRIBUTE(vspace)
		DEFINE_ATTRIBUTE(width)
		DEFINE_ATTRIBUTE(wrap)
		DEFINE_ATTRIBUTE(xmlns)
	}
}

#endif
