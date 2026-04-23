#include "Board.h"
#include "Search.h"
#include <iostream>
#include <string>

static uint8_t parseSquare(const std::string& s) {
    uint8_t file = s[0] - 'a';
    uint8_t rank = s[1] - '1';
    return rank * 8 + file;
}

static std::string squareName(uint8_t sq) {
    char file = 'a' + (sq % 8);
    char rank = '1' + (sq / 8);
    return {file, rank};
}

int main() {
    Board board;
    const int DEPTH = 6;

    std::cout << "Chess engine ready. Enter moves as 'e2 e4'. Type 'quit' to exit.\n\n";

    while (true) {
        std::string from, to;
        std::cout << "Your move: ";
        std::cin >> from;
        if (from == "quit") break;
        if (from == "print") {
            const char* names[] = {".", "P","N","B","R","Q","K","p","n","b","r","q","k"};
            for (int r = 7; r >= 0; r--) {
                std::cout << (r+1) << " ";
                for (int f = 0; f < 8; f++)
                    std::cout << names[board.getPiece(r*8+f)] << " ";
                std::cout << "\n";
            }
            std::cout << "  a b c d e f g h\n";
            continue;
        }
        std::cin >> to;

        uint8_t fromSq = parseSquare(from);
        uint8_t toSq   = parseSquare(to);

        auto moves = board.generateLegalMoves();
        bool found = false;
        for (Move& m : moves) {
            if (m.from == fromSq && m.to == toSq) {
                UndoInfo undo(0, 0);
                board.makeMove(m, undo);
                std::cout << "You played: " << from << " -> " << to << "\n";
                found = true;
                break;
            }
        }
        if (!found) { std::cout << "Move not found.\n"; continue; }

        SearchResult result = search(board, DEPTH);
        UndoInfo undo(0, 0);
        board.makeMove(result.bestMove, undo);
        std::cout << "Engine plays: " << squareName(result.bestMove.from)
                  << " -> " << squareName(result.bestMove.to)
                  << " (score: " << result.score << ")\n\n";
    }

    return 0;
}
