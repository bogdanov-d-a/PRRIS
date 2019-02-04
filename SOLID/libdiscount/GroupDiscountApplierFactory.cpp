#include "stdafx.h"
#include "GroupDiscountApplierFactory.h"

namespace discount
{

GroupDiscountApplier GetGroupDiscountApplier(std::vector<ItemId> const& ids,
	std::set<char> const& keepPrice, ItemDiscountCalculator const& itemDiscountCalculator)
{
	return [=](ItemPriceProvider const& itemPriceProvider, ItemCountProvider const& itemCountProvider,
			ItemGroupMerger const& itemGroupMerger, BeforeMutatingOrderTable const& beforeMutatingOrderTable,
			OrderTableItemMutator const& orderTableItemMutator) {
		std::set<ItemId> group;
		std::vector<std::reference_wrapper<ItemCount>> itemCounts;

		for (auto &id : ids)
		{
			group.insert(id);
			itemCounts.push_back(itemCountProvider({ id }));
		}

		auto& groupCount = itemCountProvider(group);
		itemGroupMerger(itemCounts, groupCount);

		if (groupCount == 0)
		{
			return;
		}

		beforeMutatingOrderTable(ids, groupCount);

		for (auto &id : ids)
		{
			if (keepPrice.find(id) != keepPrice.end())
			{
				continue;
			}

			const auto price = itemPriceProvider(id);
			orderTableItemMutator(id, price, id, itemDiscountCalculator(price), groupCount);
		}
	};
}

}
