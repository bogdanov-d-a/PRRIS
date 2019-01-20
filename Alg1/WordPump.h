#pragma once

#include "WordEnumerator.h"
#include "IWordAcceptor.h"

using IWordAcceptorFunc = std::function<void(IWordAcceptor&)>;
using IWordAcceptorEnumerator = std::function<void(IWordAcceptorFunc const&)>;

void PumpWords(WordEnumerator &wordSource, IWordAcceptorEnumerator const& targetEnumerator);
