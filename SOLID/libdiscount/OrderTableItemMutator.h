#pragma once

#include "Types.h"
#include <functional>

using OrderTableItemMutator = std::function<void(ItemId const& oldId,
	ItemPrice oldPrice, ItemId const& newId, ItemPrice newPrice, ItemCount count)>;
