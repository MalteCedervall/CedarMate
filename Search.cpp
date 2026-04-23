#include "Search.h"
#include "Eval.h"

static int alphaBeta(Board& board, uint8_t depth, int alpha, int beta) {
    if (depth == 0) {
        return evaluate(board);
    }

    bool anyLegalMove = false;

    for (Move m : board.generateLegalMoves()) {
        UndoInfo undo(0, 0);
        anyLegalMove = true;
        board.makeMove(m, undo);
        int score = -alphaBeta(board, depth - 1, -beta, -alpha);
        board.undoMove(undo);

        if (score >= beta) return beta;
        if (score > alpha) alpha = score;
    }

    if (!anyLegalMove) {
        if (board.isInCheck(board.getSideToMove())) {return -9999999;}
        return 0;
    }

    return alpha;
}

SearchResult search(Board& board, uint8_t depth) {
    const int max = 9999999;
    int bestScore = -max - 1;
    SearchResult result;

    for (Move m : board.generateLegalMoves()) {
        UndoInfo undo(0, 0);
        board.makeMove(m, undo);
        int score = -alphaBeta(board, depth - 1, -max, max);
        board.undoMove(undo);
        if (score > bestScore) {
            bestScore = score;
            result.bestMove = m;
        }
    }
    result.score = bestScore;
    result.depth = depth;

    return result;
}
