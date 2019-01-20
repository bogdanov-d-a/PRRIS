#pragma once

#include <string>

class IWordAcceptor
{
public:
	virtual ~IWordAcceptor() = default;
	virtual void SendWord(std::string const& word) = 0;
	virtual void Finalize() = 0;
};
