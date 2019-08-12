<!-- author:	solosTec -->
<!-- file-name:	readme.docscript -->
<!-- file-size:	7909 -->
<!-- language:	en -->
<!-- last-write-time:	2019-08-12 11:52:48.00000000 -->
<!-- og:type:	report -->
<!-- outline:	Introduction into the DocScript document generator -->
<!-- released:	2019-08-02 00:00:00.00000000 -->
<!-- text-entropy:	5.03184 -->
<!-- title:	README -->
<!-- token-count:	7629 -->
<!-- total-file-size:	7909 -->

# 1 Overview
Generate HTML, Markdown and L^A^T~E~X files from one single source file written in _docScript_. docc is a compiler for the docScript language to generate documents. It is positioned between Markdown and LaTex.
## 1.1 Features
Since you have one source file for different target formats, you don't have to memorize all the tiny syntactic quirks of LaTeX, Markdown or HTML.
1. syntax as **simple** as markdown, but with more features
2. docScript - allows to **program** your document (Turing complete)
3. documents can be compiled from different sources
4. generate compact HTML files with **inline images**
5. **UTF-8** support
6. pretty fast
7. auto generate numbering
8. auto generate meta data
9. **pre-rendering** of other sources - e. g. binary data as hexdump
10. comprehensive error messages
11. hackable

# 2 Usage
## 2.1 Building
Dependencies:
* CMake &#8805; 3.13
* recent C++ compiler
* Boost \> 1.67
* OpenSSL
* [CYNG](https://github.com/solosTec/cyng "VM with dynamic data types") library
* [NODE](https://github.com/solosTec/node "SMF") library

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
## 3.2 Specify a datetime
To specify a datetime the @ symbol can be used followed by a string according to RFC 3339.
Datetimes can be used as parameters and as part of an paragraph.
Example (1): @2022-10-02 &#10233; **2022-10-02 00:00:00**
Example (2): @2022-10-02T10:00:02 &#10233; **2022-10-02 10:00:02**
## 3.3 Basic Rules
To create a paragraph, just write text that follows an empty line. To separate paragraphs, insert a **blank** line.
To avoid any processing put your text into ' single quotation ' marks.
## 3.4 Parameters
Parameters can be a simple list of words, numbers and symbols or key value pairs separated by a comma. Key value pairs consist of a key and a value separated by colon: **key:value**. Values can be the results of a function call, but keys cannot.
## 3.5 Commands
Commands start with a dot. If the command has only one parameter no brackets are required. In case the command has no parameters an opening and closing bracket without contents is necessary.
Example: **.now()**
### 3.5.1 Formatting
#### 3.5.1.1 bold
Example: .b bold &#10233; **bold**
#### 3.5.1.2 italic
Example: .i(emphasized) &#10233; _emphasized_
#### 3.5.1.3 sup, sub
Super and subscript are not supported by Github Markdown.
Example: .sup(sup) X .sub(sub) &#10233; ^sup^ X ~sub~
#### 3.5.1.4 color
Colors are not supported by Markdown.
### 3.5.2 Header
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

To simplify writing headers the shortcuts **h1** up to **h6** are predefined.
### 3.5.3 Lists
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

* CMake &#8805; 3.13
* recent C++ compiler
* Boost \> 1.67
* OpenSSL
* [cyng](https://github.com/solosTec/cyng "VM with dynamic data types") library

### 3.5.4 Definitions
Example:
```docscript
.def(caption:'Optional title', 
	line_numbers:'[bool] print line numbers', 
	source:'source file',
	language:'programming language')

```

### 3.5.5 Code Listings
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

In case of doubt choose _txt_ format.
### 3.5.6 Link


text
: Text to display

title
: optional tooltip

url
: URL that the hyperlink points to

### 3.5.7 Figure
The specified image will be embedded into the HTML file as base64 encoded string.


alt
: alternative text

caption
: Title below the image

source
: path of source/image file

tag
: label

### 3.5.8 Quote


cite
: text below the quote

quote
: the quote itself

source
: source of quote

### 3.5.9 Note
The **.note** command generates a side note. LaTeX uses the \\marginpar\{\} command. HTML mimics the behaviour with an absolut position. Markdown simulates this by a blockquote.

> This is a marginal note.

### 3.5.10 Abstract
The specified image will be embedded into the HTML file as base64 encoded string.


text
: The statement

title
: Title of abstract

### 3.5.11 Tables
Tables are created from CSV files.


header
: overwrite header from CSV file

source
: path to CSV file

title
: table title

Values are separated by, or ;
Example:
```docscript
.table(source:'inc/table.csv', header:"uuid string int", title:'Caption')

```


| UUID| string| number |
| ----| ------| ------ |
| 83d78038-46f8-453a-b453-a56c67e0af1c| name-1| 1 |
| b1a8befb-ec42-4d77-8af1-a07e14f16549| name-2| 2 |
| b5275a88-adf2-4225-9998-4be5fd7003cd| name-3| 3 |
| 8c5bbb07-a2ca-4167-9e3e-aed8f48f097c| name-4| 4 |
| cd6995d6-c8c0-43b0-853a-c42a94eceb95| name-5| 5 |

### 3.5.12 Symbols
The following symbols are pre-defined:
* pilcrow: &#182;
* copyright: &#169;
* registered: &#174;
* latex: L^A^T~E~X
* celsius: &#8451;
* micro: &#181;
* ohm: &#8486;
* degree: &#176;
* promille: &#8240;
* lambda: &#8257;
* ellipsis: &#2026;

Example: **.symbol(lambda)**
### 3.5.13 Currency
The following currencies are pre-defined:
* euro: &euro;
* yen: &yen;
* pound: &pound;
* rupee: &#x20B9;
* sheqel: &#8362;

### 3.5.14 Set
Define one or more variables.
Example:
```docscript
 .set(name:docc, language:C++)
```

### 3.5.15 Get
Get value of a variable.
Example:
```docscript
.get(name)

.get(language)
```

### 3.5.16 Meta
Define a set of document meta data.
Example: **.meta(title:readme, author:solosTec)**
### 3.5.17 Generator Functions
#### 3.5.17.1 tag
Generates a random [uuid](https://de.wikipedia.org/wiki/Universally_Unique_Identifier "Universally Unique Identifier").
Example:
```docscript
.set(label: .tag())

```

