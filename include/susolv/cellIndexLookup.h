#ifndef CELL_INDEX_LOOKUP_H
#define CELL_INDEX_LOOKUP_H

#include <cstdint>

class CellIndexLookup {
public:
    uint8_t rowElementIndices[9][9];  // [row][index...]
    uint8_t colElementIndices[9][9];  // [col][index...]
    uint8_t quadElementIndices[9][9]; // [quad][index...]
    uint8_t indexToRow[81];
    uint8_t indexToCol[81];
    uint8_t indexToQuad[81];

    constexpr CellIndexLookup() : rowElementIndices(), colElementIndices(), quadElementIndices(), indexToRow(), indexToCol(), indexToQuad() {
        for (int y = 0; y < 9; ++y) {
            for (int x = 0; x < 9; ++x) {
                const uint8_t index = y * 9 + x;
                rowElementIndices[y][x] = index;
                indexToRow[index] = y;
            }
        }

        for (int x = 0; x < 9; ++x) {
            for (int y = 0; y < 9; ++y) {
                const uint8_t index = y * 9 + x;
                colElementIndices[x][y] = index;
                indexToCol[index] = x;
            }
        }

        for (int quad = 0; quad < 9; ++quad) {
            for (int i = 0; i < 9; ++i) {
                const int y = (quad / 3 * 3) + (i / 3);
                const int x = (quad % 3 * 3) + (i % 3);
                const uint8_t index = rowElementIndices[y][x];
                quadElementIndices[quad][i] = index;
                indexToQuad[index] = quad;
            }
        }
    }
};

inline constexpr CellIndexLookup cellIndexLookup{};

#endif