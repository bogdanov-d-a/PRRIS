#include "GroupDiscountApplierFactory.h"

GroupDiscountApplier GetGroupDiscountApplier(std::vector<ItemId> const& ids,
	std::set<char> const& keepPrice, ItemDiscountCalculator const& itemDiscountCalculator)
{
	return [=](ItemPriceProvider const& itemPriceProvider, ItemCountProvider const& itemCountProvider,
		ItemGroupMerger const& itemGroupMerger, OrderTableItemMutator const& orderTableItemMutator) {
		std::set<char> group;
		std::vector<std::reference_wrapper<ItemCount>> itemCounts;

		for (auto &id : ids)
		{
			group.insert(id.GetCharId());
			itemCounts.push_back(itemCountProvider({ id.GetCharId() }));
		}

		auto& groupCount = itemCountProvider(group);
		itemGroupMerger(itemCounts, groupCount);

		if (groupCount == 0)
		{
			return;
		}

		for (auto &id : ids)
		{
			if (keepPrice.find(id.GetCharId()) != keepPrice.end())
			{
				continue;
			}

			const auto price = itemPriceProvider(id);
			orderTableItemMutator(id, price, id, itemDiscountCalculator(price), groupCount);
		}
	};
}
