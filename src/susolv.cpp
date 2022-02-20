#include <chrono>
#include <iostream>
#include <optional>
#include <vector>

#include "susolv/board.h"
#include "susolv/euler96.h"

using elapsed_t = decltype(std::chrono::high_resolution_clock::now() - std::chrono::high_resolution_clock::now());

template<typename T>
struct TimedResult {
    T result;
    elapsed_t elapsed;
};

template<>
struct TimedResult<void> {
    elapsed_t elapsed;
};

template<typename F>
auto withTime(F&& f) -> TimedResult<decltype(f())> {
    constexpr bool IS_VOID = std::is_same_v<decltype(f()), void>;
    auto start = std::chrono::high_resolution_clock::now();

    auto result = [&f]() {
        if constexpr (IS_VOID) {
            return f(), 0;
        }
        else {
            return f();
        }
    }();

    auto end = std::chrono::high_resolution_clock::now();

    if constexpr (IS_VOID) {
        return { .elapsed = end - start };
    }
    else {
        return {
            .result = result,
            .elapsed = end - start
        };
    }
}

int main() {
    auto [boards, file_elapsed] = withTime([]() { return loadEuler96("c:\\users\\anon\\dev\\susolv\\boards\\euler96-all.txt"); });

    std::cout << "Loaded " << boards.size() << " boards..." << std::endl;

    auto [sum, solve_elapsed] = withTime([&boards]() {
        uint64_t sum = 0;

        for (int i = 0; i < boards.size(); ++i) {
            Board& board = boards[i];
            std::optional<Board> maybeSolved = solve(board);
            if (!maybeSolved) {
                std::cout << "Board (" << i << ") not solveable." << std::endl;
            }

            sum += (*maybeSolved).getSolvedValue(static_cast<uint8_t>(0)) * 100;
            sum += (*maybeSolved).getSolvedValue(1) * 10;
            sum += (*maybeSolved).getSolvedValue(2) * 1;
        }

        return sum;
    });

    std::cout << "Times:\n";
    std::cout << "------\n";
    std::cout << "File read:\n";
    std::cout << "  "     << std::chrono::duration_cast<std::chrono::milliseconds>(file_elapsed).count() << "ms\n";
    std::cout << "  ("   << std::chrono::duration_cast<std::chrono::nanoseconds>(file_elapsed).count() << "ns)\n";
    std::cout << "Solve:" << std::endl;
    std::cout << "  "     << std::chrono::duration_cast<std::chrono::milliseconds>(solve_elapsed).count() << "ms\n";
    std::cout << "  ("    << std::chrono::duration_cast<std::chrono::nanoseconds>(solve_elapsed).count() << "ns)\n";
    std::cout << "\n";
    std::cout << "Answer: " << sum << "\n";

    return 0;
}

#if 0
int main()
{
    int i = 0;
    while (i < 1e4) {
        Board board = loadBoard("c:\\users\\anon\\dev\\susolv\\boards\\euler96-29.txt");
        auto start = std::chrono::high_resolution_clock::now();
        std::optional<Board> maybeSolvedBoard = solve(board);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = end - start;
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(elapsed).count() << "ns // " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count() << "ms\n";
        ++i;
        //std::cout << *maybeSolvedBoard << std::endl;
    }
    // if (maybeSolvedBoard) {
    //     std::cout << *maybeSolvedBoard << std::endl;
    // }
    // else {
    //     std::cout << "No solution." << std::endl;
    // }
    
}
#endif