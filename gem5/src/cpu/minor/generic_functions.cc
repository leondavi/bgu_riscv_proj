#include "cpu/minor/generic_functions.hh"

/*
 * Erase First Occurrence of given  substring from main string.
 */
std::string eraseSubStr(std::string mainStr, std::string toErase)
{
	// Search for the substring in string
	size_t pos = mainStr.find(toErase);

	if (pos != std::string::npos)
	{
		// If found then erase it from string
		return mainStr.erase(pos, toErase.length());
	}

	return mainStr;
}
