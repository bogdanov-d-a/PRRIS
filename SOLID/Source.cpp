#include <array>
#include <vector>
#include <map>
#include <iostream>
#include "OrderCalculator.h"

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

OrderCalculator GetOrderCalculator()
{
	return [](ItemEnumerator const& itemEnumerator, ItemPriceProvider const& priceProvider,
			ItemTransformer const& itemTransformer, ResultAcceptor const& resultAcceptor) {
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

			++table[itemIdInt][price];
		}

		{
			ItemAccessor ia(table);
			itemTransformer(ia);
		}

		for (int itemIdInt = 0; itemIdInt < ItemId::COUNT; ++itemIdInt)
		{
			const auto item = ItemId::CreateFromInt(itemIdInt);

			for (auto &pair : table[itemIdInt])
			{
				const auto price = pair.first;
				const auto count = pair.second;
				resultAcceptor({ item, price, count, price * count });
			}
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

	auto itemTransformer = [](IItemAccessor &itemAccessor) {
		auto it = itemAccessor.Find(ItemId::CreateFromChar('B'), 4);
		it->SetValue(42);

		itemAccessor.Insert(ItemId::CreateFromChar('H'), 666, 13);
		itemAccessor.Insert(ItemId::CreateFromChar('A'), 1, 99);
	};

	auto resultPrinter = [](ItemInfo const& info) {
		std::cout << info.id.GetCharId() << ": " << info.price << " x " << info.count << " = " << info.total << std::endl;
	};

	auto oc = GetOrderCalculator();
	oc(itemEnumerator, itemPriceProvider, itemTransformer, resultPrinter);

	std::cin.get();
	return 0;
}
