#include <array>
#include <iostream>
#include <set>
#include <map>
#include "OrderCalculatorFactory.h"
#include "ItemGroupMergerFactory.h"
#include "OrderTableItemMutatorFactory.h"
#include "ItemPercentageDiscountCalculator.h"
#include "GroupDiscountApplierFactory.h"

namespace
{

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

std::ostream& PrintInfo()
{
	return (std::cout << "INFO: ");
}

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

	ItemCount itemForTotalDiscountCount = 0;

	auto itemTransformer = [&](IItemAccessor &itemAccessor) {
		auto getItemFirstPriceAndCount = [&](ItemId const& id) {
			std::pair<ItemPrice, ItemCount> result = { UINT_MAX, UINT_MAX };
			itemAccessor.Iterate([&](ItemId const& innerId, ItemPrice price, ItemCount count) {
				if (result.first == UINT_MAX && innerId.GetCharId() == id.GetCharId())
				{
					result = { price, count } ;
				}
			});
			return result;
		};

		std::map<std::set<char>, ItemCount> itemCounts;

		auto itemPriceProvider = [&](ItemId const& id) {
			auto price = getItemFirstPriceAndCount(id).first;
			if (price == UINT_MAX)
			{
				throw std::exception();
			}
			return price;
		};

		auto itemCountProvider = [&](std::set<char> const& ids) -> ItemCount& {
			auto itemCount = itemCounts.find(ids);
			if (itemCount != itemCounts.end())
			{
				return itemCount->second;
			}

			auto singleCountGetter = [&] {
				auto result = getItemFirstPriceAndCount(ItemId::CreateFromChar(*ids.begin())).second;
				return result == UINT_MAX ? 0 : result;
			};

			itemCounts[ids] = (ids.size() == 1) ? singleCountGetter() : 0;
			return itemCounts[ids];
		};

		auto igm = GetItemGroupMerger();
		auto otim = GetOrderTableItemMutator(itemAccessor);

		std::vector<GroupDiscountApplier> groupDiscountAppliers = {
			GetGroupDiscountApplier({ ItemId::CreateFromChar('A'), ItemId::CreateFromChar('B') }, {}, GetItemPercentageDiscountCalculator(10)),
			GetGroupDiscountApplier({ ItemId::CreateFromChar('D'), ItemId::CreateFromChar('E') }, {}, GetItemPercentageDiscountCalculator(5)),
			GetGroupDiscountApplier({ ItemId::CreateFromChar('E'), ItemId::CreateFromChar('F'), ItemId::CreateFromChar('G') }, {}, GetItemPercentageDiscountCalculator(5)),
		};

		for (char id : { 'K', 'L', 'M' })
		{
			groupDiscountAppliers.push_back(GetGroupDiscountApplier({ ItemId::CreateFromChar('A'), ItemId::CreateFromChar(id) }, { 'A' }, GetItemPercentageDiscountCalculator(5)));
		}

		auto beforeMutatingOrderTable = [&](std::vector<ItemId> const& ids, ItemCount count) {
			auto &info = PrintInfo();
			info << "Merging " << count << " items (";

			bool firstId = true;
			for (auto &id : ids)
			{
				if (!firstId)
				{
					info << ",";
				}
				info << id.GetCharId();
				firstId = false;
			}

			info << ")" << std::endl;
		};

		for (auto &groupDiscountApplier : groupDiscountAppliers)
		{
			groupDiscountApplier(itemPriceProvider, itemCountProvider, igm, beforeMutatingOrderTable, otim);
		}

		itemAccessor.Iterate([&](ItemId const& id, ItemPrice, ItemCount count) {
			if (id.GetCharId() != 'A' && id.GetCharId() != 'C')
			{
				itemForTotalDiscountCount += count;
			}
		});
	};

	auto totalCostModifier = [&](ItemPrice cost) {
		ItemDiscountCalculator idc;

		auto &info = PrintInfo();
		info << "cost = " << cost << ", items = " << itemForTotalDiscountCount << ", discount = ";

		if (itemForTotalDiscountCount >= 5)
		{
			idc = GetItemPercentageDiscountCalculator(20);
			info << 20;
		}
		else if (itemForTotalDiscountCount >= 4)
		{
			idc = GetItemPercentageDiscountCalculator(10);
			info << 10;
		}
		else if (itemForTotalDiscountCount >= 3)
		{
			idc = GetItemPercentageDiscountCalculator(5);
			info << 5;
		}
		else
		{
			idc = [](ItemPrice price) {
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
	oc(itemEnumerator, itemPriceProvider, itemTransformer, totalCostModifier, resultPrinter);

	std::cin.get();
	return 0;
}
