#include "Board.h"
#include <iostream>
#include <sstream>
#include <cassert>

Board::Board() {
    // Empty board
    for (int i = 16; i < 48; i++) squares[i] = EMPTY;

    // White pieces (rank 1, squares 0-7)
    squares[0] = W_ROOK;
    squares[1] = W_KNIGHT;
    squares[2] = W_BISHOP;
    squares[3] = W_QUEEN;
    squares[4] = W_KING;
    squares[5] = W_BISHOP;
    squares[6] = W_KNIGHT;
    squares[7] = W_ROOK;

    // White pawns (rank 2, squares 8-15)
    for (int i = 8; i <= 15; i++) squares[i] = W_PAWN;

    // Black pawns (rank 7, squares 48-55)
    for (int i = 48; i <= 55; i++) squares[i] = B_PAWN;

    // Black pieces (rank 8, squares 56-63)
    squares[56] = B_ROOK;
    squares[57] = B_KNIGHT;
    squares[58] = B_BISHOP;
    squares[59] = B_QUEEN;
    squares[60] = B_KING;
    squares[61] = B_BISHOP;
    squares[62] = B_KNIGHT;
    squares[63] = B_ROOK;

    sideToMove = WHITE;
    whiteCanCastleKingside  = true;
    whiteCanCastleQueenside = true;
    blackCanCastleKingside  = true;
    blackCanCastleQueenside = true;
    enPassantSquare = 64;
    whiteKingSquare = 4;
    blackKingSquare = 60;
}

bool Board::isInCheck(Color color) const {
    uint8_t king = (color == WHITE) ? whiteKingSquare : blackKingSquare;
    Piece enemyRook   = (color == WHITE) ? B_ROOK   : W_ROOK;
    Piece enemyBishop = (color == WHITE) ? B_BISHOP : W_BISHOP;
    Piece enemyQueen  = (color == WHITE) ? B_QUEEN  : W_QUEEN;
    Piece enemyKnight = (color == WHITE) ? B_KNIGHT : W_KNIGHT;
    Piece enemyPawn   = (color == WHITE) ? B_PAWN   : W_PAWN;

    // Check rooks and queens on ranks/files
    uint8_t n = king;
    while (n + 8 < 64) { n += 8; if (squares[n] != EMPTY) { if (squares[n] == enemyRook || squares[n] == enemyQueen) return true; break; } }
    n = king;
    while (n >= 8)      { n -= 8; if (squares[n] != EMPTY) { if (squares[n] == enemyRook || squares[n] == enemyQueen) return true; break; } }
    n = king;
    while (n % 8 != 7)  { n += 1; if (squares[n] != EMPTY) { if (squares[n] == enemyRook || squares[n] == enemyQueen) return true; break; } }
    n = king;
    while (n % 8 != 0)  { n -= 1; if (squares[n] != EMPTY) { if (squares[n] == enemyRook || squares[n] == enemyQueen) return true; break; } }

    // Check bishops and queens on diagonals
    n = king;
    while (n + 9 < 64 && n % 8 != 7) { n += 9; if (squares[n] != EMPTY) { if (squares[n] == enemyBishop || squares[n] == enemyQueen) return true; break; } }
    n = king;
    while (n + 7 < 64 && n % 8 != 0) { n += 7; if (squares[n] != EMPTY) { if (squares[n] == enemyBishop || squares[n] == enemyQueen) return true; break; } }
    n = king;
    while (n >= 9 && n % 8 != 0)     { n -= 9; if (squares[n] != EMPTY) { if (squares[n] == enemyBishop || squares[n] == enemyQueen) return true; break; } }
    n = king;
    while (n >= 7 && n % 8 != 7)     { n -= 7; if (squares[n] != EMPTY) { if (squares[n] == enemyBishop || squares[n] == enemyQueen) return true; break; } }

    // Check knights
    if (king % 8 != 0 && king + 15 < 64  && squares[king + 15] == enemyKnight) return true;
    if (king % 8 != 7 && king + 17 < 64  && squares[king + 17] == enemyKnight) return true;
    if (king % 8 >= 2 && king + 6  < 64  && squares[king + 6]  == enemyKnight) return true;
    if (king % 8 <= 5 && king + 10 < 64  && squares[king + 10] == enemyKnight) return true;
    if (king % 8 != 7 && king >= 15      && squares[king - 15] == enemyKnight) return true;
    if (king % 8 != 0 && king >= 17      && squares[king - 17] == enemyKnight) return true;
    if (king % 8 <= 5 && king >= 6       && squares[king - 6]  == enemyKnight) return true;
    if (king % 8 >= 2 && king >= 10      && squares[king - 10] == enemyKnight) return true;

    // Check pawns
    if (color == WHITE) {
        if (king % 8 != 0 && king + 7 < 64 && squares[king + 7] == enemyPawn) return true;
        if (king % 8 != 7 && king + 9 < 64 && squares[king + 9] == enemyPawn) return true;
    } else {
        if (king % 8 != 7 && king >= 7 && squares[king - 7] == enemyPawn) return true;
        if (king % 8 != 0 && king >= 9 && squares[king - 9] == enemyPawn) return true;
    }

    return false;
}

void Board::makeMove(const Move& move, UndoInfo& undoInfo) {
    undoInfo.from = move.from;
    undoInfo.to = move.to;
    undoInfo.captured = move.captured;
    undoInfo.isCastle = move.isCastle;
    undoInfo.isEnPassant = move.isEnPassant;
    undoInfo.promoted = move.promoted;

    // Save current rights so undoMove can restore them
    undoInfo.prevEnPassantSquare = enPassantSquare;
    undoInfo.prevWhiteCanCastleKingside = whiteCanCastleKingside;
    undoInfo.prevWhiteCanCastleQueenside = whiteCanCastleQueenside;
    undoInfo.prevBlackCanCastleKingside = blackCanCastleKingside;
    undoInfo.prevBlackCanCastleQueenside = blackCanCastleQueenside;
    undoInfo.prevWhiteKingSquare = whiteKingSquare;
    undoInfo.prevBlackKingSquare = blackKingSquare;

    // Update king position if king moves
    if (squares[move.from] == W_KING) whiteKingSquare = move.to;
    else if (squares[move.from] == B_KING) blackKingSquare = move.to;

    // En passant square is only valid for one ply
    enPassantSquare = 64;

    // Set en passant square if this is a double pawn push
    if (squares[move.from] == W_PAWN && move.to - move.from == 16)
        enPassantSquare = move.from + 8;
    else if (squares[move.from] == B_PAWN && move.from - move.to == 16)
        enPassantSquare = move.from - 8;

    // Revoke castle rights
    if (squares[move.from] == W_KING) {
        whiteCanCastleKingside = false;
        whiteCanCastleQueenside = false;
    } else if (squares[move.from] == B_KING) {
        blackCanCastleKingside = false;
        blackCanCastleQueenside = false;
    } else if (move.from == 7)  whiteCanCastleKingside = false;
    else if (move.from == 0)    whiteCanCastleQueenside = false;
    else if (move.from == 63)   blackCanCastleKingside = false;
    else if (move.from == 56)   blackCanCastleQueenside = false;

    // Rook captured on its starting square
    if (move.to == 7)  whiteCanCastleKingside = false;
    else if (move.to == 0)  whiteCanCastleQueenside = false;
    else if (move.to == 63) blackCanCastleKingside = false;
    else if (move.to == 56) blackCanCastleQueenside = false;

    if (move.isCastle) {
        squares[move.to] = squares[move.from];
        squares[move.from] = EMPTY;
        if (move.to == 6) {
            squares[5] = squares[7];
            squares[7] = EMPTY;
        } else if (move.to == 2) {
            squares[3] = squares[0];
            squares[0] = EMPTY;
        } else if (move.to == 62) {
            squares[61] = squares[63];
            squares[63] = EMPTY;
        } else if (move.to == 58) {
            squares[59] = squares[56];
            squares[56] = EMPTY;
        }
    }
    else if (move.isEnPassant) {
        if (sideToMove == WHITE) {
            undoInfo.captured = squares[move.to - 8];
            squares[move.to - 8] = EMPTY;}
        else {
            undoInfo.captured = squares[move.to + 8];
            squares[move.to + 8] = EMPTY;}
        squares[move.to] = squares[move.from];
        squares[move.from] = EMPTY;
    }
    else if (move.promoted != 0) {
        squares[move.to] = (Piece)move.promoted;
        squares[move.from] = EMPTY;
    }
    else if (squares[move.to] != EMPTY) {
        undoInfo.captured = squares[move.to];
        squares[move.to] = squares[move.from];
        squares[move.from] = EMPTY;
    }
    else {
        squares[move.to] = squares[move.from];
        squares[move.from] = EMPTY;
    }
    sideToMove = (sideToMove == WHITE) ? BLACK : WHITE;
}


void Board::undoMove(const UndoInfo& undoInfo) {
    sideToMove = (sideToMove == WHITE) ? BLACK : WHITE;
    enPassantSquare = undoInfo.prevEnPassantSquare;
    whiteCanCastleKingside = undoInfo.prevWhiteCanCastleKingside;
    whiteCanCastleQueenside = undoInfo.prevWhiteCanCastleQueenside;
    blackCanCastleKingside = undoInfo.prevBlackCanCastleKingside;
    blackCanCastleQueenside = undoInfo.prevBlackCanCastleQueenside;
    whiteKingSquare = undoInfo.prevWhiteKingSquare;
    blackKingSquare = undoInfo.prevBlackKingSquare;

    squares[undoInfo.from] = squares[undoInfo.to];
    if (undoInfo.isCastle) {
        squares[undoInfo.to] = EMPTY;
        if (undoInfo.to == 6) {
            squares[7] = squares[5];
            squares[5] = EMPTY;
        } else if (undoInfo.to == 2) {
            squares[0] = squares[3];
            squares[3] = EMPTY;
        } else if (undoInfo.to == 62) {
            squares[63] = squares[61];
            squares[61] = EMPTY;
        } else if (undoInfo.to == 58) {
            squares[56] = squares[59];
            squares[59] = EMPTY;
        }
    }
    else if (undoInfo.promoted != 0) {
        squares[undoInfo.from] = (undoInfo.promoted >= B_KNIGHT) ? B_PAWN : W_PAWN;
        squares[undoInfo.to] = (Piece)undoInfo.captured;
    }
    else if (undoInfo.isEnPassant) {
        if (sideToMove == WHITE) {squares[undoInfo.to - 8] = (Piece)undoInfo.captured;}
        else {squares[undoInfo.to + 8] = (Piece)undoInfo.captured;}
        squares[undoInfo.to] = EMPTY;
    }
    else {
        squares[undoInfo.to] = (Piece)undoInfo.captured;
    }
}


namespace {

// Piece values run white (1-6) then black (7-12), so one comparison splits them.
inline bool isWhitePiece(Piece p) { return p != EMPTY && p < B_PAWN; }
inline bool isBlackPiece(Piece p) { return p >= B_PAWN; }

inline Color colorOf(Piece p) { return isBlackPiece(p) ? BLACK : WHITE; }

inline bool isEnemy(Piece p, Color mover) {
    return (mover == WHITE) ? isBlackPiece(p) : isWhitePiece(p);
}

// How many files apart two squares are. A step that "wraps" off the a- or
// h-file lands on the far side of the board and shows up here as a big
// distance, which is how every generator below rejects it.
inline int fileDistance(int a, int b) {
    int d = (a % 8) - (b % 8);
    return (d < 0) ? -d : d;
}

// A pawn's diagonal step: on the board and exactly one file sideways.
inline bool diagonalStep(uint8_t from, int delta, uint8_t& to) {
    int t = from + delta;
    if (t < 0 || t > 63) return false;
    if (fileDistance(t, from) != 1) return false;
    to = (uint8_t)t;
    return true;
}

} // namespace


void Board::generateStepMoves(uint8_t from, const int* deltas, int count,
                              int maxFileShift, std::vector<Move>& moves) const {
    for (int d = 0; d < count; d++) {
        int to = from + deltas[d];
        if (to < 0 || to > 63) continue;
        if (fileDistance(to, from) > maxFileShift) continue;

        Piece target = squares[to];
        if (target == EMPTY || isEnemy(target, sideToMove))
            moves.emplace_back(Move(from, (uint8_t)to, (uint8_t)target));
    }
}


void Board::generateSlidingMoves(uint8_t from, const int* deltas, int count,
                                 std::vector<Move>& moves) const {
    for (int d = 0; d < count; d++) {
        int sq = from;
        while (true) {
            int to = sq + deltas[d];
            if (to < 0 || to > 63) break;
            if (fileDistance(to, sq) > 1) break;

            Piece target = squares[to];
            if (target != EMPTY) {
                if (isEnemy(target, sideToMove))
                    moves.emplace_back(Move(from, (uint8_t)to, (uint8_t)target));
                break;
            }
            moves.emplace_back(Move(from, (uint8_t)to));
            sq = to;
        }
    }
}


void Board::generatePawnMoves(uint8_t from, std::vector<Move>& moves) const {
    const bool white = (sideToMove == WHITE);

    const int forward = white ? 8 : -8;
    const int capA    = white ? 7 : -7;   // one diagonal...
    const int capB    = white ? 9 : -9;   // ...and the other
    const int capture[2] = { capA, capB };

    const uint8_t promoFirst  = white ? W_KNIGHT : B_KNIGHT;
    const bool    onPromoRank = white ? (from >= 48 && from <= 55) : (from >= 8  && from <= 15);
    const bool    onStartRank = white ? (from >= 8  && from <= 15) : (from >= 48 && from <= 55);

    // En passant. The target square is empty, so it needs its own test rather
    // than falling out of the ordinary capture checks below.
    if (enPassantSquare != 64) {
        const uint8_t victim = white ? enPassantSquare - 8 : enPassantSquare + 8;
        for (int delta : capture) {
            uint8_t to;
            if (diagonalStep(from, delta, to) && to == enPassantSquare)
                moves.emplace_back(Move(from, enPassantSquare, (uint8_t)squares[victim], 0, false, true));
        }
    }

    const int push = from + forward;

    if (onPromoRank) {
        if (push >= 0 && push <= 63 && squares[push] == EMPTY) {
            for (uint8_t promo = promoFirst; promo < promoFirst + 4; promo++)
                moves.emplace_back(Move(from, (uint8_t)push, 0, promo));
        }
        for (int delta : capture) {
            uint8_t to;
            if (diagonalStep(from, delta, to) && isEnemy(squares[to], sideToMove)) {
                for (uint8_t promo = promoFirst; promo < promoFirst + 4; promo++)
                    moves.emplace_back(Move(from, to, (uint8_t)squares[to], promo));
            }
        }
        return;
    }

    if (push >= 0 && push <= 63 && squares[push] == EMPTY) {
        moves.emplace_back(Move(from, (uint8_t)push));
        if (onStartRank && squares[from + 2 * forward] == EMPTY)
            moves.emplace_back(Move(from, (uint8_t)(from + 2 * forward)));
    }
    for (int delta : capture) {
        uint8_t to;
        if (diagonalStep(from, delta, to) && isEnemy(squares[to], sideToMove))
            moves.emplace_back(Move(from, to, (uint8_t)squares[to]));
    }
}


void Board::generateKnightMoves(uint8_t from, std::vector<Move>& moves) const {
    static const int deltas[8] = { 15, 17, -10, 6, -15, -17, 10, -6 };
    generateStepMoves(from, deltas, 8, 2, moves);
}


void Board::generateBishopMoves(uint8_t from, std::vector<Move>& moves) const {
    static const int deltas[4] = { 7, -7, 9, -9 };
    generateSlidingMoves(from, deltas, 4, moves);
}


void Board::generateRookMoves(uint8_t from, std::vector<Move>& moves) const {
    static const int deltas[4] = { 8, -8, -1, 1 };
    generateSlidingMoves(from, deltas, 4, moves);
}


void Board::generateQueenMoves(uint8_t from, std::vector<Move>& moves) const {
    generateRookMoves(from, moves);
    generateBishopMoves(from, moves);
}


void Board::generateKingMoves(uint8_t from, std::vector<Move>& moves) const {
    static const int deltas[8] = { 1, -1, 8, -8, 7, -9, 9, -7 };
    generateStepMoves(from, deltas, 8, 1, moves);

    const bool    kingside  = (sideToMove == WHITE) ? whiteCanCastleKingside  : blackCanCastleKingside;
    const bool    queenside = (sideToMove == WHITE) ? whiteCanCastleQueenside : blackCanCastleQueenside;
    const uint8_t home      = (sideToMove == WHITE) ? 4 : 60;

    if (from != home || isInCheck(sideToMove)) return;

    if (kingside && squares[home + 1] == EMPTY && squares[home + 2] == EMPTY)
        moves.emplace_back(Move(from, from + 2, 0, 0, true));

    if (queenside && squares[home - 1] == EMPTY && squares[home - 2] == EMPTY && squares[home - 3] == EMPTY)
        moves.emplace_back(Move(from, from - 2, 0, 0, true));
}


std::vector<Move> Board::generateLegalMoves() {
    std::vector<Move> moves;
    moves.reserve(64);

    for (uint8_t i = 0; i < 64; i++) {
        Piece p = squares[i];
        if (p == EMPTY || colorOf(p) != sideToMove) continue;

        switch (p) {
            case W_PAWN:
            case B_PAWN:    generatePawnMoves(i, moves);   break;

            case W_KNIGHT:
            case B_KNIGHT:  generateKnightMoves(i, moves); break;

            case W_BISHOP:
            case B_BISHOP:  generateBishopMoves(i, moves); break;

            case W_ROOK:
            case B_ROOK:    generateRookMoves(i, moves);   break;

            case W_QUEEN:
            case B_QUEEN:   generateQueenMoves(i, moves);  break;

            case W_KING:
            case B_KING:    generateKingMoves(i, moves);   break;

            case EMPTY:     break;
        }
    }

    std::vector<Move> legal;
    legal.reserve(moves.size());
    Color mover = sideToMove;
    for (Move& m : moves) {
        UndoInfo undo(0, 0);
        makeMove(m, undo);
        if (!isInCheck(mover))
            legal.push_back(m);
        undoMove(undo);
    }
    return legal;
}
