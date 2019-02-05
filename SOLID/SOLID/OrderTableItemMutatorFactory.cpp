#include "OrderTableItemMutatorFactory.h"

discount::OrderTableItemMutator GetOrderTableItemMutator(IItemAccessor &data)
{
	return [&](discount::ItemId const& oldId, discount::ItemPrice oldPrice,
		discount::ItemId const& newId, discount::ItemPrice newPrice, discount::ItemCount count)
	{
		{
			auto oldIdConcrete = ItemId::CreateFromDiscountId(oldId);
			auto old = data.Find(oldIdConcrete, oldPrice);
			if (!old)
			{
				throw std::exception();
			}

			auto oldValue = old->GetValue();
			if (oldValue < count)
			{
				throw std::exception();
			}
			else if (oldValue == count)
			{
				data.Remove(oldIdConcrete, oldPrice);
			}
			else {
				old->SetValue(oldValue - count);
			}
		}

		{
			auto newIdConcrete = ItemId::CreateFromDiscountId(newId);
			auto new_ = data.Find(newIdConcrete, newPrice);
			if (!new_)
			{
				data.Insert(newIdConcrete, newPrice, 0);
				new_ = data.Find(newIdConcrete, newPrice);
			}

			new_->SetValue(new_->GetValue() + count);
		}
	};
}
