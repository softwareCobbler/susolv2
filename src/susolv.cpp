#include <chrono>
#include <optional>
#include <iostream>

#include "susolv/board.h"

int main()
{
    int i = 0;
    while (i < 1) {
        Board board = loadBoard("c:\\users\\anon\\dev\\susolv\\boards\\euler96-29.txt");
        auto start = std::chrono::high_resolution_clock::now();
        std::optional<Board> maybeSolvedBoard = solve(board);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - start;
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << "ns // " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << "ms\n";
        ++i;
        std::cout << *maybeSolvedBoard << std::endl;
    }
    // if (maybeSolvedBoard) {
    //     std::cout << *maybeSolvedBoard << std::endl;
    // }
    // else {
    //     std::cout << "No solution." << std::endl;
    // }
    
}

