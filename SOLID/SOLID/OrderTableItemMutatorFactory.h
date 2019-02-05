#pragma once

#include "../libdiscount/OrderTableItemMutator.h"
#include "OrderCalculator.h"

discount::OrderTableItemMutator GetOrderTableItemMutator(IItemAccessor &data);
