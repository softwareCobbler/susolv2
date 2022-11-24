#include <cstdint>
#include <deque>
#include <iostream>
#include <optional>

#include "susolv/board.h"

#define CELL_GROUP_ITERATOR_STATIC_SENTINEL(which)                      \
    template<>                                                          \
    const CellGroupIterator<CellGroupIteratorKind::which>               \
    CellGroupIterator<CellGroupIteratorKind::which>::end_sentinel = {};

CELL_GROUP_ITERATOR_STATIC_SENTINEL(row);
CELL_GROUP_ITERATOR_STATIC_SENTINEL(col);
CELL_GROUP_ITERATOR_STATIC_SENTINEL(quad);
CELL_GROUP_ITERATOR_STATIC_SENTINEL(end_sentinel);

#undef CELL_GROUP_ITERATOR_STATIC_SENTINEL

PossibleSolutionIterator::PossibleSolutionIterator(const Board* board, uint8_t cellIndex) :
    board_(board),
    cellIndex_(cellIndex),
    solutions_(board->availableValuesForCell(cellIndex)),
    totalSolutions_(std::popcount(solutions_)),
    bitIndex_(totalSolutions_ == 0 ? END : std::countr_zero(solutions_))
{}

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

Board loadBoard(const char* fname) {
    Board board;

    FILE* f = fopen(fname, "r");

    if (f == NULL) {
        std::cout << "Can't open " << fname << std::endl;
        std::terminate();
    }

    size_t index = 0;
    char c;

    while ((c = fgetc(f)) != EOF) {
        if (c == '#') {
            do {
                c = fgetc(f);
            } while (c != '\n');
        }
        else if (c == '\n' || c == '\r') {
            continue;
        }
        else {
            uint8_t value = c - '0';
            if (value == 0) {
                board.setUnknown(static_cast<uint8_t>(index));
            }
            else {
                board.setSolved(static_cast<uint8_t>(index), value - 1);
            }
            ++index;
        }

    }

    fclose(f);

    return board;
}

std::optional<Board> solve(const Board& board) {
    std::deque<Board> boards{ board };
    boards.front().fullComputeTakenVals();

    size_t maxSize = 0;
    while (boards.size() > 0) {
        if (boards.size() > maxSize) maxSize = boards.size();

        Board& workingBoard = boards.front();
        Board::SimpleSolveResult result = workingBoard.simpleSolve();

        if (result.solved) {
            // std::cout << "SOLVED" << std::endl;
            // std::cout << "max boards..." << maxSize << std::endl;
            return {workingBoard};
        }
        else if (result.invalid) {
            boards.pop_front();
        }
        else {
            for (auto iter = workingBoard.possibleSolutionsBegin(result.bestIndex); iter != workingBoard.possibleSolutionsEnd(); ++iter) {
                boards.emplace_back(std::move(*iter));
            }
            boards.pop_front();
        }
    }

    return std::nullopt;
}