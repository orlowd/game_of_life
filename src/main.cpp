#include "options.h"
#include "utility.h"
#include <cstdlib>

int main(int argc, char** argv) {
    const auto options = parseOptions(argc, argv);
    runGame(options);

    return EXIT_SUCCESS;
}
