#include <chrono>
#include <deque>
#include <iostream>
#include <optional>

#include "susolv/board.h"

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
};

std::optional<Board> solve(const Board& board) {
    std::deque<Board> boards{ board };

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
                boards.push_back(*iter);
            }
            boards.pop_front();
        }
    }

    return {};
}

int main()
{
    int i = 0;
    while (i < 1000) {
        Board board = loadBoard("/home/david/rmme/susolv2/boards/euler96-29.txt");
        auto start = std::chrono::high_resolution_clock::now();
        std::optional<Board> maybeSolvedBoard = solve(board);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - start;
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << "ns // " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << "ms\n";
        ++i;
    }
    // if (maybeSolvedBoard) {
    //     std::cout << *maybeSolvedBoard << std::endl;
    // }
    // else {
    //     std::cout << "No solution." << std::endl;
    // }
    
}

