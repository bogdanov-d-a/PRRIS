#pragma once

#include "GroupDiscountApplier.h"
#include "ItemDiscountCalculator.h"

namespace discount
{

GroupDiscountApplier GetGroupDiscountApplier(std::vector<ItemId> const& ids,
	std::set<char> const& keepPrice, ItemDiscountCalculator const& itemDiscountCalculator);

}
