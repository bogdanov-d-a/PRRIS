#include <algorithm>
#include <iostream>
#include "WordPump.h"
#include "FileWordsEnumerator.h"

// Author: Bogdanov Dmitry, PSm-22
// Time spent: 99m
// Complexity: O(N); N - input text words count
// Memory usage: O(M+N); M, N - keywords length

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
			if (m_firstDistance != INT_MAX)
			{
				++m_firstDistance;
			}
		}

		if (word == m_second && m_firstDistance != INT_MAX)
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

class MaxSolver : public IWordAcceptor
{
public:
	MaxSolver(std::string const& first, std::string const& second)
		: m_first(first)
		, m_second(second)
	{
	}

	void SendWord(std::string const & word) final
	{
		if (m_firstDistance == INT_MIN)
		{
			if (word == m_first)
			{
				m_firstDistance = 0;
			}
		}
		else
		{
			++m_firstDistance;
			if (word == m_second)
			{
				m_result = m_firstDistance - 1;
			}
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
	int m_result = INT_MIN;
	int m_firstDistance = INT_MIN;
};

struct Solution
{
	int min;
	int max;
};

Solution Solve(WordEnumerator &wordSource, std::string const& first, std::string const& second)
{
	MinSolver minFwd(first, second);
	MinSolver minRev(second, first);
	MaxSolver maxFwd(first, second);
	MaxSolver maxRev(second, first);

	PumpWords(wordSource, [&](IWordAcceptorFunc const& accCb) {
		accCb(minFwd);
		accCb(minRev);
		accCb(maxFwd);
		accCb(maxRev);
	});

	return { std::min(minFwd.GetResult(), minRev.GetResult()),
		std::max(maxFwd.GetResult(), maxRev.GetResult()) };
}

int main()
{
	auto wordEnumerator = GetFileWordsEnumerator("input.txt");
	const auto solution = Solve(wordEnumerator, "Today", "day");
	std::cout << "min distance: " << ((solution.min == INT_MAX) ? "no" : std::to_string(solution.min)) << std::endl;
	std::cout << "max distance: " << ((solution.max == INT_MIN) ? "no" : std::to_string(solution.max)) << std::endl;
	std::cin.get();
	return 0;
}
