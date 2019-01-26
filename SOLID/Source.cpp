#include <array>
#include <iostream>
#include <algorithm>
#include <vector>
#include "OrderCalculatorFactory.h"

using ItemGroupMerger = std::function<void(std::vector<std::reference_wrapper<ItemCount>> const&, ItemCount&)>;

ItemGroupMerger GetItemGroupMerger()
{
	return [](std::vector<std::reference_wrapper<ItemCount>> const& sources, ItemCount &target) {
		ItemCount maxUnion = sources[0].get();
		for (auto &source : sources)
		{
			maxUnion = std::min(maxUnion, source.get());
		}

		for (auto &source : sources)
		{
			source.get() -= maxUnion;
		}
		target += maxUnion;
	};
}

using OrderTableItemMutator = std::function<void(ItemId const& oldId,
	ItemPrice oldPrice, ItemId const& newId, ItemPrice newPrice, ItemCount count)>;

OrderTableItemMutator GetOrderTableItemMutator(IItemAccessor &data)
{
	return [&](ItemId const& oldId, ItemPrice oldPrice,
		ItemId const& newId, ItemPrice newPrice, ItemCount count)
	{
		{
			auto old = data.Find(oldId, oldPrice);
			if (!old)
			{
				throw std::exception();
			}

			auto oldValue = old->GetValue();
			if (oldValue < count)
			{
				throw std::exception();
			}
			else if (oldValue == count)
			{
				data.Remove(oldId, oldPrice);
			}
			else {
				old->SetValue(oldValue - count);
			}
		}

		{
			auto new_ = data.Find(newId, newPrice);
			if (!new_)
			{
				data.Insert(newId, newPrice, 0);
				new_ = data.Find(newId, newPrice);
			}

			new_->SetValue(new_->GetValue() + count);
		}
	};
}

class ResultPrinter : public IResultAcceptor
{
	void OnItem(ItemInfo const & info) final
	{
		std::cout << info.id.GetCharId() << ": " << info.price << " x " << info.count << " = " << info.total << std::endl;
	}

	void OnTotalCost(ItemPrice cost) final
	{
		std::cout << "Total cost: " << cost << std::endl;
	}
};

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
			{ 'A', 20 },
			{ 'B', 40 },
			{ 'C', 30 },
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

	auto itemTransformer = [](IItemAccessor &itemAccessor) {
		auto getItemFirstPriceAndCount = [&](ItemId const& id) {
			std::pair<ItemPrice, ItemCount> result = { UINT_MAX, UINT_MAX };
			itemAccessor.Iterate([&](ItemId const& innerId, ItemPrice price, ItemCount count) {
				if (result.first == UINT_MAX && innerId.GetCharId() == id.GetCharId())
				{
					result = { price, count } ;
				}
			});
			if (result.first == UINT_MAX)
			{
				throw std::exception();
			}
			return result;
		};

		const auto aId = ItemId::CreateFromChar('A');
		auto aPriceCount = getItemFirstPriceAndCount(aId);

		const auto bId = ItemId::CreateFromChar('B');
		auto bPriceCount = getItemFirstPriceAndCount(bId);

		ItemCount abCount = 0;

		auto igm = GetItemGroupMerger();
		igm({ aPriceCount.second, bPriceCount.second }, abCount);

		auto otim = GetOrderTableItemMutator(itemAccessor);
		otim(aId, aPriceCount.first, aId, aPriceCount.first - 5, abCount);
		otim(bId, bPriceCount.first, bId, bPriceCount.first - 5, abCount);
	};

	auto totalCostModifier = [](ItemPrice cost) {
		return cost;
	};

	ResultPrinter resultPrinter;

	auto oc = GetOrderCalculator();
	oc(itemEnumerator, itemPriceProvider, itemTransformer, totalCostModifier, resultPrinter);

	std::cin.get();
	return 0;
}
