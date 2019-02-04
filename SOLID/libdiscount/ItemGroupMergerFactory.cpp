#include "stdafx.h"
#include "ItemGroupMergerFactory.h"
#include <algorithm>

ItemGroupMerger GetItemGroupMerger()
{
	return [](std::vector<std::reference_wrapper<ItemCount>> const& sources, ItemCount &target) {
		ItemCount maxUnion = sources[0].get();
		for (auto &source : sources)
		{
			maxUnion = std::min(maxUnion, source.get());
		}

		for (auto &source : sources)
		{
			source.get() -= maxUnion;
		}
		target += maxUnion;
	};
}
