#pragma once

#include "ItemPriceProvider.h"

using ItemFunc = std::function<void(ItemId const&)>;
using ItemEnumerator = std::function<void(ItemFunc const&)>;

struct ItemInfo
{
	ItemId id;
	ItemPrice price;
	ItemCount count;
	ItemPrice total;
};

using ResultAcceptor = std::function<void(ItemInfo const&)>;

using OrderCalculator = std::function<void(ItemEnumerator const&, ItemPriceProvider const&, ResultAcceptor const&)>;
