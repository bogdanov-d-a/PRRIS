#pragma once

#include <functional>
#include "Types.h"

namespace discount
{

using ItemDiscountCalculator = std::function<ItemPrice(ItemPrice)>;

}
