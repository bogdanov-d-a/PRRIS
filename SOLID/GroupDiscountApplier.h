#pragma once

#include <functional>
#include <set>
#include "Types.h"
#include "ItemPriceProvider.h"
#include "ItemGroupMerger.h"
#include "OrderTableItemMutator.h"

using ItemCountProvider = std::function<ItemCount&(std::set<char> const&)>;
using GroupDiscountApplier = std::function<void(ItemPriceProvider const&,
	ItemCountProvider const&, ItemGroupMerger const&, OrderTableItemMutator const&)>;
