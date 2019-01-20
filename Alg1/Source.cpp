#include <algorithm>
#include <iostream>
#include "WordPump.h"
#include "FileWordsEnumerator.h"

class MinSolver : public IWordAcceptor
{
public:
	MinSolver(std::string const& first, std::string const& second)
		: m_first(first)
		, m_second(second)
	{
	}

	void SendWord(std::string const & word) final
	{
		if (word == m_first)
		{
			m_firstDistance = 0;
		}
		else
		{
			++m_firstDistance;
		}

		if (word == m_second)
		{
			m_result = std::min(m_result, m_firstDistance - 1);
		}
	}

	void Finalize() final
	{
	}

	int GetResult() const
	{
		return m_result;
	}

private:
	std::string m_first;
	std::string m_second;
	int m_result = INT_MAX;
	int m_firstDistance = INT_MAX;
};

int SolveMin(WordEnumerator &wordSource, std::string const& first, std::string const& second)
{
	MinSolver ms(first, second);
	PumpWords(wordSource, [&](IWordAcceptorFunc const& accCb) {
		accCb(ms);
	});
	return ms.GetResult();
}

int main()
{
	auto wordEnumerator = GetFileWordsEnumerator("input.txt");
	std::cout << SolveMin(wordEnumerator, "Today", "day") << std::endl;
	std::cin.get();
	return 0;
}
