#include <iostream>
#include <set>
#include <map>
#include "OrderCalculatorFactory.h"
#include "../libdiscount/ItemGroupMergerFactory.h"
#include "OrderTableItemMutatorFactory.h"
#include "../libdiscount/ItemPercentageDiscountCalculator.h"
#include "../libdiscount/GroupDiscountApplierFactory.h"
#include "ItemDataProvider.h"

namespace
{

class ResultPrinter : public IResultAcceptor
{
	void OnItem(ItemInfo const & info) final
	{
		std::cout << info.id.GetCharId() << ": " << info.price << " x " << info.count << " = " << info.total << std::endl;
	}

	void OnTotalCost(discount::ItemPrice cost) final
	{
		std::cout << "Total cost: " << cost << std::endl;
	}
};

std::ostream& PrintInfo()
{
	return (std::cout << "INFO: ");
}

}

int main()
{
	auto getItemPrice = [](discount::ItemId const& itemId) {
		return GetItemPrice(ItemId::CreateFromDiscountId(itemId));
	};

	discount::ItemCount itemForTotalDiscountCount = 0;

	auto itemTransformer = [&](IItemAccessor &itemAccessor) {
		auto getItemFirstPriceAndCount = [&](ItemId const& id) {
			std::pair<discount::ItemPrice, discount::ItemCount> result = { UINT_MAX, UINT_MAX };
			itemAccessor.Iterate([&](ItemId const& innerId, discount::ItemPrice price, discount::ItemCount count) {
				if (result.first == UINT_MAX && innerId.GetCharId() == id.GetCharId())
				{
					result = { price, count } ;
				}
			});
			return result;
		};

		std::map<std::set<discount::ItemId>, discount::ItemCount> itemCounts;

		auto itemPriceProvider = [&](discount::ItemId const& id) {
			auto price = getItemFirstPriceAndCount(ItemId::CreateFromDiscountId(id)).first;
			if (price == UINT_MAX)
			{
				throw std::exception();
			}
			return price;
		};

		auto itemCountProvider = [&](std::set<discount::ItemId> const& ids) -> discount::ItemCount& {
			auto itemCount = itemCounts.find(ids);
			if (itemCount != itemCounts.end())
			{
				return itemCount->second;
			}

			auto singleCountGetter = [&] {
				auto result = getItemFirstPriceAndCount(ItemId::CreateFromDiscountId(*ids.begin())).second;
				return result == UINT_MAX ? 0 : result;
			};

			itemCounts[ids] = (ids.size() == 1) ? singleCountGetter() : 0;
			return itemCounts[ids];
		};

		auto igm = discount::GetItemGroupMerger();
		auto otim = GetOrderTableItemMutator(itemAccessor);

		const auto discountItemIdFromChar = [](char c) {
			return ItemId::CreateFromChar(c).GetDiscountId();
		};

		std::vector<discount::GroupDiscountApplier> groupDiscountAppliers = {
			discount::GetGroupDiscountApplier({ discountItemIdFromChar('A'), discountItemIdFromChar('B') }, {}, discount::GetItemPercentageDiscountCalculator(10)),
			discount::GetGroupDiscountApplier({ discountItemIdFromChar('D'), discountItemIdFromChar('E') }, {}, discount::GetItemPercentageDiscountCalculator(5)),
			discount::GetGroupDiscountApplier({ discountItemIdFromChar('E'), discountItemIdFromChar('F'), discountItemIdFromChar('G') }, {}, discount::GetItemPercentageDiscountCalculator(5)),
		};

		for (char id : { 'K', 'L', 'M' })
		{
			groupDiscountAppliers.push_back(discount::GetGroupDiscountApplier({ discountItemIdFromChar('A'), discountItemIdFromChar(id) }, { 'A' }, discount::GetItemPercentageDiscountCalculator(5)));
		}

		auto beforeMutatingOrderTable = [&](std::vector<discount::ItemId> const& ids, discount::ItemCount count) {
			auto &info = PrintInfo();
			info << "Merging " << count << " items (";

			bool firstId = true;
			for (auto &id : ids)
			{
				if (!firstId)
				{
					info << ",";
				}
				info << ItemId::CreateFromDiscountId(id).GetCharId();
				firstId = false;
			}

			info << ")" << std::endl;
		};

		for (auto &groupDiscountApplier : groupDiscountAppliers)
		{
			groupDiscountApplier(itemPriceProvider, itemCountProvider, igm, beforeMutatingOrderTable, otim);
		}

		itemAccessor.Iterate([&](ItemId const& id, discount::ItemPrice, discount::ItemCount count) {
			if (id.GetCharId() != 'A' && id.GetCharId() != 'C')
			{
				itemForTotalDiscountCount += count;
			}
		});
	};

	auto totalCostModifier = [&](discount::ItemPrice cost) {
		discount::ItemDiscountCalculator idc;

		auto &info = PrintInfo();
		info << "cost = " << cost << ", items = " << itemForTotalDiscountCount << ", discount = ";

		if (itemForTotalDiscountCount >= 5)
		{
			idc = discount::GetItemPercentageDiscountCalculator(20);
			info << 20;
		}
		else if (itemForTotalDiscountCount >= 4)
		{
			idc = discount::GetItemPercentageDiscountCalculator(10);
			info << 10;
		}
		else if (itemForTotalDiscountCount >= 3)
		{
			idc = discount::GetItemPercentageDiscountCalculator(5);
			info << 5;
		}
		else
		{
			idc = [](discount::ItemPrice price) {
				return price;
			};
			info << 0;
		}

		auto newCost = idc(cost);
		info << ", new cost = " << newCost << std::endl;

		return newCost;
	};

	ResultPrinter resultPrinter;

	auto oc = GetOrderCalculator();
	oc(EnumerateItems, getItemPrice, itemTransformer, totalCostModifier, resultPrinter);

	std::cin.get();
	return 0;
}
