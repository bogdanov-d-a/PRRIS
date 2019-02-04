#pragma once

#include <functional>
#include <set>
#include "Types.h"
#include "ItemPriceProvider.h"
#include "ItemGroupMerger.h"
#include "OrderTableItemMutator.h"

namespace discount
{

using ItemCountProvider = std::function<ItemCount&(std::set<ItemId> const&)>;
using BeforeMutatingOrderTable = std::function<void(std::vector<ItemId> const&, ItemCount)>;
using GroupDiscountApplier = std::function<void(ItemPriceProvider const&, ItemCountProvider const&,
	ItemGroupMerger const&, BeforeMutatingOrderTable const&, OrderTableItemMutator const&)>;

}
