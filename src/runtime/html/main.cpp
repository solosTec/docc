#include "controller.h"

#include <docc/utils.h>
#include <version.hpp>

#include <iostream>
#include <fstream>
#include <thread>
#include <locale>

#include <boost/program_options.hpp>
#include <boost/predef.h>
#include <boost/uuid/string_generator.hpp>
//#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <fmt/core.h>
#include <fmt/color.h>


int main(int argc, char* argv[]) {

	std::string config_file = std::string("doc2html-") + std::string(docc::version_string) + ".cfg";
	auto const here = std::filesystem::current_path();
    std::string inp_file = "main.docscript";
	std::string out_file = (here / "out.html").string();    
    std::string stag = "ef008e59-810f-427f-800b-d121c1b12deb";  //  stable toc IDs
    std::string index_file = (here / "index.json").string();
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
        ("include-path,I", boost::program_options::value< std::vector<std::string> >()->default_value(std::vector<std::string>(1, here.string()), here.string()), "include paths")
        //	verbose level
        ("verbose,V", boost::program_options::value<int>()->default_value(0)->implicit_value(1), "verbose level")
        ;

    boost::program_options::options_description gen("generator");
    gen.add_options()
        ("generator.body", boost::program_options::bool_switch()->default_value(false), "generate (HTML) body only")
        ("generator.meta", boost::program_options::bool_switch()->default_value(true), "generate a JSON file with meta data")
        ("generator.index", boost::program_options::value(&index_file)->default_value(index_file), "index file")
        ("generator.type,Y", boost::program_options::value<std::string>()->default_value("report"), "og:type (article/report/blog/...)")
        ;

    //
    //	all you can grab from the command line
    //
    boost::program_options::options_description cmdline_options;
    cmdline_options.add(generic).add(compiler).add(gen);

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
            << "doc2html generator v"
            << docc::version_string
            << std::endl
            ;
        return EXIT_SUCCESS;
    }

    docscript::init_console();

    std::ifstream ifs(config_file);
    if (!ifs)
    {
        fmt::print(
            stdout,
            fg(fmt::color::dark_orange) | fmt::emphasis::bold,
            "***warn  : config file [{}] not found\n", config_file);
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
    auto const inc_paths = docscript::get_include_paths(
        vm["include-path"].as< std::vector<std::string>>(),
        std::filesystem::path(inp_file).parent_path()
    );

    if (verbose > 1)    {
        fmt::print(
            stdout,
            fg(fmt::color::gray),
            "***info : {} include paths\n", inc_paths.size());

        std::copy(inc_paths.begin(), inc_paths.end(), std::ostream_iterator<std::filesystem::path>(std::cout, "\n"));
    }

    //
    //	get VM tag
    //
    auto const tag = boost::uuids::string_generator()(stag);

    //
    //	generate some temporary file names for intermediate files
    //
    std::filesystem::path const tmp_asm = std::filesystem::temp_directory_path() / ("doc2html-" + std::filesystem::path(inp_file).filename().string() + ".docs");
    std::filesystem::path const tmp_html = std::filesystem::temp_directory_path() / ("doc2html-" + stag + ".html");
    if (verbose > 2)   {
        fmt::print(
            stdout,
            fg(fmt::color::gray),
            "***info : temporary files {}\n", tmp_asm.string());
        fmt::print(
            stdout,
            fg(fmt::color::gray),
            "***info : temporary files {}\n", tmp_html.string());
    }

    //
    //  start compiler
    //
    
    docruntime::controller ctl(
        out_file.empty() ? std::filesystem::path(inp_file).replace_extension("html") : std::filesystem::path(out_file),
        inc_paths,
        tmp_asm,
        tmp_html,
        verbose
    );
    return ctl.run(docasm::verify_extension(inp_file, "docscript")
        , pool_size
        , tag
        , vm["generator.body"].as< bool >()
        , vm["generator.meta"].as< bool >()
        , index_file
        , vm["generator.type"].as< std::string >());
}