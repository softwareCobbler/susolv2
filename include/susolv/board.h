#ifndef BOARD_H
#define BOARD_H

#include <cstdint>
#include <iostream>
#include <string>

#include "susolv/cellIndexLookup.h"

enum class CellGroupIteratorKind { row, col, quad, end_sentinel };

template<CellGroupIteratorKind kind>
class CellGroupIterator {
private:

    uint16_t* const board_cells;
    const uint8_t base;
    uint8_t index = 0;

    // default constructor is only used to init the "end sentinel" for each templated type
    CellGroupIterator() : board_cells(nullptr), base(0), index(9) {}
    static const CellGroupIterator<kind> end_sentinel;

public:
    CellGroupIterator(uint16_t* _board_cells, uint8_t _base) : board_cells(_board_cells), base(_base) {}

    bool operator==(const CellGroupIterator& r) const {
        return index == r.index;
    }

    CellGroupIterator<kind>& operator ++() {
        index += 1;
        return *this;
    }

    uint16_t* operator*() {
        if constexpr (kind == CellGroupIteratorKind::row) {
            return &board_cells[cellIndexLookup.rowElementIndices[base][index]];
        }
        else if constexpr (kind == CellGroupIteratorKind::col) {
            return &board_cells[cellIndexLookup.colElementIndices[base][index]];
        }
        else if constexpr (kind == CellGroupIteratorKind::quad) {
            return &board_cells[cellIndexLookup.quadElementIndices[base][index]];
        }
        else {
            static_assert(static_cast<bool>(kind) && false, "Unhandled CellGroupIteratorKind");
        }
    }

    static CellGroupIterator<kind> end() {
        return end_sentinel;
    }
};

#define CELL_GROUP_ITERATOR_STATIC_SENTINEL(which)                      \
    const CellGroupIterator<CellGroupIteratorKind::which>               \
    CellGroupIterator<CellGroupIteratorKind::which>::end_sentinel{};

CELL_GROUP_ITERATOR_STATIC_SENTINEL(row);
CELL_GROUP_ITERATOR_STATIC_SENTINEL(col);
CELL_GROUP_ITERATOR_STATIC_SENTINEL(quad);
CELL_GROUP_ITERATOR_STATIC_SENTINEL(end_sentinel);

#undef CELL_GROUP_ITERATOR_STATIC_SENTINEL

class Board;

class PossibleSolutionIterator {
private:
    friend Board;

    const Board* board_;
    const uint8_t cellIndex_;
    const uint16_t solutions_;
    const uint8_t totalSolutions_;
    uint8_t bitIndex_ = 0;

    static constexpr uint8_t END = 0xFF;

    bool someBitIsSet(uint8_t index) const {
        return static_cast<bool>(solutions_ & (1 << index));
    }

    bool currentBitIsSet() const {
        return someBitIsSet(bitIndex_);
    }
    
    PossibleSolutionIterator() :
        board_(nullptr),
        cellIndex_(0),
        solutions_(0),
        totalSolutions_(0)
    {
        bitIndex_ = END;
    }

public:
    PossibleSolutionIterator(const Board* board, uint8_t cellIndex);

    bool operator==(const PossibleSolutionIterator& rhs) const {
        return bitIndex_ == rhs.bitIndex_;
    }

    Board operator*();

    PossibleSolutionIterator& operator++() {
        if (bitIndex_ == END) {
            return *this;
        }

        // bitIndex_ in range [0,8] for 9 possible values
        while (++bitIndex_ < 9) {
            if (currentBitIsSet()) {
                break;
            }
        }

        if (bitIndex_ == 9) {
            bitIndex_ = END;
        }

        return *this;
    }
};

class alignas(256) Board {
    friend PossibleSolutionIterator;

public:

    uint16_t cells[81];

    static constexpr uint16_t SOLVED_FLAG     = 0b1000'0000'0000'0000;
    static constexpr uint16_t ALL_VALUES_MASK = 0b0000'0001'1111'1111;
    static constexpr uint16_t TAKEN_INIT      = 0b1111'1110'0000'0000;

    struct PossibleValues {
        uint16_t row[9];
        uint16_t col[9];
        uint16_t quad[9];
    };

    static inline thread_local PossibleValues takenValues{};

    Board() = default;
    Board(const Board& rhs) = default;
    Board(Board&& rhs) = default;

    friend std::ostream& operator<<(std::ostream& out, const Board& board);

    // generally don't need to pay for this at construction,
    // except maybe in a test
    static Board ZeroedBoard() {
        Board board;
        std::fill(board.cells, board.cells+81, 0);
        return board;
    }

    CellGroupIterator<CellGroupIteratorKind::row> rowBegin(uint8_t y) {
        return CellGroupIterator<CellGroupIteratorKind::row>(cells, y);
    }

    CellGroupIterator<CellGroupIteratorKind::row> rowEnd() {
        return CellGroupIterator<CellGroupIteratorKind::row>::end();
    }

    CellGroupIterator<CellGroupIteratorKind::col> colBegin(uint8_t x) {
        return CellGroupIterator<CellGroupIteratorKind::col>(cells, x);
    }

    CellGroupIterator<CellGroupIteratorKind::col> colEnd() {
        return CellGroupIterator<CellGroupIteratorKind::col>::end();
    }

    /**
    * 0-indexed
    * 
    *  0 | 1 | 2
    * ---+---+---
    *  3 | 4 | 5
    * ---+---+---
    *  6 | 7 | 8
    * 
    */
    CellGroupIterator<CellGroupIteratorKind::quad> quadBegin(uint8_t quad) {
        return CellGroupIterator<CellGroupIteratorKind::quad>(cells, quad);
    }

    CellGroupIterator<CellGroupIteratorKind::quad> quadEnd() {
        return CellGroupIterator<CellGroupIteratorKind::quad>::end();
    }

private:
    uint16_t unionTakenValues(uint8_t row, uint8_t col, uint8_t quad) const noexcept {
        return takenValues.row[row]
            | takenValues.col[col]
            | takenValues.quad[quad];
    }
    
public:
    void fullComputeTakenVals() noexcept {
        for (int rowIndex = 0; rowIndex < 9; ++rowIndex) {
            uint16_t taken = TAKEN_INIT;
            for (auto rowIter = rowBegin(rowIndex); rowIter != rowEnd(); ++rowIter) {
                if (isSolved(*rowIter)) {
                    taken |= **rowIter & ALL_VALUES_MASK;
                }
            }
            takenValues.row[rowIndex] = taken;
        }

        for (int colIndex = 0; colIndex < 9; ++colIndex) {
            uint16_t taken = TAKEN_INIT;
            for (auto colIter = colBegin(colIndex); colIter != colEnd(); ++colIter) {
                if (isSolved(*colIter)) {
                    taken |= **colIter & ALL_VALUES_MASK;
                }
            }
            takenValues.col[colIndex] = taken;
        }

        for (int quadIndex = 0; quadIndex < 9; ++quadIndex) {
            uint16_t taken = TAKEN_INIT;
            for (auto quadIter = quadBegin(quadIndex); quadIter != quadEnd(); ++quadIter) {
                if (isSolved(*quadIter)) {
                    taken |= **quadIter & ALL_VALUES_MASK;
                }
            }
            takenValues.quad[quadIndex] = taken;
        }
    }

    struct SimpleSolveResult {
        uint8_t bestIndex = 0xFF;
        uint8_t bitCount = 0xFF;
        bool invalid = false;
        bool solved = false;
    };

private:
    uint16_t availableValuesForCell(uint8_t index) const {
        const uint8_t row = cellIndexLookup.indexToRow[index];
        const uint8_t col = cellIndexLookup.indexToCol[index];
        const uint8_t quad = cellIndexLookup.indexToQuad[index];
        const uint16_t takenUnion = unionTakenValues(row, col, quad);
        const uint16_t available = ~takenUnion;
        return available;
    }

public:
    SimpleSolveResult simpleSolve() noexcept {
        uint8_t solvedCount;
        SimpleSolveResult result;
        bool didChange = false;

        while (true) {
            fullComputeTakenVals();

            solvedCount = 0;
            result = {};

            for (uint8_t index = 0; index < 81; ++index) {
                if (isSolved(index)) {
                    ++solvedCount;
                    continue;
                }

                const uint16_t available = availableValuesForCell(index);

                const auto bitCount = std::popcount(available);

                if (bitCount == 0) {
                    result.invalid = true;
                    return result;
                }
                else if (bitCount == 1) {
                    setSolved(index, std::countr_zero(available));
                    didChange = true;
                    goto retry;
                }
                else if (bitCount < result.bitCount) {
                    result.bestIndex = index;
                    result.bitCount = bitCount;
                }
            }

            break;

        retry:
            continue;
        }

        if (solvedCount == 81) {
            result.solved = true;
        }

        return result;
    }

    // bitIndex 0 will set the lsb, bitIndex the next, etc
    void setSolved(uint8_t cellIndex, uint8_t bitIndex) noexcept {
        setSolved(&cells[cellIndex], bitIndex);
    }

    void setSolved(uint16_t* cell, uint8_t bitIndex) noexcept {
        *cell = SOLVED_FLAG | (1 << bitIndex);
    }

    void setUnknown(uint8_t cellIndex) noexcept {
        setUnknown(&cells[cellIndex]);
    }

    void setUnknown(uint16_t* cell) noexcept {
        *cell = ALL_VALUES_MASK;
    }

    bool isSolved(uint8_t cellIndex) const noexcept {
        return isSolved(&cells[cellIndex]);
    }

    bool isSolved(const uint16_t* cell) const noexcept {
        return (*cell) & SOLVED_FLAG;
    }

    uint8_t getSolvedValue(uint8_t cellIndex) const noexcept {
        return getSolvedValue(&cells[cellIndex]);
    }

    // undefined behavior if cell is not solved
    uint8_t getSolvedValue(const uint16_t* cell) const noexcept {
        return std::countr_zero(*cell) + 1;
    }

private:

    static const inline PossibleSolutionIterator solutionIteratorEnd{};

public:

    PossibleSolutionIterator possibleSolutionsBegin(uint8_t cellIndex) const {
        return PossibleSolutionIterator(this, cellIndex);
    }

    PossibleSolutionIterator possibleSolutionsEnd() const {
        return solutionIteratorEnd;
    }
};

static_assert(sizeof(Board) == 256, "Expected sizeof(Board) to be 256");
static_assert(alignof(Board) == 256, "Expected alignof(Board) to be 256");

#endif // BOARD_H
