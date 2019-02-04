#pragma once

#include <functional>
#include "Types.h"

using ItemPriceProvider = std::function<ItemPrice(ItemId const&)>;
