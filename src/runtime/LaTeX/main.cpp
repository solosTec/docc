#include "controller.h"

#include <version.hpp>

#include <iostream>
#include <fstream>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/predef.h>
#include <boost/uuid/string_generator.hpp>

#include <fmt/core.h>
#include <fmt/color.h>

#if BOOST_OS_WINDOWS
#include "Windows.h"
//
//	set console outpt code page to UTF-8
//	requires a TrueType font like Lucida 
//
void init_console() {
    if (::SetConsoleOutputCP(65001) == 0)
    {
        std::cerr
            << "Cannot set console code page"
            << std::endl
            ;

    }
    auto h_out = ::GetStdHandle(STD_OUTPUT_HANDLE);
    if (h_out != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (::GetConsoleMode(h_out, &dwMode)) {
            ::SetConsoleMode(h_out, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
}
#else
void init_console() {
}
#endif


int main(int argc, char* argv[]) {

#ifdef _DEBUG
#endif

	std::string config_file = std::string("do2html-") + std::string(docc::version_string) + ".cfg";
	auto const here = std::filesystem::current_path();
    std::string inp_file = "main.cyng";
	std::string out_file = (here / "out.html").string();    
    std::string stag = "d28dde25-8445-4fc7-a4f0-76a33a4c6179";
    std::size_t pool_size = std::min<std::size_t>(std::thread::hardware_concurrency(), 4) * 2ul;

    //
    //	generic options
    //
    boost::program_options::options_description generic("Generic options");
    generic.add_options()

        ("help,h", "print usage message")
        ("version,v", "print version string")
        ("config,C", boost::program_options::value<std::string>(&config_file)->default_value(config_file), "configuration file")

        ;

    //
    //	all runtime options
    //
    boost::program_options::options_description compiler("runtime");
    compiler.add_options()

        ("source,S", boost::program_options::value(&inp_file)->default_value(inp_file), "main source file")
        ("output,O", boost::program_options::value(&out_file)->default_value(out_file), "output file")
        ("tag,T", boost::program_options::value(&stag)->default_value(stag), "VM tag")
        //("include-path,I", boost::program_options::value< std::vector<std::string> >()->default_value(std::vector<std::string>(1, here.string()), here.string()), "include paths")
        //	verbose level
        ("verbose,V", boost::program_options::value<int>()->default_value(0)->implicit_value(1), "verbose level")
        ;

    //
    //	all you can grab from the command line
    //
    boost::program_options::options_description cmdline_options;
    cmdline_options.add(generic).add(compiler);

    //
    //	positional arguments
    //
    boost::program_options::positional_options_description p;
    p.add("source", -1);

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::command_line_parser(argc, argv).options(cmdline_options).positional(p).run(), vm);
    boost::program_options::notify(vm);

    if (vm.count("help"))
    {
        std::cout
            << cmdline_options
            << std::endl
            ;
        return EXIT_SUCCESS;
    }

    if (vm.count("version"))
    {
        std::cout
            << "doc2LaTeX generator v"
            << docc::version_string
            << std::endl
            ;
        return EXIT_SUCCESS;
    }

    init_console();

    std::ifstream ifs(config_file);
    if (!ifs)
    {
        fmt::print(
            stdout,
            fg(fmt::color::dark_orange) | fmt::emphasis::bold,
            "***info : config file [{}] not found\n", config_file);
    }
    else
    {
        //
        //	options available from config file
        //
        boost::program_options::options_description file_options;
        file_options.add_options()
            ("pool-size,P", boost::program_options::value(&pool_size)->default_value(pool_size), "Thread pool size")
            ;
        file_options.add(compiler);

        boost::program_options::store(boost::program_options::parse_config_file(ifs, file_options), vm);
        boost::program_options::notify(vm);
    }

    //
    //  verbose level
    //
    auto const verbose = vm["verbose"].as< int >();

    fmt::print(
        stdout,
        fg(fmt::color::gray),
        "***info : verbose level = [{}]\n", verbose);

 
    //
    //	get VM tag
    //
    auto const tag = boost::uuids::string_generator()(stag);

    //
    //  start tasks
    //
    
    docruntime::controller ctl(
        out_file,
        verbose
    );
    return ctl.run(docruntime::verify_extension(inp_file, "cyng"), pool_size, tag);
}