#include <array>
#include <vector>
#include <map>
#include <iostream>
#include <algorithm>
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

			table[itemIdInt][price] += count;
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

	auto resultPrinter = [](ItemInfo const& info) {
		std::cout << info.id.GetCharId() << ": " << info.price << " x " << info.count << " = " << info.total << std::endl;
	};

	auto oc = GetOrderCalculator();
	oc(itemEnumerator, itemPriceProvider, itemTransformer, resultPrinter);

	std::cin.get();
	return 0;
}
