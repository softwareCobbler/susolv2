#include <vector>

#include "susolv/board.h"

/**
 * loads the euler96 puzzle, which is like
 * /(^Grid.*\n(^\d{9}\n){9}){50}/
 * or thereabout
 */
std::vector<Board> loadEuler96(const char* fname) {
    FILE* f = fopen(fname, "r");

    if (f == NULL) {
        std::cout << "Can't open " << fname << std::endl;
        std::terminate();
    }

    size_t cellIndex = 0;
    char c;

    std::vector<Board> result;
    result.reserve(50);

    while ((c = fgetc(f)) != EOF) {
        if (c == 'G') { // "^G.*$" uniquely starts a grid on the next line
            do {
                c = fgetc(f);
            } while (c != '\n');
            result.emplace_back();
            cellIndex = 0;
        }
        else if (c == '\n' || c == '\r') {
            continue;
        }
        else {
            uint8_t value = c - '0';
            if (value == 0) {
                result.back().setUnknown(static_cast<uint8_t>(cellIndex));
            }
            else {
                result.back().setSolved(static_cast<uint8_t>(cellIndex), value - 1);
            }
            ++cellIndex;
        }

    }

    fclose(f);

    return result;
}
