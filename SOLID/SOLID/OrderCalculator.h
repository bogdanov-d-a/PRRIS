#pragma once

#include "ItemPriceProvider.h"
#include <memory>

using ItemFunc = std::function<void(ItemId const&)>;
using ItemEnumerator = std::function<void(ItemFunc const&)>;

struct ItemInfo
{
	ItemId id;
	ItemPrice price;
	ItemCount count;
	ItemPrice total;
};

class IItemAccessor
{
public:
	class IFindResult
	{
	public:
		virtual ~IFindResult() = default;
		virtual const ItemCount GetValue() const = 0;
		virtual void SetValue(ItemCount newPrice) = 0;
	};

	virtual ~IItemAccessor() = default;
	virtual std::unique_ptr<IFindResult> Find(ItemId const& id, ItemPrice price) = 0;
	virtual void Insert(ItemId const& id, ItemPrice price, ItemCount count) = 0;
	virtual void Remove(ItemId const& id, ItemPrice price) = 0;
	virtual void Iterate(std::function<void(ItemId const& id, ItemPrice price, ItemCount count)> const& cb) const = 0;
};

using ItemTransformer = std::function<void(IItemAccessor &itemAccessor)>;

using TotalCostModifier = std::function<ItemPrice(ItemPrice cost)>;

class IResultAcceptor
{
public:
	virtual ~IResultAcceptor() = default;
	virtual void OnItem(ItemInfo const& item) = 0;
	virtual void OnTotalCost(ItemPrice cost) = 0;
};

using OrderCalculator = std::function<void(ItemEnumerator const&, ItemPriceProvider const&,
	ItemTransformer const&, TotalCostModifier const&, IResultAcceptor&)>;