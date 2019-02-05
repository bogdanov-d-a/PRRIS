#include "ItemDataProvider.h"
#include <vector>
#include <algorithm>

void EnumerateItems(ItemFunc const& itemFunc)
{
	const std::vector<char> items = { 'A', 'B', 'C', 'B', 'A', 'D', 'D', 'E', 'E', 'E', 'F', 'G', 'K', 'L', 'M', 'A', 'A' };

	for (auto &item : items)
	{
		itemFunc(ItemId::CreateFromChar(item));
	}
}

discount::ItemPrice GetItemPrice(ItemId const& itemId)
{
	const std::vector<std::pair<char, int>> itemPrices = { {
		{ 'A', 200 },
		{ 'B', 400 },
		{ 'C', 600 },
		{ 'D', 350 },
		{ 'E', 450 },
		{ 'F', 150 },
		{ 'G', 650 },
		{ 'H', 250 },
		{ 'I', 50 },
		{ 'J', 100 },
		{ 'K', 500 },
		{ 'L', 300 },
		{ 'M', 550 },
	} };

	const auto result = find_if(itemPrices.begin(), itemPrices.end(), [&](std::pair<char, int> const& item) {
		return item.first == itemId.GetCharId();
	});

	if (result == itemPrices.end())
	{
		throw std::exception();
	}

	return result->second;
}
