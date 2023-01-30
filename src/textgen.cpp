#include "Textgen.hpp"
#include "Values.hpp"
#include "Words.hpp"

namespace BirbType
{
	std::string GenerateLine(int length)
	{
		std::string result_string;
		while (static_cast<int>(result_string.size()) < length)
		{
			result_string += (words[Birb::Global::random.RandomInt(0, words.size() - 1)]) + " ";
		}

		return result_string;
	}
}
