#pragma once

#include <functional>
#include "Types.h"

namespace discount
{

using ItemPriceProvider = std::function<ItemPrice(ItemId const&)>;

}
