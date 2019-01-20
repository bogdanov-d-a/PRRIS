#include "FileWordsEnumerator.h"
#include <fstream>
#include <string>

WordEnumerator GetFileWordsEnumerator(std::string const& fileName)
{
	return [fileName](StringFunc const& wordCb) {
		std::ifstream file(fileName);
		std::string word;
		while (file >> word)
		{
			wordCb(word);
		}
	};
}
