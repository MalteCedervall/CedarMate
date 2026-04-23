#include "Eval.h"

static const int pieceVal[] = {0, 100, 300, 300, 500, 900, 0, 100, 300, 300, 500, 900, 0};

// Piece-Square Tables for white. Mirror vertically (63-i) for black.
// Square 0 = a1 (white back rank), 63 = h8 (black back rank).
static const int knightPST[64] = {
    -30,-20,-10,-10,-10,-10,-20,-30,
    -20,-10,  0,  5,  5,  0,-10,-20,
    -10,  5, 15, 20, 20, 15,  5,-10,
    -10,  5, 20, 25, 25, 20,  5,-10,
    -10,  5, 20, 25, 25, 20,  5,-10,
    -10,  5, 15, 20, 20, 15,  5,-10,
    -20,-10,  0,  5,  5,  0,-10,-20,
    -30,-20,-10,-10,-10,-10,-20,-30,
};

static const int bishopPST[64] = {
    -20,-10,-10,-10,-10,-10,-10,-20,
    -10,  0,  0,  0,  0,  0,  0,-10,
    -10,  0,  5, 10, 10,  5,  0,-10,
    -10,  5,  5, 15, 15,  5,  5,-10,
    -10,  0, 15, 15, 15, 15,  0,-10,
    -10, 10, 10, 10, 10, 10, 10,-10,
    -10,  5,  0,  0,  0,  0,  5,-10,
    -20,-10,-10,-10,-10,-10,-10,-20,
};

// Pawn PST: reward center control and advancement.
static const int pawnPST[64] = {
     0,  0,  0,  0,  0,  0,  0,  0,  // rank 1 (unused)
     0,  0, -5,  0,  0, -5,  0,  0,  // rank 2: only penalize c/f pawns staying put, flanks neutral
     5,  5, 10, 20, 20, 10,  5,  5,  // rank 3
     5, 10, 20, 40, 40, 20, 10,  5,  // rank 4: big center bonus (d4/e4)
    10, 20, 30, 50, 50, 30, 20, 10,  // rank 5
    20, 30, 40, 50, 50, 40, 30, 20,  // rank 6
    50, 50, 50, 50, 50, 50, 50, 50,  // rank 7 (one step from promotion)
     0,  0,  0,  0,  0,  0,  0,  0,  // rank 8 (promoted)
};

// King PST (middlegame): reward castled squares, heavily penalize center and advanced ranks.
static const int kingPST[64] = {
   -10, 10, 20, -30, -50, -20, 30, 10,  // rank 1: g1/c1 good, center bad
   -20,-30, -40, -50, -50, -40,-30,-20,  // rank 2: king wandering
   -40,-50, -60, -70, -70, -60,-50,-40,  // rank 3
   -60,-70, -80, -90, -90, -80,-70,-60,  // rank 4+: king in the open is disaster
   -60,-70, -80, -90, -90, -80,-70,-60,
   -60,-70, -80, -90, -90, -80,-70,-60,
   -60,-70, -80, -90, -90, -80,-70,-60,
   -60,-70, -80, -90, -90, -80,-70,-60,
};

// Rook PST: bonus for rank 7 (squares 48-55), small bonus for center files on rank 1.
static const int rookPST[64] = {
     0,  0,  0,  5,  5,  0,  0,  0,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
    -5,  0,  0,  0,  0,  0,  0, -5,
     5, 10, 10, 10, 10, 10, 10,  5,
     0,  0,  0,  5,  5,  0,  0,  0,
};

int evaluate(const Board& board) {
    int score = 0;

    //piece value
    for (uint8_t i = 0; i < 64; i++) {
        switch (board.getPiece(i)) {
            case W_PAWN:   score += 100;
                score += pawnPST[i];
                break;
            case W_KNIGHT: score += 300;
                score += knightPST[i];
                if (i % 8 != 0 && i + 15 < 64  && board.getPiece(i + 15) >= B_PAWN) score += pieceVal[board.getPiece(i + 15)] / 4;
                if (i % 8 != 7 && i + 17 < 64  && board.getPiece(i + 17) >= B_PAWN) score += pieceVal[board.getPiece(i + 17)] / 4;
                if (i % 8 >= 2 && i + 6  < 64  && board.getPiece(i + 6)  >= B_PAWN) score += pieceVal[board.getPiece(i + 6)]  / 4;
                if (i % 8 <= 5 && i + 10 < 64  && board.getPiece(i + 10) >= B_PAWN) score += pieceVal[board.getPiece(i + 10)] / 4;
                if (i % 8 != 7 && i >= 15      && board.getPiece(i - 15) >= B_PAWN) score += pieceVal[board.getPiece(i - 15)] / 4;
                if (i % 8 != 0 && i >= 17      && board.getPiece(i - 17) >= B_PAWN) score += pieceVal[board.getPiece(i - 17)] / 4;
                if (i % 8 <= 5 && i >= 6       && board.getPiece(i - 6)  >= B_PAWN) score += pieceVal[board.getPiece(i - 6)]  / 4;
                if (i % 8 >= 2 && i >= 10      && board.getPiece(i - 10) >= B_PAWN) score += pieceVal[board.getPiece(i - 10)] / 4;
                break;
            case W_BISHOP: score += 300;
                score += bishopPST[i]; {
                uint8_t n = i;
                while (n % 8 != 0 && n + 7 < 64 && board.getPiece(n + 7) == EMPTY) { n += 7; }
                if (n % 8 != 0    && n + 7 < 64 && board.getPiece(n + 7) >= B_PAWN) score += pieceVal[board.getPiece(n + 7)] / 4;
                n = i;
                while (n % 8 != 7 && n + 9 < 64 && board.getPiece(n + 9) == EMPTY) { n += 9; }
                if (n % 8 != 7    && n + 9 < 64 && board.getPiece(n + 9) >= B_PAWN) score += pieceVal[board.getPiece(n + 9)] / 4;
                n = i;
                while (n % 8 != 0 && n >= 9     && board.getPiece(n - 9) == EMPTY) { n -= 9; }
                if (n % 8 != 0    && n >= 9     && board.getPiece(n - 9) >= B_PAWN) score += pieceVal[board.getPiece(n - 9)] / 4;
                n = i;
                while (n % 8 != 7 && n >= 7     && board.getPiece(n - 7) == EMPTY) { n -= 7; }
                if (n % 8 != 7    && n >= 7     && board.getPiece(n - 7) >= B_PAWN) score += pieceVal[board.getPiece(n - 7)] / 4;
                break; }
            case W_ROOK:   score += 500;
                score += rookPST[i]; {
                uint8_t n = i;
                while (n + 8 < 64 && board.getPiece(n + 8) == EMPTY) { n += 8; }
                if (n + 8 < 64    && board.getPiece(n + 8) >= B_PAWN) score += pieceVal[board.getPiece(n + 8)] / 4;
                n = i;
                while (n >= 8     && board.getPiece(n - 8) == EMPTY) { n -= 8; }
                if (n >= 8        && board.getPiece(n - 8) >= B_PAWN) score += pieceVal[board.getPiece(n - 8)] / 4;
                n = i;
                while (n % 8 != 7 && board.getPiece(n + 1) == EMPTY) { n += 1; }
                if (n % 8 != 7    && board.getPiece(n + 1) >= B_PAWN) score += pieceVal[board.getPiece(n + 1)] / 4;
                n = i;
                while (n % 8 != 0 && board.getPiece(n - 1) == EMPTY) { n -= 1; }
                if (n % 8 != 0    && board.getPiece(n - 1) >= B_PAWN) score += pieceVal[board.getPiece(n - 1)] / 4;
                break; }
            case W_QUEEN:  score += 900; {
                uint8_t n = i;
                // rook rays
                while (n + 8 < 64 && board.getPiece(n + 8) == EMPTY) { n += 8; }
                if (n + 8 < 64    && board.getPiece(n + 8) >= B_PAWN) score += pieceVal[board.getPiece(n + 8)] / 4;
                n = i;
                while (n >= 8     && board.getPiece(n - 8) == EMPTY) { n -= 8; }
                if (n >= 8        && board.getPiece(n - 8) >= B_PAWN) score += pieceVal[board.getPiece(n - 8)] / 4;
                n = i;
                while (n % 8 != 7 && board.getPiece(n + 1) == EMPTY) { n += 1; }
                if (n % 8 != 7    && board.getPiece(n + 1) >= B_PAWN) score += pieceVal[board.getPiece(n + 1)] / 4;
                n = i;
                while (n % 8 != 0 && board.getPiece(n - 1) == EMPTY) { n -= 1; }
                if (n % 8 != 0    && board.getPiece(n - 1) >= B_PAWN) score += pieceVal[board.getPiece(n - 1)] / 4;
                n = i;
                // bishop rays
                while (n % 8 != 0 && n + 7 < 64 && board.getPiece(n + 7) == EMPTY) { n += 7; }
                if (n % 8 != 0    && n + 7 < 64 && board.getPiece(n + 7) >= B_PAWN) score += pieceVal[board.getPiece(n + 7)] / 4;
                n = i;
                while (n % 8 != 7 && n + 9 < 64 && board.getPiece(n + 9) == EMPTY) { n += 9; }
                if (n % 8 != 7    && n + 9 < 64 && board.getPiece(n + 9) >= B_PAWN) score += pieceVal[board.getPiece(n + 9)] / 4;
                n = i;
                while (n % 8 != 0 && n >= 9     && board.getPiece(n - 9) == EMPTY) { n -= 9; }
                if (n % 8 != 0    && n >= 9     && board.getPiece(n - 9) >= B_PAWN) score += pieceVal[board.getPiece(n - 9)] / 4;
                n = i;
                while (n % 8 != 7 && n >= 7     && board.getPiece(n - 7) == EMPTY) { n -= 7; }
                if (n % 8 != 7    && n >= 7     && board.getPiece(n - 7) >= B_PAWN) score += pieceVal[board.getPiece(n - 7)] / 4;
                break; }
            case W_KING:
                score += kingPST[i];
                if (i + 8 < 64) {
                    if (i % 8 != 0 && board.getPiece(i + 7) == W_PAWN) score += 15;
                    if (              board.getPiece(i + 8) == W_PAWN) score += 15;
                    if (i % 8 != 7 && board.getPiece(i + 9) == W_PAWN) score += 15;
                }
                break;
            case B_PAWN:   score -= 100;
                score -= pawnPST[63 - i];
                break;
            case B_KNIGHT: score -= 300;
                score -= knightPST[63 - i];
                if (i % 8 != 0 && i + 15 < 64  && board.getPiece(i + 15) > 0 && board.getPiece(i + 15) < B_PAWN) score -= pieceVal[board.getPiece(i + 15)] / 4;
                if (i % 8 != 7 && i + 17 < 64  && board.getPiece(i + 17) > 0 && board.getPiece(i + 17) < B_PAWN) score -= pieceVal[board.getPiece(i + 17)] / 4;
                if (i % 8 >= 2 && i + 6  < 64  && board.getPiece(i + 6)  > 0 && board.getPiece(i + 6)  < B_PAWN) score -= pieceVal[board.getPiece(i + 6)]  / 4;
                if (i % 8 <= 5 && i + 10 < 64  && board.getPiece(i + 10) > 0 && board.getPiece(i + 10) < B_PAWN) score -= pieceVal[board.getPiece(i + 10)] / 4;
                if (i % 8 != 7 && i >= 15      && board.getPiece(i - 15) > 0 && board.getPiece(i - 15) < B_PAWN) score -= pieceVal[board.getPiece(i - 15)] / 4;
                if (i % 8 != 0 && i >= 17      && board.getPiece(i - 17) > 0 && board.getPiece(i - 17) < B_PAWN) score -= pieceVal[board.getPiece(i - 17)] / 4;
                if (i % 8 <= 5 && i >= 6       && board.getPiece(i - 6)  > 0 && board.getPiece(i - 6)  < B_PAWN) score -= pieceVal[board.getPiece(i - 6)]  / 4;
                if (i % 8 >= 2 && i >= 10      && board.getPiece(i - 10) > 0 && board.getPiece(i - 10) < B_PAWN) score -= pieceVal[board.getPiece(i - 10)] / 4;
                break;
            case B_BISHOP: score -= 300;
                score -= bishopPST[63 - i]; {
                uint8_t n = i;
                while (n % 8 != 0 && n + 7 < 64 && board.getPiece(n + 7) == EMPTY) { n += 7; }
                if (n % 8 != 0    && n + 7 < 64 && board.getPiece(n + 7) > 0 && board.getPiece(n + 7) < B_PAWN) score -= pieceVal[board.getPiece(n + 7)] / 4;
                n = i;
                while (n % 8 != 7 && n + 9 < 64 && board.getPiece(n + 9) == EMPTY) { n += 9; }
                if (n % 8 != 7    && n + 9 < 64 && board.getPiece(n + 9) > 0 && board.getPiece(n + 9) < B_PAWN) score -= pieceVal[board.getPiece(n + 9)] / 4;
                n = i;
                while (n % 8 != 0 && n >= 9     && board.getPiece(n - 9) == EMPTY) { n -= 9; }
                if (n % 8 != 0    && n >= 9     && board.getPiece(n - 9) > 0 && board.getPiece(n - 9) < B_PAWN) score -= pieceVal[board.getPiece(n - 9)] / 4;
                n = i;
                while (n % 8 != 7 && n >= 7     && board.getPiece(n - 7) == EMPTY) { n -= 7; }
                if (n % 8 != 7    && n >= 7     && board.getPiece(n - 7) > 0 && board.getPiece(n - 7) < B_PAWN) score -= pieceVal[board.getPiece(n - 7)] / 4;
                break; }
            case B_ROOK:   score -= 500;
                score -= rookPST[63 - i]; {
                uint8_t n = i;
                while (n + 8 < 64 && board.getPiece(n + 8) == EMPTY) { n += 8; }
                if (n + 8 < 64    && board.getPiece(n + 8) > 0 && board.getPiece(n + 8) < B_PAWN) score -= pieceVal[board.getPiece(n + 8)] / 4;
                n = i;
                while (n >= 8     && board.getPiece(n - 8) == EMPTY) { n -= 8; }
                if (n >= 8        && board.getPiece(n - 8) > 0 && board.getPiece(n - 8) < B_PAWN) score -= pieceVal[board.getPiece(n - 8)] / 4;
                n = i;
                while (n % 8 != 7 && board.getPiece(n + 1) == EMPTY) { n += 1; }
                if (n % 8 != 7    && board.getPiece(n + 1) > 0 && board.getPiece(n + 1) < B_PAWN) score -= pieceVal[board.getPiece(n + 1)] / 4;
                n = i;
                while (n % 8 != 0 && board.getPiece(n - 1) == EMPTY) { n -= 1; }
                if (n % 8 != 0    && board.getPiece(n - 1) > 0 && board.getPiece(n - 1) < B_PAWN) score -= pieceVal[board.getPiece(n - 1)] / 4;
                break; }
            case B_QUEEN:  score -= 900; {
                uint8_t n = i;
                // rook rays
                while (n + 8 < 64 && board.getPiece(n + 8) == EMPTY) { n += 8; }
                if (n + 8 < 64    && board.getPiece(n + 8) > 0 && board.getPiece(n + 8) < B_PAWN) score -= pieceVal[board.getPiece(n + 8)] / 4;
                n = i;
                while (n >= 8     && board.getPiece(n - 8) == EMPTY) { n -= 8; }
                if (n >= 8        && board.getPiece(n - 8) > 0 && board.getPiece(n - 8) < B_PAWN) score -= pieceVal[board.getPiece(n - 8)] / 4;
                n = i;
                while (n % 8 != 7 && board.getPiece(n + 1) == EMPTY) { n += 1; }
                if (n % 8 != 7    && board.getPiece(n + 1) > 0 && board.getPiece(n + 1) < B_PAWN) score -= pieceVal[board.getPiece(n + 1)] / 4;
                n = i;
                while (n % 8 != 0 && board.getPiece(n - 1) == EMPTY) { n -= 1; }
                if (n % 8 != 0    && board.getPiece(n - 1) > 0 && board.getPiece(n - 1) < B_PAWN) score -= pieceVal[board.getPiece(n - 1)] / 4;
                n = i;
                // bishop rays
                while (n % 8 != 0 && n + 7 < 64 && board.getPiece(n + 7) == EMPTY) { n += 7; }
                if (n % 8 != 0    && n + 7 < 64 && board.getPiece(n + 7) > 0 && board.getPiece(n + 7) < B_PAWN) score -= pieceVal[board.getPiece(n + 7)] / 4;
                n = i;
                while (n % 8 != 7 && n + 9 < 64 && board.getPiece(n + 9) == EMPTY) { n += 9; }
                if (n % 8 != 7    && n + 9 < 64 && board.getPiece(n + 9) > 0 && board.getPiece(n + 9) < B_PAWN) score -= pieceVal[board.getPiece(n + 9)] / 4;
                n = i;
                while (n % 8 != 0 && n >= 9    && board.getPiece(n - 9) == EMPTY) { n -= 9; }
                if (n % 8 != 0    && n >= 9    && board.getPiece(n - 9) > 0 && board.getPiece(n - 9) < B_PAWN) score -= pieceVal[board.getPiece(n - 9)] / 4;
                n = i;
                while (n % 8 != 7 && n >= 7    && board.getPiece(n - 7) == EMPTY) { n -= 7; }
                if (n % 8 != 7    && n >= 7    && board.getPiece(n - 7) > 0 && board.getPiece(n - 7) < B_PAWN) score -= pieceVal[board.getPiece(n - 7)] / 4;
                break; }
            case B_KING:
                score -= kingPST[63 - i];
                if (i >= 8) {
                    if (i % 8 != 7 && board.getPiece(i - 7) == B_PAWN) score -= 15;
                    if (              board.getPiece(i - 8) == B_PAWN) score -= 15;
                    if (i % 8 != 0 && board.getPiece(i - 9) == B_PAWN) score -= 15;
                }
                break;
            default: break;
        }
    }

    //check value
    if (board.isInCheck(BLACK)) {
        score += 10;
    }
    if (board.isInCheck(WHITE)) {
        score -= 10;
    }

    return (board.getSideToMove() == WHITE) ? score : -score;
}
