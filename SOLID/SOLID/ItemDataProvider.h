#pragma once

#include "OrderCalculator.h"

void EnumerateItems(ItemFunc const& itemFunc);
discount::ItemPrice GetItemPrice(ItemId const& itemId);
