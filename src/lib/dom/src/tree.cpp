
#include <html/tree.h>
#include <html/formatting.h>

#include <sstream>
#include <iomanip>

//#include <boost/algorithm/string.hpp>

namespace dom
{

	void render_tree(std::ostream& os, std::filesystem::path const& root, std::size_t depth) {

		std::set<std::filesystem::path>	dirs;
		std::set<std::filesystem::path>	files;

		for (auto const& p : std::filesystem::directory_iterator{ root }) {
			if (p.is_regular_file()) {
				files.insert(p.path());
			}
			else if (p.is_directory()) {
				dirs.insert(p.path());
			}
			else {
				;	//	ignore
			}
		}

		os << std::string(depth, ' ') << "<ul>" << std::endl;

		//
		//	1. files
		//
		//os << "<li>" << files.size() << " FILES:</li>" << std::endl;

		for (auto const& p : files) {
			os << std::string(depth, ' ')
				<< "<li class=\"doc-file\">"
				<< p.filename().string()
				<< " ("
				<< std::filesystem::file_size(p)
				<< " bytes)</li>"
				<< std::endl;
		}

		//
		//	2. subdirectories
		//
		//os << "<li>" << dirs.size() << " DIRECTORIES:</li>" << std::endl;
		for (auto const& p : dirs) {
			os << std::string(depth, ' ')
				<< "<li class=\"doc-dir\">"
				<< p.filename().string()
				<< "</li>"
				<< std::endl;
			render_tree(os, p, depth + 1);
		}
		os << std::string(depth, ' ') << "</ul>" << std::endl;
	}

	void render_tree(std::ostream& os, std::filesystem::path const& root) {
		render_tree(os, root, 0);
	}


}