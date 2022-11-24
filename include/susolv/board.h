#ifndef BOARD_H
#define BOARD_H

#include <bit>
#include <cstdint>
#include <iostream>
#include <optional>
#include <string>
#include <type_traits>
#include <cassert>

#include "susolv/cellIndexLookup.h"

enum class CellGroupIteratorKind { row, col, quad, end_sentinel };

template<CellGroupIteratorKind kind>
class CellGroupIterator {
private:

    uint16_t* const board_cells;
    const uint8_t base;
    uint8_t index = 0;

    static const CellGroupIterator<kind> end_sentinel;

    // default constructor is only used to init the "end sentinel" for each templated type
    CellGroupIterator() : board_cells(nullptr), base(0), index(9) {}

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

// alignas prevents passing by value on msvc ("formal parameter with requested alignment of <alignment> won't be aligned")
// (not that we need to pass these by value? maybe we want to move construct into function calls though?)
class alignas(256) Board {
    friend PossibleSolutionIterator;

public:

    uint16_t cells[81];

    static constexpr uint16_t SOLVED_FLAG     = 0b1000'0000'0000'0000;
    static constexpr uint16_t ALL_VALUES_MASK = 0b0000'0001'1111'1111;
    static constexpr uint16_t TAKEN_INIT      = 0b1111'1110'0000'0000;

    // possible values per cell in each row/col/quad
    // only relevant for unsolved cells
    // e.g. row[1] = 13 = 0b0000'0000'0000'1101 = row[1] can assign 1 or 3 or 4 to some cell
    struct PossibleValues {
        uint16_t row[9];
        uint16_t col[9];
        uint16_t quad[9];
    };

    // tracks the status of the 81 cells,
    // where they are either solved or not solved
    struct SolvedCellTracker {
        uint64_t b1 = 0;
        uint32_t b2 = 0;

        using B1 = decltype(b1);
        using B2 = decltype(b2);

        void setSolved(uint8_t index) {
            if (index >= 64) {
                b2 |= static_cast<B2>(1) << static_cast<B2>(index - 64);
            }
            else {
                b1 |= static_cast<B1>(1) << static_cast<B1>(index);
            }
        }

        bool isSolved(uint8_t index) {
            if (index >= 64) {
                return b2 & (static_cast<B2>(1) << static_cast<B2>(index - 64));
            }
            else {
                return b1 & (static_cast<B1>(1) << static_cast<B1>(index));
            }
        }

        uint8_t nextUnsolvedOnOrAfter(uint8_t index) const {
            if (index >= 64) {
                return index + std::countr_one(b2 >> (index - 64));
            }
            else {
                auto result = index + std::countr_one(b1 >> index);
                if (result == 64) {
                    return nextUnsolvedOnOrAfter(64);
                }
                else {
                    return result;
                }
            }
        }

        bool boardIsFullySolved() const {
            // b1 is fully set (64 bits) and the bottom 17 bits of b2 are set
            // 64 + 17 = 81
            return b1 == 0xffff'ffff'ffff'ffff && b2 == 0x0001'ffff;
        }

    } solvedIndices;

    PossibleValues takenValues{};

    Board() = default;
    Board(const Board& rhs) = default;
    Board(Board&& rhs) = default;

    friend std::ostream& operator<<(std::ostream& out, const Board& board);

    static Board ZeroedBoard() {
        Board board;
        std::fill(board.cells, board.cells+81, 0);
        return board;
    }

    Board(uint8_t const (&cells_literal)[9][9]) {
        for (int y = 0; y < 9; ++y) {
            for (int x = 0; x < 9; ++x) {
                const auto val = cells_literal[y][x];
                if (val == 0) {
                    setUnknown(cellIndexLookup.rowElementIndices[y][x]);
                }
                else {
                    setSolved(cellIndexLookup.rowElementIndices[y][x], val - 1);
                }
            }
        }
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
        const uint16_t available = ALL_VALUES_MASK & ~takenUnion;
        return available;
    }

public:
    SimpleSolveResult simpleSolve() noexcept {
        SimpleSolveResult result;
        bool didChange;

        while (true) {
            result = {};
            didChange = false;

            int index = 0;

            while(true) {
                index = solvedIndices.nextUnsolvedOnOrAfter(index);
                if (index >= 81) {
                    break;
                }

                const uint16_t availableBitFlags = availableValuesForCell(index);

                const auto bitCount = std::popcount(availableBitFlags);

                if (bitCount == 0) {
                    result.invalid = true;
                    return result;
                }
                else if (bitCount == 1) {
                    auto bit_index = std::countr_zero(availableBitFlags);
                    setSolved(index, bit_index);

                    assert(0 <= bit_index && bit_index <= 8);
                    assert(getSolvedValue(index) == bit_index + 1);

                    didChange = true;
                }
                else if (bitCount < result.bitCount) {
                    result.bestIndex = index;
                    result.bitCount = bitCount;
                }

                ++index;
            }

            if (!solvedIndices.boardIsFullySolved() && didChange) {
                continue;
            }
            else {
                break;
            }
        }

        result.solved = solvedIndices.boardIsFullySolved();

        return result;
    }

    // bitIndex 0 will set the lsb, bitIndex the next, etc
    void setSolved(uint8_t cellIndex, uint8_t bitIndex) noexcept {
        solvedIndices.setSolved(cellIndex);

        auto row = cellIndexLookup.indexToRow[cellIndex];
        auto col = cellIndexLookup.indexToCol[cellIndex];
        auto quad = cellIndexLookup.indexToQuad[cellIndex];
        takenValues.row[row] |= (1 << bitIndex);
        takenValues.col[col] |= (1 << bitIndex);
        takenValues.quad[quad] |= (1 << bitIndex);

        cells[cellIndex] = SOLVED_FLAG | (1 << bitIndex);
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
static_assert(std::is_nothrow_move_constructible_v<Board>, "`Board` should be nothrow move constructible.");

Board loadBoard(const char* fname);
std::optional<Board> solve(const Board& board);

#endif // BOARD_H
