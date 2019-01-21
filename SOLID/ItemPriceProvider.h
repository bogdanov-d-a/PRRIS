#pragma once

#include <functional>
#include "Types.h"
#include "ItemId.h"

using ItemPriceProvider = std::function<ItemPrice(ItemId const&)>;
