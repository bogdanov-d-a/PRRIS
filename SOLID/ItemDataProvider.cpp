#include "ItemDataProvider.h"
#include <vector>
#include <algorithm>

void EnumerateItems(ItemFunc const& itemFunc)
{
	const std::vector<char> items = { 'A', 'B', 'C', 'B' };

	for (auto &item : items)
	{
		itemFunc(ItemId::CreateFromChar(item));
	}
}

ItemPrice GetItemPrice(ItemId const& itemId)
{
	const std::vector<std::pair<char, int>> itemPrices = { {
		{ 'A', 20 },
		{ 'B', 40 },
		{ 'C', 30 },
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
