#include <version.hpp>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <thread>

#include <boost/program_options.hpp>
#include <boost/predef.h>

#include <fmt/core.h>
#include <fmt/color.h>

#include "controller.h"

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

std::vector<std::filesystem::path> get_include_paths(std::vector<std::string> const& vec, std::filesystem::path parent_path) {

    //
    //  convert from string to path
    //
    std::vector<std::filesystem::path> includes(vec.begin(), vec.end());

    //
    //	Add the path of the input file as include path, if it is not already specified
    //
    auto pos = std::find(vec.begin(), vec.end(), parent_path);
    if (pos == vec.end() && !parent_path.empty()) {
        includes.push_back(parent_path);
    }

    //
    //	last entry is empty
    //
#if BOOST_OS_WINDOWS
    includes.push_back(".\\");
#else
    includes.push_back("./");
#endif

    return includes;

}

int main(int argc, char* argv[]) {

#ifdef _DEBUG
#endif

	std::string config_file = std::string(docc::project_name)+ "-" + std::string(docc::version_string) + ".cfg";
	auto const here = std::filesystem::current_path();
    std::string inp_file = "main.docscript";
	std::string out_file = (here / "out.docs").string();    
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
    //	all compiler options
    //
    boost::program_options::options_description compiler("compiler");
    compiler.add_options()

        ("source,S", boost::program_options::value(&inp_file)->default_value(inp_file), "main source file")
        ("output,O", boost::program_options::value(&out_file)->implicit_value("")->default_value(out_file), "output file")
        ("include-path,I", boost::program_options::value< std::vector<std::string> >()->default_value(std::vector<std::string>(1, here.string()), here.string()), "include paths")
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
            << "docScript compiler v"
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
    //	read specified include paths
    //
    auto const inc_paths = get_include_paths(
        vm["include-path"].as< std::vector<std::string>>(),
        std::filesystem::path(inp_file).parent_path()
        );

 
    if (verbose > 1)
    {
        fmt::print(
            stdout,
            fg(fmt::color::gray),
            "***info : {} include paths\n", inc_paths.size());

        std::copy(inc_paths.begin(), inc_paths.end(), std::ostream_iterator<std::filesystem::path>(std::cout, "\n"));
    }


    //
    //  start tasks
    //
    
    docscript::controller ctl(
        out_file.empty() ? std::filesystem::path(inp_file).replace_extension("docs") : std::filesystem::path(out_file),
        inc_paths,
        verbose);

    return ctl.run(std::filesystem::path(inp_file).filename(),
        pool_size);

}