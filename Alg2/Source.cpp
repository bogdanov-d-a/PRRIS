#include <vector>
#include <memory>
#include <deque>
#include <iostream>
#include <string>

// Author: Bogdanov Dmitry, PSm-22
// Time spent: 68m
// Insert complexity: O(N); N - insert word length
// FindCount complexity: O(N*M); N - average prefix matching word length, M - prefix matching word count
// Memory usage: O(N*M); N - average prefix matching word length, M - prefix matching word count
//
// Words are split into characters and every character is used to create a tree node:
//   first character is appended to root
//   next character is appended to previous character's child
//   ...
//   last character's child is marked as endpoint
//
// Breadth-first search (BFS) algorithm is used to find and count endpoints on prefix subtree

constexpr int CHAR_COUNT = 26;

int CharToInt(char c)
{
	return c - 'a';
}

struct Node
{
public:
	Node()
		: children(CHAR_COUNT)
	{
	}

	std::vector<std::unique_ptr<Node>> children;
	bool endpoint = false;
};

void Insert(Node &root, std::string const& word)
{
	Node *node = &root;
	for (char c : word)
	{
		const int ci = CharToInt(c);
		if (!node->children[ci])
		{
			node->children[ci] = std::make_unique<Node>();
		}
		node = node->children[ci].get();
	}
	node->endpoint = true;
}

int FindCount(Node &root, std::string const& prefix)
{
	Node *node = &root;
	for (char c : prefix)
	{
		const int ci = CharToInt(c);
		if (!node->children[ci])
		{
			return 0;
		}
		node = node->children[ci].get();
	}

	int result = 0;
	std::deque<Node*> nodeQueue;
	nodeQueue.push_back(node);

	while (!nodeQueue.empty())
	{
		const auto curNode = nodeQueue.front();
		nodeQueue.pop_front();

		if (curNode->endpoint)
		{
			++result;
		}

		for (int ci = 0; ci < CHAR_COUNT; ++ci)
		{
			if (curNode->children[ci])
			{
				nodeQueue.push_back(curNode->children[ci].get());
			}
		}
	}

	return result;
}

struct UserCommand
{
	std::string cmd;
	std::string arg;
};

UserCommand ParseUserCommand(std::string const& line)
{
	size_t wsInd = line.find(' ');
	if (wsInd == std::string::npos)
	{
		throw std::exception();
	}
	return { line.substr(0, wsInd), line.substr(wsInd + 1) };
}

int main()
{
	auto tree = std::make_unique<Node>();

	std::string userInput;
	while (getline(std::cin, userInput))
	{
		UserCommand cmd;
		bool cmdParseError = false;
		try
		{
			cmd = ParseUserCommand(userInput);
		}
		catch (std::exception const&)
		{
			cmdParseError = true;
		}

		if (cmdParseError)
		{
			std::cout << "Command parsing error" << std::endl;
			continue;
		}

		if (cmd.cmd == "Add")
		{
			Insert(*tree, cmd.arg);
		}
		else if (cmd.cmd == "Find")
		{
			std::cout << FindCount(*tree, cmd.arg) << std::endl;
		}
		else
		{
			std::cout << "Unknown command " << cmd.cmd << std::endl;
		}
	}

	return 0;
}
