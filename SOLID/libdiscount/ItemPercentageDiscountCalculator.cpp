#include "stdafx.h"
#include "ItemPercentageDiscountCalculator.h"

ItemDiscountCalculator GetItemPercentageDiscountCalculator(ItemPrice percentage)
{
	return [=](ItemPrice price) {
		return price * (100 - percentage) / 100;
	};
}
