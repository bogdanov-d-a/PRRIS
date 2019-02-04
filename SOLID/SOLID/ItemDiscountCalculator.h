#pragma once

#include <functional>
#include "Types.h"

using ItemDiscountCalculator = std::function<ItemPrice(ItemPrice)>;
