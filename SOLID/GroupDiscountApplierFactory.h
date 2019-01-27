#pragma once

#include "GroupDiscountApplier.h"
#include "ItemDiscountCalculator.h"

GroupDiscountApplier GetGroupDiscountApplier(std::vector<ItemId> const& ids,
	std::set<char> const& keepPrice, ItemDiscountCalculator const& itemDiscountCalculator);
