#include <iostream>
#include <cstdint>

#include "susolv/board.h"

PossibleSolutionIterator::PossibleSolutionIterator(const Board* board, uint8_t cellIndex) :
    board_(board),
    cellIndex_(cellIndex),
    solutions_(board->availableValuesForCell(cellIndex)),
    totalSolutions_(std::popcount(solutions_))
{
    for (uint8_t i = 0; i < 9; ++i) {
        // use countr_zero? in member initializer list?
        if (someBitIsSet(i)) {
            bitIndex_ = i;
            return;
        }
    }
    bitIndex_ = END; // no bits in solutions is set
}

Board PossibleSolutionIterator::operator*() {
    Board result = *board_;
    result.setSolved(cellIndex_, bitIndex_);
    return result;
}

std::ostream& operator<<(std::ostream& out, const Board& board) {
    for (int y = 0; y < 9; ++y) {
        for (int x = 0; x < 9; ++x) {
            uint8_t index = cellIndexLookup.rowElementIndices[y][x];
            bool solved = board.isSolved(index);
            std::string s = solved ? std::to_string(board.getSolvedValue(index)) : "?";
            s = s.size() == 1 ? " " + s : s;
            out << s;
            if (x != 8) {
                out << ", ";
            }
        }
        if (y != 8) {
            out << "\n";
        }
    }
    return out;
}