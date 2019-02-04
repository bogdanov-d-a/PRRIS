#pragma once

#include "OrderTableItemMutator.h"
#include "OrderCalculator.h"

OrderTableItemMutator GetOrderTableItemMutator(IItemAccessor &data);
