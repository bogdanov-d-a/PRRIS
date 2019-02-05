#include "OrderCalculatorFactory.h"
#include <vector>
#include <map>

namespace
{

class ItemAccessor : public IItemAccessor
{
public:
	explicit ItemAccessor(std::vector<std::map<discount::ItemPrice, discount::ItemCount>> &table)
		: m_table(table)
	{
	}

	std::unique_ptr<IFindResult> Find(ItemId const & id, discount::ItemPrice price) final
	{
		const auto it = m_table[id.GetIntId()].find(price);
		if (it == m_table[id.GetIntId()].end())
		{
			return std::unique_ptr<IFindResult>();
		}
		return std::make_unique<FoundResult>(m_table[id.GetIntId()], price);
	}

	void Insert(ItemId const & id, discount::ItemPrice price, discount::ItemCount count) final
	{
		m_table[id.GetIntId()][price] = count;
	}

	void Remove(ItemId const & id, discount::ItemPrice price) final
	{
		m_table[id.GetIntId()].erase(price);
	}

	void Iterate(std::function<void(ItemId const& id, discount::ItemPrice price, discount::ItemCount count)> const& cb) const final
	{
		for (int itemIdInt = 0; itemIdInt < ItemId::COUNT; ++itemIdInt)
		{
			for (auto &pair : m_table[itemIdInt])
			{
				cb(ItemId::CreateFromInt(itemIdInt), pair.first, pair.second);
			}
		}
	}

private:
	class FoundResult : public IFindResult
	{
	public:
		explicit FoundResult(std::map<discount::ItemPrice, discount::ItemCount> &map, discount::ItemPrice key)
			: m_map(map)
			, m_key(key)
		{
		}

		const discount::ItemCount GetValue() const final
		{
			return m_map[m_key];
		}

		void SetValue(discount::ItemCount newPrice) final
		{
			m_map[m_key] = newPrice;
		}

	private:
		std::map<discount::ItemPrice, discount::ItemCount> &m_map;
		discount::ItemPrice m_key;
	};

	std::vector<std::map<discount::ItemPrice, discount::ItemCount>> &m_table;
};

}

OrderCalculator GetOrderCalculator()
{
	return [](ItemEnumerator const& itemEnumerator, discount::ItemPriceProvider const& priceProvider, ItemTransformer const& itemTransformer,
		TotalCostModifier const& totalCostModifier, IResultAcceptor &resultAcceptor) {
		std::vector<discount::ItemCount> countTable(ItemId::COUNT, 0);
		itemEnumerator([&](ItemId const& itemId) {
			++countTable[itemId.GetIntId()];
		});

		std::vector<std::map<discount::ItemPrice, discount::ItemCount>> table(ItemId::COUNT);
		for (int itemIdInt = 0; itemIdInt < ItemId::COUNT; ++itemIdInt)
		{
			const auto count = countTable[itemIdInt];
			if (count == 0)
			{
				continue;
			}

			const auto item = ItemId::CreateFromInt(itemIdInt);
			const auto price = priceProvider(item.GetDiscountId());

			table[itemIdInt][price] += count;
		}

		{
			ItemAccessor ia(table);
			itemTransformer(ia);
		}

		discount::ItemPrice totalCost = 0;
		for (int itemIdInt = 0; itemIdInt < ItemId::COUNT; ++itemIdInt)
		{
			const auto item = ItemId::CreateFromInt(itemIdInt);

			for (auto &pair : table[itemIdInt])
			{
				const auto price = pair.first;
				const auto count = pair.second;
				const auto cost = price * count;
				resultAcceptor.OnItem({ item, price, count, cost });
				totalCost += cost;
			}
		}
		resultAcceptor.OnTotalCost(totalCostModifier(totalCost));
	};
}
