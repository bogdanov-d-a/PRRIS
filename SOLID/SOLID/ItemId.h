#pragma once

#include <exception>
#include "../libdiscount/Types.h"

class ItemId
{
public:
	static constexpr char MIN_ID = 'A';
	static constexpr char MAX_ID = 'M';
	static constexpr int COUNT = MAX_ID - MIN_ID + 1;

	static ItemId CreateFromChar(char c)
	{
		return ItemId(c);
	}

	static ItemId CreateFromInt(int a)
	{
		return ItemId(a + MIN_ID);
	}

	static ItemId CreateFromDiscountId(discount::ItemId id)
	{
		return CreateFromInt(id);
	}

	const char GetCharId() const
	{
		return m_id;
	}

	const int GetIntId() const
	{
		return GetCharId() - MIN_ID;
	}

	discount::ItemId GetDiscountId() const
	{
		return GetIntId();
	}

private:
	explicit ItemId(char id)
		: m_id(id)
	{
		if (m_id < MIN_ID || m_id > MAX_ID)
		{
			throw std::exception();
		}
	}

	const char m_id;
};
