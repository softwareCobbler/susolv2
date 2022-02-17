#include <gtest/gtest.h>
#include "susolv/cellIndexLookup.h"
#include "susolv/board.h"

TEST(MainSuite, PossibleSolutionsIterator) {
    Board board;

    for (int i = 0; i < 81; ++i) {
        board.cells[i] = 0;
    }

    Board::SimpleSolveResult result = board.simpleSolve();
    EXPECT_EQ(result.invalid, false);
    EXPECT_EQ(result.solved, false);
    EXPECT_EQ(result.bestIndex, 0);

    uint8_t expectedSolvedValue = 1;
    for (auto iter = board.possibleSolutionsBegin(0); iter != board.possibleSolutionsEnd(); ++iter, ++expectedSolvedValue) {
        Board freshBoard = *iter;
        EXPECT_EQ(freshBoard.isSolved(static_cast<uint8_t>(0)), true);
        EXPECT_EQ(expectedSolvedValue, freshBoard.getSolvedValue(static_cast<uint8_t>(0)));
    }
}

TEST(MainSuite, SomeOtherTest) {
    Board board = Board::ZeroedBoard();

    for (int i = 0; i < 7; ++i) {
        // row 0, index i
        const uint8_t cellIndex = cellIndexLookup.rowElementIndices[0][i];
        board.setSolved(cellIndex, i);
    }

    // row 0 is now [1,2,3,4,5,6,7,?,?]
    // indexes       0 1 2 3 4 5 6 7 8

    Board::SimpleSolveResult result = board.simpleSolve();
    // it's a bit of implementation detail that 7 is the best index, since 7 and 8 are both equivalently "best"
    // but once a "best" is chosen, equivalently good cells don't override that decision
    const uint8_t expectedBestIndex = 7;

    EXPECT_EQ(result.invalid, false);
    EXPECT_EQ(result.solved, false);
    EXPECT_EQ(result.bestIndex, expectedBestIndex);

    int solutionIndex = 0;
    int solvedForValues[2] = { 8, 9 };

    for (auto iter = board.possibleSolutionsBegin(result.bestIndex); iter != board.possibleSolutionsEnd(); ++iter, ++solutionIndex) {
        Board freshBoard = *iter;
        EXPECT_EQ(freshBoard.isSolved(7), true);
        EXPECT_EQ(freshBoard.getSolvedValue(static_cast<uint8_t>(7)), solvedForValues[solutionIndex]);
    }
}