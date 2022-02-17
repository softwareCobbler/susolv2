#include <iostream>
#include <deque>
#include "susolv/board.h"

Board loadBoard(const char* fname) {
    Board board;

    FILE* f;
    fopen_s(&f, fname, "r");

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
        else if (c == '\n') {
            continue;
        }
        else {
            uint8_t value = c - '0';
            if (value == 0) {
                board.setUnknown(static_cast<uint8_t>(index));
            }
            else {
                board.setSolved(static_cast<uint8_t>(index), value);
            }
            ++index;
        }

    }

    fclose(f);

    return board;
};

void solve(const Board& board) {
    std::deque<Board> boards{ board };

    while (boards.size() > 0) {
        Board& workingBoard = boards.front();
        Board::SimpleSolveResult result = workingBoard.simpleSolve();

        if (result.solved) {
            std::cout << "SOLVED" << std::endl;
        }
        else if (result.invalid) {
            boards.pop_front();
        }
        else {
            for (auto iter = workingBoard.possibleSolutionsBegin(result.bestIndex); iter != workingBoard.possibleSolutionsEnd(); ++iter) {
                auto board = *iter;
                boards.push_back(board);
            }
            boards.pop_front();
        }
    }

    std::cout << "HM" << std::endl;
}

int main(int argc)
{
    Board board = loadBoard("c:\\users\\anon\\dev\\rmme\\sudoku1.txt");
    solve(board);

    std::cout << "X" << std::endl;
}

