#include "WordPump.h"

void PumpWords(WordEnumerator &wordSource, IWordAcceptorEnumerator const& targetEnumerator)
{
	wordSource([&](std::string const& word) {
		targetEnumerator([&](IWordAcceptor &target) {
			target.SendWord(word);
		});
	});

	targetEnumerator([](IWordAcceptor &target) {
		target.Finalize();
	});
}
