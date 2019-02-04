#include "OrderTableItemMutatorFactory.h"

OrderTableItemMutator GetOrderTableItemMutator(IItemAccessor &data)
{
	return [&](ItemId const& oldId, ItemPrice oldPrice,
		ItemId const& newId, ItemPrice newPrice, ItemCount count)
	{
		{
			auto old = data.Find(oldId, oldPrice);
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
				data.Remove(oldId, oldPrice);
			}
			else {
				old->SetValue(oldValue - count);
			}
		}

		{
			auto new_ = data.Find(newId, newPrice);
			if (!new_)
			{
				data.Insert(newId, newPrice, 0);
				new_ = data.Find(newId, newPrice);
			}

			new_->SetValue(new_->GetValue() + count);
		}
	};
}
