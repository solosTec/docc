
#include <rt/stream.h>

#include <iterator>

#include <boost/assert.hpp>

namespace docruntime
{
	std::pair<cyng::buffer_t, std::streamsize> prepare_buffer(std::istream& is) {
	
		//	https://stackoverflow.com/a/22986486
		//	This test is disabled in the special case when count equals std::numeric_limits<std::streamsize>::max()
		//is.ignore(std::numeric_limits<std::streamsize>::max() - 1);
		//auto const length = is.gcount();
		is.seekg(0, std::ios_base::end);
		auto const length = is.tellg();
		is.clear();
		is.seekg(0, std::ios::beg);
		cyng::buffer_t buffer;
		buffer.reserve(length);
		return { buffer, length };
	}

	std::pair<cyng::buffer_t, std::streamsize> stream_to_buffer(std::istream& is) {

		auto [buffer, length] = prepare_buffer(is);

		auto pos = std::istream_iterator<char>(is);
		auto end = std::istream_iterator<char>();

		buffer.insert(buffer.begin(), pos, end);
		return { buffer, length == 0 ? buffer.size() : length };
	}

	std::string get_extension(std::filesystem::path const& p)
	{
		if (p.empty())
		{
			return "";
		}
		std::string s = p.extension().string();
		return s.substr(1);
	}

}

