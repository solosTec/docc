<!-- author:	{solosTec} -->
<!-- file-name:	readme.docscript -->
<!-- file-size:	5037 -->
<!-- last-write-time:	2019-05-13 11:45:08.00000000 -->
<!-- text-entropy:	4.93656 -->
<!-- title:	readme -->
<!-- token-count:	4696 -->
<!-- total-file-size:	5037 -->

[![Codacy Badge](https://api.codacy.com/project/badge/Grade/d4494601ebc84a2da2057f76652624df)](https://www.codacy.com/app/solosTec/docc?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=solosTec/docc&amp;utm_campaign=Badge_Grade)

# 1 Overview
Generate HTML, Markdown and L^A^T~E~X files from one single source file written in _docScript_. docc is a compiler for the docScript language to generate documents. It is positioned between Markdown and LaTex.
## 1.1 Features
Since you have one source file for different target formats, you don't have to memorize all the tiny syntactic quirks of LaTeX, Markdown or HTML.
1. syntax as simple as markdown, but with more features
2. docScript - allows to **program** your document ( Turing complete)
3. documents can be compiled from different sources
4. UTF-8 support
5. pretty fast
6. auto generate numbering
7. auto generate meta data
8. pre-rendering of other sources - e. g. binary data as hexdump
9. comprehensive error messages
10. extensible

# 2 Usage
## 2.1 Building
Dependencies:
* CMake &#8805; 3.13
* recent C++ compiler
* Boost \> 1.67
* OpenSSL
* [cyng](https://github.com/solosTec/cyng "VM with dynamic data types") library

### 2.1.1 Linux
```sh
$ git clone https://github.com/solosTec/docc.git
$ cd docc
$ mkdir build && cd build
$ cmake ..
$ make
$; sudo make install
```

### 2.1.2 Windows
```cmd
> git clone https://github.com/solosTec/docc.git
> cd docc
> mkdir build 
> cd build
> cmake -G "Visual Studio 15 2017" -A x64 ..
> REM cmake -G "Visual Studio 16 2019" -A x64 ..
> REM start devenv.exe (Visual Studio)
```

## 2.2 Usage
```verbatim
$ docc -h

Generic options:
  -h [ --help ]                         print usage message
  -v [ --version ]                      print version string
  -b [ --build ]                        last built timestamp and platform
  -C [ --config ] arg (=docscript.cfg)  configuration file

compiler:
  -S [ --source ] arg (=main.docscript) main source file
  -O [ --output ] arg (=/home/sol/projects/docc/out.html)
                                        output file
  -I [ --include-path ] arg (=/home/sol/projects/docc)
                                        include path
  -V [ --verbose ] [=arg(=1)] (=0)      verbose level
  --body                                generate only HTML body
  --meta                                generate a JSON file with meta data
  --index                               generate an index file "index.json"


```

# 3 DocScript
Writing in _docScript_ is as simple as Markdown but with a more formalized syntax. One of the main objectives of _docScript_ is to make as less syntactic noise as possible. It exploits the fact, that a word never starts with a dot. So every literal that starts with a dot is a potential docScript keyword. In case you really want a word with leading point, simply write two consecutive points. This is possible since the dot is also the escape sign.
## 3.1 Special Characters


colon
: :

dot
: .

round brackets
: ()

square backets
: \[\]

That's all.
## 3.2 Basic Rules
To generate a paragraph simple write a text. To separate paragraphs, insert a **blank** line.
To avoid any processing put your text into ' single quotation ' marks.
## 3.3 Parameters
Parameters can be a simple list of words, numbers and symbols or key value pairs separated by a comma. Key vlaue pairs consist of a key and a value separated by colon: **key:value**. Values can be the results of a function call, but keys cannot.
## 3.4 Commands
Commands start with a dot. If the command has only one parameter no brackets are required. In case the command has no parameters an opening and closing bracket without contents is necessary.
Example: **.now()**
### 3.4.1 Formatting
#### 3.4.1.1 bold
Example: .b bold &#10233; **bold**
#### 3.4.1.2 italic
Example: .i(emphasized) &#10233; _emphasized_
#### 3.4.1.3 sup, sub
Super and subscript is not supported by Github Markdown.
#### 3.4.1.4 color
Colors are not supported by Markdown.
### 3.4.2 Header
A header has the following options:


level
: indentation depth

tag
: unique identifier

title
: caption title

Example:
```docscript
.header(title: "Basics", level: 1, tag: '79bf3ba0-2362-4ea5-bcb5-ed93844ac59a')

.h2 Details

```

To simplify writing headers the shortcuts **h1** up to **h6** are defined.
### 3.4.3 Lists
A list has the following options:


items
: a list of items as \[vector\]

style
: list style

type
: available options are ordered and unordered

Example:
```docscript
.list(type:unordered
 , style: 'circle'
 , items: [
	(CMake >= 3.13),	
	(recent C++ compiler),
	(Boost > 1.67), 
	(OpenSSL), 
	(.link(text:'cyng', url:'https://github.com/solosTec/cyng', title:'VM with dynamic data types') library)])

```

### 3.4.4 Definitions
### 3.4.5 Code Listings
The command is **.code**. A listing has the following options:


caption
: Optional title

language
: programming language

line_numbers
: \[bool\] print line numbers

source
: source file

The following languages are strongly supported:
* C++
* JSON
* txt
* binary data as hexdump
* docScript

In case of dubt choose _txt_ format.
### 3.4.6 Link


text
: Text to display

title
: optional tooltip

url
: URL that the hyperlink points to

### 3.4.7 Figure
The specified image will be embedded into the HTML file as base64 encoded string.


alt
: alternative text

caption
: Title below the image

source
: path of source/image file

tag
: label

### 3.4.8 Quote


cite
: text below the quote

quote
: the quote itself

source
: source of quote

### 3.4.9 Set
Define one or more variables.
Example: **.set(name:docc, language:C++)**
### 3.4.10 Get
Get value of a variable.
Example: **.get(name)**
### 3.4.11 Meta
Define a set of document meta data.
Example: **.meta(title:readme, author:solosTec)**
