#pragma once

#include "Types.h"
#include <functional>
#include <vector>

using ItemGroupMerger = std::function<void(std::vector<std::reference_wrapper<ItemCount>> const&, ItemCount&)>;
