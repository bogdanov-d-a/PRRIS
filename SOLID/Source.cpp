#include <array>
#include <vector>
#include <iostream>
#include "OrderCalculator.h"

OrderCalculator GetOrderCalculator()
{
	return [](ItemEnumerator const& itemEnumerator, ItemPriceProvider const& priceProvider, ResultAcceptor const& resultAcceptor) {
		std::vector<ItemCount> countTable(ItemId::COUNT, 0);
		itemEnumerator([&](ItemId const& itemId) {
			++countTable[itemId.GetIntId()];
		});

		for (int itemIdInt = 0; itemIdInt < ItemId::COUNT; ++itemIdInt)
		{
			const auto count = countTable[itemIdInt];
			if (count == 0)
			{
				continue;
			}

			const auto item = ItemId::CreateFromInt(itemIdInt);
			const auto price = priceProvider(item);
			resultAcceptor({ item, price, count, price * count });
		}
	};
}

int main()
{
	auto itemEnumerator = [](ItemFunc const& itemFunc) {
		constexpr std::array<char, 4> items = {{ 'A', 'B', 'C', 'B' }};

		for (auto &item : items)
		{
			itemFunc(ItemId::CreateFromChar(item));
		}
	};

	auto itemPriceProvider = [](ItemId const& itemId) {
		constexpr std::array<std::pair<char, int>, 3> itemPrices = {{
			{ 'A', 2 },
			{ 'B', 4 },
			{ 'C', 3 },
		}};

		const auto result = find_if(itemPrices.begin(), itemPrices.end(), [&](std::pair<char, int> const& item) {
			return item.first == itemId.GetCharId();
		});

		if (result == itemPrices.end())
		{
			throw std::exception();
		}

		return result->second;
	};

	auto resultPrinter = [](ItemInfo const& info) {
		std::cout << info.id.GetCharId() << ": " << info.price << " x " << info.count << " = " << info.total << std::endl;
	};

	auto oc = GetOrderCalculator();
	oc(itemEnumerator, itemPriceProvider, resultPrinter);

	std::cin.get();
	return 0;
}
