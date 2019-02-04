#include "OrderCalculatorFactory.h"
#include <vector>
#include <map>

namespace
{

class ItemAccessor : public IItemAccessor
{
public:
	explicit ItemAccessor(std::vector<std::map<ItemPrice, ItemCount>> &table)
		: m_table(table)
	{
	}

	std::unique_ptr<IFindResult> Find(ItemId const & id, ItemPrice price) final
	{
		const auto it = m_table[id.GetIntId()].find(price);
		if (it == m_table[id.GetIntId()].end())
		{
			return std::unique_ptr<IFindResult>();
		}
		return std::make_unique<FoundResult>(m_table[id.GetIntId()], price);
	}

	void Insert(ItemId const & id, ItemPrice price, ItemCount count) final
	{
		m_table[id.GetIntId()][price] = count;
	}

	void Remove(ItemId const & id, ItemPrice price) final
	{
		m_table[id.GetIntId()].erase(price);
	}

	void Iterate(std::function<void(ItemId const& id, ItemPrice price, ItemCount count)> const& cb) const final
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
		explicit FoundResult(std::map<ItemPrice, ItemCount> &map, ItemPrice key)
			: m_map(map)
			, m_key(key)
		{
		}

		const ItemCount GetValue() const final
		{
			return m_map[m_key];
		}

		void SetValue(ItemCount newPrice) final
		{
			m_map[m_key] = newPrice;
		}

	private:
		std::map<ItemPrice, ItemCount> &m_map;
		ItemPrice m_key;
	};

	std::vector<std::map<ItemPrice, ItemCount>> &m_table;
};

}

OrderCalculator GetOrderCalculator()
{
	return [](ItemEnumerator const& itemEnumerator, ItemPriceProvider const& priceProvider, ItemTransformer const& itemTransformer,
		TotalCostModifier const& totalCostModifier, IResultAcceptor &resultAcceptor) {
		std::vector<ItemCount> countTable(ItemId::COUNT, 0);
		itemEnumerator([&](ItemId const& itemId) {
			++countTable[itemId.GetIntId()];
		});

		std::vector<std::map<ItemPrice, ItemCount>> table(ItemId::COUNT);
		for (int itemIdInt = 0; itemIdInt < ItemId::COUNT; ++itemIdInt)
		{
			const auto count = countTable[itemIdInt];
			if (count == 0)
			{
				continue;
			}

			const auto item = ItemId::CreateFromInt(itemIdInt);
			const auto price = priceProvider(item);

			table[itemIdInt][price] += count;
		}

		{
			ItemAccessor ia(table);
			itemTransformer(ia);
		}

		ItemPrice totalCost = 0;
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
