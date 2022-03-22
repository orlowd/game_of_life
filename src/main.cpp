#include <cstdlib>
#include "options.h"
#include "utility.h"

int main(int argc, char** argv)
{
	const auto options = parseOptions(argc, argv);
	runGame(options);

	return EXIT_SUCCESS;
}
