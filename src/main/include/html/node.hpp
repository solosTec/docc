/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Sylko Olzscher
 *
 */

#ifndef HTML_NODE_H
#define HTML_NODE_H

#include <html/element.hpp>
#include <html/attribute.hpp>
#include <list>

namespace html
{
	class node : public base
	{
	public:
		explicit node(std::string const &name)
			: base(name)
			, attrs_()
			, nodes_()
		{}

		template<class ...Args>
		explicit node(std::string const& tag, Args&&... args) 
			: base(tag)
		{
			assign(std::forward<Args>(args)...);
		}

		/**
		 * simple serialization to string
		 */
		virtual std::string to_str() const override;

		/**
		 * append child
		 */
		node& operator+=(node&&);
		
	private:
		template<class T, class ...Args>
		void assign(T&& val, Args &&...args)
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
			if constexpr (std::is_base_of<attr, typename std::decay<T>::type>::value) {
				//
				//	any kind of attribute
				//
				attrs_.emplace_back(std::move(val));
			}
			else if constexpr (std::is_invocable<T>::value) {
				if constexpr (is_string < typename std::invoke_result<T>::type >::value) {
					//
					//	function with return type string
					//
					nodes_.emplace_back(val);
				}
			}
			else if constexpr (is_string<typename std::decay<T>::type>::value) {
				//
				//	simple string type
				//
				nodes_.emplace_back([val](){ return val; });
			}
			else {
				//	simple non-string
				nodes_.emplace_back([val]() { return std::to_string(val); });
			}
		}

	protected:
		std::list<std::function<std::string()>> attrs_;
		std::list<std::function<std::string()>> nodes_;
	};

	//
	//	define factory functions
	//
#define DEFINE_NODE(X) template<class ...Args> node X(Args &&...args){return node(#X, std::forward<Args>(args)...);}
#define DEFINE_ATTR(X) template<class T> attr X##_(T &&val){return attr(#X, std::forward<T>(val));}

	//template<class ...Args> node a(Args &&...args) { 
	//	return std::move(node("a", std::forward<Args>(args)...)); 
	//}
	inline namespace nodes
	{
		DEFINE_NODE(a)
		DEFINE_NODE(abbr)
		DEFINE_NODE(acronym)
		DEFINE_NODE(address)
		DEFINE_NODE(applet)
		DEFINE_NODE(area)
		DEFINE_NODE(article)
		DEFINE_NODE(aside)
		DEFINE_NODE(audio)
		DEFINE_NODE(autocomplete)
		DEFINE_NODE(autofocus)
		DEFINE_NODE(b)
		DEFINE_NODE(base)
		DEFINE_NODE(basefont)
		DEFINE_NODE(bdi)
		DEFINE_NODE(bdo)
		DEFINE_NODE(big)
		DEFINE_NODE(blockquote)
		DEFINE_NODE(body)
		DEFINE_NODE(br)
		DEFINE_NODE(button)
		DEFINE_NODE(canvas)
		DEFINE_NODE(caption)
		DEFINE_NODE(center)
		DEFINE_NODE(cite)
		DEFINE_NODE(code)
		DEFINE_NODE(col)
		DEFINE_NODE(colgroup)
		DEFINE_NODE(command)
		DEFINE_NODE(datalist)
		DEFINE_NODE(dd)
		DEFINE_NODE(del)
		DEFINE_NODE(details)
		DEFINE_NODE(dfn)
		DEFINE_NODE(dialog)
		DEFINE_NODE(dir)
		DEFINE_NODE(div)
		DEFINE_NODE(dl)
		DEFINE_NODE(dt)
		DEFINE_NODE(em)
		DEFINE_NODE(embed)
		DEFINE_NODE(fieldset)
		DEFINE_NODE(figcaption)
		DEFINE_NODE(figure)
		DEFINE_NODE(font)
		DEFINE_NODE(footer)
		DEFINE_NODE(form)
		DEFINE_NODE(frame)
		DEFINE_NODE(frameset)
		DEFINE_NODE(h1)
		DEFINE_NODE(h2)
		DEFINE_NODE(h3)
		DEFINE_NODE(h4)
		DEFINE_NODE(h5)
		DEFINE_NODE(h6)
		DEFINE_NODE(head)
		DEFINE_NODE(header)
		DEFINE_NODE(hr)
		DEFINE_NODE(html)
		DEFINE_NODE(i)
		DEFINE_NODE(iframe)
		DEFINE_NODE(img)
		DEFINE_NODE(input)
		DEFINE_NODE(ins)
		DEFINE_NODE(isindex)
		DEFINE_NODE(kbd)
		DEFINE_NODE(keygen)
		DEFINE_NODE(label)
		DEFINE_NODE(legend)
		DEFINE_NODE(li)
		DEFINE_NODE(link)
		DEFINE_NODE(map)
		DEFINE_NODE(mark)
		DEFINE_NODE(menu)
		DEFINE_NODE(menuitem)
		DEFINE_NODE(meta)
		DEFINE_NODE(meter)
		DEFINE_NODE(nav)
		DEFINE_NODE(noframes)
		DEFINE_NODE(noscript)
		DEFINE_NODE(object)
		DEFINE_NODE(ol)
		DEFINE_NODE(optgroup)
		DEFINE_NODE(option)
		DEFINE_NODE(output)
		DEFINE_NODE(p)
		DEFINE_NODE(param)
		DEFINE_NODE(pre)
		DEFINE_NODE(progress)
		DEFINE_NODE(q)
		DEFINE_NODE(rp)
		DEFINE_NODE(rt)
		DEFINE_NODE(ruby)
		DEFINE_NODE(s)
		DEFINE_NODE(samp)
		DEFINE_NODE(script)
		DEFINE_NODE(section)
		DEFINE_NODE(select)
		//DEFINE_NODE(small)
		DEFINE_NODE(source)
		DEFINE_NODE(span)
		DEFINE_NODE(strike)
		DEFINE_NODE(strong)
		DEFINE_NODE(style)
		DEFINE_NODE(sub)
		DEFINE_NODE(summary)
		DEFINE_NODE(sup)
		DEFINE_NODE(table)
		DEFINE_NODE(tbody)
		DEFINE_NODE(td)
		DEFINE_NODE(textarea)
		DEFINE_NODE(tfoot)
		DEFINE_NODE(th)
		DEFINE_NODE(thead)
		DEFINE_NODE(time)
		DEFINE_NODE(title)
		DEFINE_NODE(tr)
		DEFINE_NODE(track)
		DEFINE_NODE(tt)
		DEFINE_NODE(u)
		DEFINE_NODE(ul)
		DEFINE_NODE(var)
		DEFINE_NODE(video)
		DEFINE_NODE(wbr)
		DEFINE_NODE(xmp)
	}

	inline namespace attrs
	{
		DEFINE_ATTR(abbr)
		DEFINE_ATTR(accept)
		DEFINE_ATTR(accept_charset)
		DEFINE_ATTR(accesskey)
		DEFINE_ATTR(action)
		DEFINE_ATTR(align)
		DEFINE_ATTR(alink)
		DEFINE_ATTR(alt)
		DEFINE_ATTR(archive)
		DEFINE_ATTR(async)
		DEFINE_ATTR(autocomplete)
		DEFINE_ATTR(autofocus)
		DEFINE_ATTR(autoplay)
		DEFINE_ATTR(axis)
		DEFINE_ATTR(background)
		DEFINE_ATTR(bgcolor)
		DEFINE_ATTR(border)
		DEFINE_ATTR(cellpadding)
		DEFINE_ATTR(cellspacing)
		DEFINE_ATTR(challenge)
		DEFINE_ATTR(char)
		DEFINE_ATTR(charoff)
		DEFINE_ATTR(charset)
		DEFINE_ATTR(checked)
		DEFINE_ATTR(cite)
		DEFINE_ATTR(class)
		DEFINE_ATTR(classid)
		DEFINE_ATTR(code)
		DEFINE_ATTR(codebase)
		DEFINE_ATTR(codetype)
		DEFINE_ATTR(color)
		DEFINE_ATTR(cols)
		DEFINE_ATTR(colspan)
		DEFINE_ATTR(compact)
		DEFINE_ATTR(content)
		DEFINE_ATTR(contenteditable)
		DEFINE_ATTR(contextmenu)
		DEFINE_ATTR(controls)
		DEFINE_ATTR(coords)
		DEFINE_ATTR(data)
		DEFINE_ATTR(datetime)
		DEFINE_ATTR(declare)
		DEFINE_ATTR(default)
		DEFINE_ATTR(defer)
		DEFINE_ATTR(dir)
		DEFINE_ATTR(disabled)
		DEFINE_ATTR(download)
		DEFINE_ATTR(draggable)
		DEFINE_ATTR(dropzone)
		DEFINE_ATTR(enctype)
		DEFINE_ATTR(face)
		DEFINE_ATTR(for)
		DEFINE_ATTR(form)
		DEFINE_ATTR(formaction)
		DEFINE_ATTR(formenctype)
		DEFINE_ATTR(formmethod)
		DEFINE_ATTR(formnovalidate)
		DEFINE_ATTR(formtarget)
		DEFINE_ATTR(frame)
		DEFINE_ATTR(frameborder)
		DEFINE_ATTR(headers)
		DEFINE_ATTR(height)
		DEFINE_ATTR(hidden)
		DEFINE_ATTR(high)
		DEFINE_ATTR(href)
		DEFINE_ATTR(hreflang)
		DEFINE_ATTR(hspace)
		DEFINE_ATTR(http_equiv)
		DEFINE_ATTR(icon)
		DEFINE_ATTR(id)
		DEFINE_ATTR(ismap)
		DEFINE_ATTR(keytype)
		DEFINE_ATTR(kind)
		DEFINE_ATTR(label)
		DEFINE_ATTR(lang)
		DEFINE_ATTR(language)
		DEFINE_ATTR(link)
		DEFINE_ATTR(list)
		DEFINE_ATTR(longdesc)
		DEFINE_ATTR(loop)
		DEFINE_ATTR(low)
		DEFINE_ATTR(manifest)
		DEFINE_ATTR(marginheight)
		DEFINE_ATTR(marginwidth)
		DEFINE_ATTR(max)
		DEFINE_ATTR(maxlength)
		DEFINE_ATTR(media)
		DEFINE_ATTR(method)
		DEFINE_ATTR(min)
		DEFINE_ATTR(multiple)
		DEFINE_ATTR(muted)
		DEFINE_ATTR(name)
		DEFINE_ATTR(nohref)
		DEFINE_ATTR(noresize)
		DEFINE_ATTR(noshade)
		DEFINE_ATTR(novalidate)
		DEFINE_ATTR(nowrap)
		DEFINE_ATTR(object)
		DEFINE_ATTR(onabort)
		DEFINE_ATTR(onafterprint)
		DEFINE_ATTR(onbeforeprint)
		DEFINE_ATTR(onbeforeunload)
		DEFINE_ATTR(onblur)
		DEFINE_ATTR(oncanplay)
		DEFINE_ATTR(oncanplaythrough)
		DEFINE_ATTR(onchange)
		DEFINE_ATTR(onclick)
		DEFINE_ATTR(oncontextmenu)
		DEFINE_ATTR(ondblclick)
		DEFINE_ATTR(ondrag)
		DEFINE_ATTR(ondragend)
		DEFINE_ATTR(ondragenter)
		DEFINE_ATTR(ondragleave)
		DEFINE_ATTR(ondragover)
		DEFINE_ATTR(ondragstart)
		DEFINE_ATTR(ondrop)
		DEFINE_ATTR(ondurationchange)
		DEFINE_ATTR(onemptied)
		DEFINE_ATTR(onended)
		DEFINE_ATTR(onerror)
		DEFINE_ATTR(onfocus)
		DEFINE_ATTR(onformchange)
		DEFINE_ATTR(onforminput)
		DEFINE_ATTR(onhaschange)
		DEFINE_ATTR(oninput)
		DEFINE_ATTR(oninvalid)
		DEFINE_ATTR(onkeydown)
		DEFINE_ATTR(onkeypress)
		DEFINE_ATTR(onkeyup)
		DEFINE_ATTR(onload)
		DEFINE_ATTR(onloadeddata)
		DEFINE_ATTR(onloadedmetadata)
		DEFINE_ATTR(onloadstart)
		DEFINE_ATTR(onmessage)
		DEFINE_ATTR(onmousedown)
		DEFINE_ATTR(onmousemove)
		DEFINE_ATTR(onmouseout)
		DEFINE_ATTR(onmouseover)
		DEFINE_ATTR(onmouseup)
		DEFINE_ATTR(onmousewheel)
		DEFINE_ATTR(onoffline)
		DEFINE_ATTR(ononline)
		DEFINE_ATTR(onpagehide)
		DEFINE_ATTR(onpageshow)
		DEFINE_ATTR(onpause)
		DEFINE_ATTR(onplay)
		DEFINE_ATTR(onplaying)
		DEFINE_ATTR(onpopstate)
		DEFINE_ATTR(onprogress)
		DEFINE_ATTR(onratechange)
		DEFINE_ATTR(onreadystatechange)
		DEFINE_ATTR(onredo)
		DEFINE_ATTR(onreset)
		DEFINE_ATTR(onresize)
		DEFINE_ATTR(onscroll)
		DEFINE_ATTR(onseeked)
		DEFINE_ATTR(onseeking)
		DEFINE_ATTR(onselect)
		DEFINE_ATTR(onstalled)
		DEFINE_ATTR(onstorage)
		DEFINE_ATTR(onsubmit)
		DEFINE_ATTR(onsuspend)
		DEFINE_ATTR(ontimeupdate)
		DEFINE_ATTR(onundo)
		DEFINE_ATTR(onunload)
		DEFINE_ATTR(onvolumechange)
		DEFINE_ATTR(onwaiting)
		DEFINE_ATTR(open)
		DEFINE_ATTR(optimum)
		DEFINE_ATTR(pattern)
		DEFINE_ATTR(placeholder)
		DEFINE_ATTR(poster)
		DEFINE_ATTR(preload)
		DEFINE_ATTR(profile)
		DEFINE_ATTR(pubdate)
		DEFINE_ATTR(radiogroup)
		DEFINE_ATTR(readonly)
		DEFINE_ATTR(rel)
		DEFINE_ATTR(required)
		DEFINE_ATTR(rev)
		DEFINE_ATTR(reversed)
		DEFINE_ATTR(rows)
		DEFINE_ATTR(rowspan)
		DEFINE_ATTR(rules)
		DEFINE_ATTR(sandbox)
		DEFINE_ATTR(scheme)
		DEFINE_ATTR(scope)
		DEFINE_ATTR(scrolling)
		DEFINE_ATTR(seamless)
		DEFINE_ATTR(selected)
		DEFINE_ATTR(shape)
		DEFINE_ATTR(size)
		DEFINE_ATTR(span)
		DEFINE_ATTR(spellcheck)
		DEFINE_ATTR(src)
		DEFINE_ATTR(srcdoc)
		DEFINE_ATTR(srclang)
		DEFINE_ATTR(standby)
		DEFINE_ATTR(start)
		DEFINE_ATTR(step)
		DEFINE_ATTR(style)
		DEFINE_ATTR(summary)
		DEFINE_ATTR(tabindex)
		DEFINE_ATTR(target)
		DEFINE_ATTR(text)
		DEFINE_ATTR(title)
		DEFINE_ATTR(translate)
		DEFINE_ATTR(type)
		DEFINE_ATTR(usemap)
		DEFINE_ATTR(valign)
		DEFINE_ATTR(value)
		DEFINE_ATTR(valuetype)
		DEFINE_ATTR(vlink)
		DEFINE_ATTR(vspace)
		DEFINE_ATTR(width)
		DEFINE_ATTR(wrap)
		DEFINE_ATTR(xmlns)
	}
}

#endif
