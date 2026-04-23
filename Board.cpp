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



std::vector<Move> Board::generateLegalMoves() {
    std::vector<Move> moves;
    for (uint8_t i = 0; i < 64; i += 1) {
        Piece p = squares[i];
        if (sideToMove == WHITE) {
            if (p == W_PAWN) {

                if (enPassantSquare != 64 && i % 8 != 0 && i + 7 == enPassantSquare) {
                    moves.emplace_back(Move(i, enPassantSquare, (uint8_t)squares[enPassantSquare - 8], 0, false, true));
                }
                if (enPassantSquare != 64 && i % 8 != 7 && i + 9 == enPassantSquare) {
                    moves.emplace_back(Move(i, enPassantSquare, (uint8_t)squares[enPassantSquare - 8], 0, false, true));
                }

                //last row
                if (i >= 48 && i <= 55) {
                    if (squares[i + 8] == EMPTY) {
                        for (uint8_t j = 2; j < 6; j += 1){
                            moves.emplace_back(Move(i, i + 8, 0, j));}
                    }
                    if (i % 8 != 0 && squares[i + 7] != EMPTY && squares[i + 7] >= B_PAWN) {
                        for (uint8_t j = 2; j < 6; j += 1){
                            moves.emplace_back(Move(i, i + 7, (uint8_t)squares[i + 7], j));
                        }
                    }
                    if (i % 8 != 7 && squares[i + 9] != EMPTY && squares[i + 9] >= B_PAWN) {
                        for (uint8_t j = 2; j < 6; j += 1) {
                            moves.emplace_back(Move(i, i + 9, (uint8_t)squares[i + 9], j));
                        }
                    }
                }
                // all other rows
                else {
                    if (squares[i + 8] == EMPTY) {
                        moves.emplace_back(Move(i, i + 8));
                        if (i >= 8 && i <= 15 && squares[i + 16] == EMPTY) {
                            moves.emplace_back(Move(i, i + 16));
                        }
                    }
                    if (i % 8 != 0 && squares[i + 7] != EMPTY && squares[i + 7] >= B_PAWN) {
                        moves.emplace_back(Move(i, i + 7, (uint8_t)squares[i + 7]));
                    }
                    if (i % 8 != 7 && squares[i + 9] != EMPTY && squares[i + 9] >= B_PAWN) {
                        moves.emplace_back(Move(i, i + 9, (uint8_t)squares[i + 9]));
                    }
                }
            }

            else if (p == W_KNIGHT) {
                //forward
                if (i % 8 != 0 && i + 15 < 64 && (squares[i + 15] == EMPTY || squares[i + 15] >= B_PAWN))   {moves.emplace_back(Move(i, i + 15, (uint8_t)squares[i + 15]));}
                if (i % 8 != 7 && i + 17 < 64 && (squares[i + 17] == EMPTY || squares[i + 17] >= B_PAWN))   {moves.emplace_back(Move(i, i + 17, (uint8_t)squares[i + 17]));}

                //left
                if (i % 8 >= 2 && i >= 10 && (squares[i - 10] == EMPTY || squares[i - 10] >= B_PAWN))       {moves.emplace_back(Move(i, i - 10, (uint8_t)squares[i - 10]));}
                if (i % 8 >= 2 && i + 6 < 64 && (squares[i + 6] == EMPTY || squares[i + 6] >= B_PAWN))      {moves.emplace_back(Move(i, i + 6, (uint8_t)squares[i + 6]));}

                //down
                if (i % 8 != 7 && i >= 15 && (squares[i - 15] == EMPTY || squares[i - 15] >= B_PAWN))       {moves.emplace_back(Move(i, i - 15, (uint8_t)squares[i - 15]));}
                if (i % 8 != 0 && i >= 17 && (squares[i - 17] == EMPTY || squares[i - 17] >= B_PAWN))       {moves.emplace_back(Move(i, i - 17, (uint8_t)squares[i - 17]));}

                //right
                if (i % 8 <= 5 && i + 10 < 64 && (squares[i + 10] == EMPTY || squares[i + 10] >= B_PAWN))   {moves.emplace_back(Move(i, i + 10, (uint8_t)squares[i + 10]));}
                if (i % 8 <= 5 && i >= 6 && (squares[i - 6] == EMPTY || squares[i - 6] >= B_PAWN))          {moves.emplace_back(Move(i, i - 6, (uint8_t)squares[i - 6]));}

            }

            else if (p == W_BISHOP) {
                uint8_t n = i;

                //left up
                while (n + 7 < 64 && n % 8 != 0 && squares[n + 7] == EMPTY) {
                    moves.emplace_back(Move(i, n + 7));
                    n += 7;
                }
                if (n + 7 < 64 && n % 8 != 0 && squares[n + 7] >= B_PAWN)       {moves.emplace_back(Move(i, n + 7, (uint8_t)squares[n + 7]));}
                n = i;

                // right down
                while (n >= 7 && n % 8 != 7 && squares[n - 7] == EMPTY) {
                    moves.emplace_back(Move(i, n - 7));
                    n -= 7;
                }
                if (n >= 7 && n % 8 != 7 && squares[n - 7] >= B_PAWN)            {moves.emplace_back(Move(i, n - 7, (uint8_t)squares[n - 7]));}
                n = i;

                //right up
                while (n + 9 < 64 && n % 8 != 7 && squares[n + 9] == EMPTY) {
                    moves.emplace_back(Move(i, n + 9));
                    n += 9;
                }
                if (n + 9 < 64 && n % 8 != 7 && squares[n + 9] >= B_PAWN)       {moves.emplace_back(Move(i, n + 9, (uint8_t)squares[n + 9]));}
                n = i;

                //left down
                while (n >= 9 && n % 8 != 0 && squares[n - 9] == EMPTY) {
                    moves.emplace_back(Move(i, n - 9));
                    n -= 9;
                }
                if (n >= 9 && n % 8 != 0 && squares[n - 9] >= B_PAWN)            {moves.emplace_back(Move(i, n - 9, (uint8_t)squares[n - 9]));}
            }

            else if (p == W_ROOK) {
                uint8_t n = i;

                // up
                while (n + 8 < 64 && squares[n + 8] == EMPTY) {
                    moves.emplace_back(Move(i, n + 8));
                    n = n + 8;
                }
                if (n + 8 < 64 && squares[n + 8] >= B_PAWN) {
                    moves.emplace_back(Move(i, n + 8, (uint8_t)squares[n + 8]));
                }
                n = i;

                //down
                while (n >= 8 && squares[n - 8] == EMPTY) {
                    moves.emplace_back(Move(i, n - 8));
                    n = n - 8;
                }
                if (n >= 8 && squares[n - 8] >= B_PAWN) {
                    moves.emplace_back(Move(i, n - 8, (uint8_t)squares[n - 8]));
                }
                n = i;

                //left
                while (n % 8 != 0 && squares[n - 1] == EMPTY) {
                    moves.emplace_back(Move(i, n - 1));
                    n = n - 1;
                }
                if (n % 8 != 0 && squares[n - 1] >= B_PAWN) {
                    moves.emplace_back(Move(i, n - 1, (uint8_t)squares[n - 1]));
                }
                n = i;

                //right
                while (n % 8 != 7 && squares[n + 1] == EMPTY) {
                    moves.emplace_back(Move(i, n + 1));
                    n = n + 1;
                }
                if (n % 8 != 7 && squares[n + 1] >= B_PAWN) {
                    moves.emplace_back(Move(i, n + 1, (uint8_t)squares[n + 1]));
                }
            }

            else if (p == W_QUEEN) {
                uint8_t n = i;

                // up
                while (n + 8 < 64 && squares[n + 8] == EMPTY) {
                    moves.emplace_back(Move(i, n + 8));
                    n = n + 8;
                }
                if (n + 8 < 64 && squares[n + 8] >= B_PAWN) {
                    moves.emplace_back(Move(i, n + 8, (uint8_t)squares[n + 8]));
                }
                n = i;

                //down
                while (n >= 8 && squares[n - 8] == EMPTY) {
                    moves.emplace_back(Move(i, n - 8));
                    n = n - 8;
                }
                if (n >= 8 && squares[n - 8] >= B_PAWN) {
                    moves.emplace_back(Move(i, n - 8, (uint8_t)squares[n - 8]));
                }
                n = i;

                //left
                while (n % 8 != 0 && squares[n - 1] == EMPTY) {
                    moves.emplace_back(Move(i, n - 1));
                    n = n - 1;
                }
                if (n % 8 != 0 && squares[n - 1] >= B_PAWN) {
                    moves.emplace_back(Move(i, n - 1, (uint8_t)squares[n - 1]));
                }
                n = i;

                //right
                while (n % 8 != 7 && squares[n + 1] == EMPTY) {
                    moves.emplace_back(Move(i, n + 1));
                    n = n + 1;
                }
                if (n % 8 != 7 && squares[n + 1] >= B_PAWN) {
                    moves.emplace_back(Move(i, n + 1, (uint8_t)squares[n + 1]));
                }
                n = i;

                //left up
                while (n + 7 < 64 && n % 8 != 0 && squares[n + 7] == EMPTY) {
                    moves.emplace_back(Move(i, n + 7));
                    n += 7;
                }
                if (n + 7 < 64 && n % 8 != 0 && squares[n + 7] >= B_PAWN)     {moves.emplace_back(Move(i, n + 7, (uint8_t)squares[n + 7]));}
                n = i;

                // right down
                while (n >= 7 && n % 8 != 7 && squares[n - 7] == EMPTY) {
                    moves.emplace_back(Move(i, n - 7));
                    n -= 7;
                }
                if (n >= 7 && n % 8 != 7 && squares[n - 7] >= B_PAWN)         {moves.emplace_back(Move(i, n - 7, (uint8_t)squares[n - 7]));}
                n = i;

                //right up
                while (n + 9 < 64 && n % 8 != 7 && squares[n + 9] == EMPTY) {
                    moves.emplace_back(Move(i, n + 9));
                    n += 9;
                }
                if (n + 9 < 64 && n % 8 != 7 && squares[n + 9] >= B_PAWN)     {moves.emplace_back(Move(i, n + 9, (uint8_t)squares[n + 9]));}
                n = i;

                //left down
                while (n >= 9 && n % 8 != 0 && squares[n - 9] == EMPTY) {
                    moves.emplace_back(Move(i, n - 9));
                    n -= 9;
                }
                if (n >= 9 && n % 8 != 0 && squares[n - 9] >= B_PAWN)         {moves.emplace_back(Move(i, n - 9, (uint8_t)squares[n - 9]));}
            }

            else if (p == W_KING) {
                //+ moves
                if (i % 8 != 7 && (squares[i + 1] == EMPTY || squares[i + 1] >= B_PAWN))    {moves.emplace_back(Move(i, i + 1, squares[i + 1]));}
                if (i % 8 != 0 && (squares[i - 1] == EMPTY || squares[i - 1] >= B_PAWN))    {moves.emplace_back(Move(i, i - 1, squares[i - 1]));}
                if (i + 8 < 64 && (squares[i + 8] == EMPTY || squares[i + 8] >= B_PAWN))    {moves.emplace_back(Move(i, i + 8, squares[i + 8]));}
                if (i >= 8     && (squares[i - 8] == EMPTY || squares[i - 8] >= B_PAWN))    {moves.emplace_back(Move(i, i - 8, squares[i - 8]));}

                //diagonal
                if (i % 8 != 7) {
                if (i <= 55 && (squares[i + 7] == EMPTY || squares[i + 7] >= B_PAWN))       {moves.emplace_back(Move(i, i + 7, squares[i + 7]));}
                if (i >= 8 && (squares[i - 9] == EMPTY || squares[i - 9] >= B_PAWN))        {moves.emplace_back(Move(i, i - 9, squares[i - 9]));}}
                if (i % 8 != 0) {
                if (i <= 55 && (squares[i + 9] == EMPTY || squares[i + 9] >= B_PAWN))       {moves.emplace_back(Move(i, i + 9, squares[i + 9]));}
                if (i >= 8 && (squares[i - 7] == EMPTY || squares[i - 7] >= B_PAWN))        {moves.emplace_back(Move(i, i - 7, squares[i - 7]));}}

                //Castle
                if (!isInCheck(WHITE)) {
                    if (whiteCanCastleKingside && squares[5] == EMPTY && squares[6] == EMPTY) {
                        moves.emplace_back(Move(i, i + 2, 0, 0, true));
                    }
                    if (whiteCanCastleQueenside && squares[3] == EMPTY && squares[2] == EMPTY && squares[1] == EMPTY) {
                        moves.emplace_back(Move(i, i - 2, 0, 0, true));
                    }
                }
            }
        }


        else {
            if (p == B_PAWN) {

                if (enPassantSquare != 64 && i % 8 != 7 && i - 7 == enPassantSquare) {
                    moves.emplace_back(Move(i, enPassantSquare, (uint8_t)squares[enPassantSquare + 8], 0, false, true));
                }
                if (enPassantSquare != 64 && i % 8 != 0 && i - 9 == enPassantSquare) {
                    moves.emplace_back(Move(i, enPassantSquare, (uint8_t)squares[enPassantSquare + 8], 0, false, true));
                }

                // last row (promotion)
                if (i >= 8 && i <= 15) {
                    if (squares[i - 8] == EMPTY) {
                        for (uint8_t j = 8; j < 12; j += 1) {
                            moves.emplace_back(Move(i, i - 8, 0, j));
                        }
                    }
                    if (i % 8 != 7 && squares[i - 7] != EMPTY && squares[i - 7] < B_PAWN) {
                        for (uint8_t j = 8; j < 12; j += 1) {
                            moves.emplace_back(Move(i, i - 7, (uint8_t)squares[i - 7], j));
                        }
                    }
                    if (i % 8 != 0 && squares[i - 9] != EMPTY && squares[i - 9] < B_PAWN) {
                        for (uint8_t j = 8; j < 12; j += 1) {
                            moves.emplace_back(Move(i, i - 9, (uint8_t)squares[i - 9], j));
                        }
                    }
                }
                // all other rows
                else {
                    if (squares[i - 8] == EMPTY) {
                        moves.emplace_back(Move(i, i - 8));
                        if (i >= 48 && i <= 55 && squares[i - 16] == EMPTY) {
                            moves.emplace_back(Move(i, i - 16));
                        }
                    }
                    if (i % 8 != 7 && squares[i - 7] != EMPTY && squares[i - 7] < B_PAWN) {
                        moves.emplace_back(Move(i, i - 7, (uint8_t)squares[i - 7]));
                    }
                    if (i % 8 != 0 && squares[i - 9] != EMPTY && squares[i - 9] < B_PAWN) {
                        moves.emplace_back(Move(i, i - 9, (uint8_t)squares[i - 9]));
                    }
                }
            }

            else if (p == B_KNIGHT) {
                //forward
                if (i % 8 != 0 && i + 15 < 64 && (squares[i + 15] == EMPTY || squares[i + 15] < B_PAWN))   {moves.emplace_back(Move(i, i + 15, (uint8_t)squares[i + 15]));}
                if (i % 8 != 7 && i + 17 < 64 && (squares[i + 17] == EMPTY || squares[i + 17] < B_PAWN))   {moves.emplace_back(Move(i, i + 17, (uint8_t)squares[i + 17]));}

                //left
                if (i % 8 >= 2 && i >= 10 && (squares[i - 10] == EMPTY || squares[i - 10] < B_PAWN))       {moves.emplace_back(Move(i, i - 10, (uint8_t)squares[i - 10]));}
                if (i % 8 >= 2 && i + 6 < 64 && (squares[i + 6] == EMPTY || squares[i + 6] < B_PAWN))      {moves.emplace_back(Move(i, i + 6, (uint8_t)squares[i + 6]));}

                //down
                if (i % 8 != 7 && i >= 15 && (squares[i - 15] == EMPTY || squares[i - 15] < B_PAWN))       {moves.emplace_back(Move(i, i - 15, (uint8_t)squares[i - 15]));}
                if (i % 8 != 0 && i >= 17 && (squares[i - 17] == EMPTY || squares[i - 17] < B_PAWN))       {moves.emplace_back(Move(i, i - 17, (uint8_t)squares[i - 17]));}

                //right
                if (i % 8 <= 5 && i + 10 < 64 && (squares[i + 10] == EMPTY || squares[i + 10] < B_PAWN))   {moves.emplace_back(Move(i, i + 10, (uint8_t)squares[i + 10]));}
                if (i % 8 <= 5 && i >= 6 && (squares[i - 6] == EMPTY || squares[i - 6] < B_PAWN))          {moves.emplace_back(Move(i, i - 6, (uint8_t)squares[i - 6]));}
            }

            else if (p == B_BISHOP) {
                uint8_t n = i;

                //left up
                while (n + 7 < 64 && n % 8 != 0 && squares[n + 7] == EMPTY) {
                    moves.emplace_back(Move(i, n + 7));
                    n += 7;
                }
                if (n + 7 < 64 && n % 8 != 0 && squares[n + 7] != EMPTY && squares[n + 7] < B_PAWN)   {moves.emplace_back(Move(i, n + 7, (uint8_t)squares[n + 7]));}
                n = i;

                // right down
                while (n >= 7 && n % 8 != 7 && squares[n - 7] == EMPTY) {
                    moves.emplace_back(Move(i, n - 7));
                    n -= 7;
                }
                if (n >= 7 && n % 8 != 7 && squares[n - 7] != EMPTY && squares[n - 7] < B_PAWN)       {moves.emplace_back(Move(i, n - 7, (uint8_t)squares[n - 7]));}
                n = i;

                //right up
                while (n + 9 < 64 && n % 8 != 7 && squares[n + 9] == EMPTY) {
                    moves.emplace_back(Move(i, n + 9));
                    n += 9;
                }
                if (n + 9 < 64 && n % 8 != 7 && squares[n + 9] != EMPTY && squares[n + 9] < B_PAWN)   {moves.emplace_back(Move(i, n + 9, (uint8_t)squares[n + 9]));}
                n = i;

                //left down
                while (n >= 9 && n % 8 != 0 && squares[n - 9] == EMPTY) {
                    moves.emplace_back(Move(i, n - 9));
                    n -= 9;
                }
                if (n >= 9 && n % 8 != 0 && squares[n - 9] != EMPTY && squares[n - 9] < B_PAWN)       {moves.emplace_back(Move(i, n - 9, (uint8_t)squares[n - 9]));}
            }

            else if (p == B_ROOK) {
                uint8_t n = i;

                // up
                while (n + 8 < 64 && squares[n + 8] == EMPTY) {
                    moves.emplace_back(Move(i, n + 8));
                    n = n + 8;
                }
                if (n + 8 < 64 && squares[n + 8] < B_PAWN && squares[n + 8] != EMPTY) {
                    moves.emplace_back(Move(i, n + 8, (uint8_t)squares[n + 8]));
                }
                n = i;

                // down
                while (n >= 8 && squares[n - 8] == EMPTY) {
                    moves.emplace_back(Move(i, n - 8));
                    n = n - 8;
                }
                if (n >= 8 && squares[n - 8] < B_PAWN && squares[n - 8] != EMPTY) {
                    moves.emplace_back(Move(i, n - 8, (uint8_t)squares[n - 8]));
                }
                n = i;

                // left
                while (n % 8 != 0 && squares[n - 1] == EMPTY) {
                    moves.emplace_back(Move(i, n - 1));
                    n = n - 1;
                }
                if (n % 8 != 0 && squares[n - 1] < B_PAWN && squares[n - 1] != EMPTY) {
                    moves.emplace_back(Move(i, n - 1, (uint8_t)squares[n - 1]));
                }
                n = i;

                // right
                while (n % 8 != 7 && squares[n + 1] == EMPTY) {
                    moves.emplace_back(Move(i, n + 1));
                    n = n + 1;
                }
                if (n % 8 != 7 && squares[n + 1] < B_PAWN && squares[n + 1] != EMPTY) {
                    moves.emplace_back(Move(i, n + 1, (uint8_t)squares[n + 1]));
                }
            }

            else if (p == B_QUEEN) {
                uint8_t n = i;

                // up
                while (n + 8 < 64 && squares[n + 8] == EMPTY) {
                    moves.emplace_back(Move(i, n + 8));
                    n = n + 8;
                }
                if (n + 8 < 64 && squares[n + 8] != EMPTY && squares[n + 8] < B_PAWN) {
                    moves.emplace_back(Move(i, n + 8, (uint8_t)squares[n + 8]));
                }
                n = i;

                //down
                while (n >= 8 && squares[n - 8] == EMPTY) {
                    moves.emplace_back(Move(i, n - 8));
                    n = n - 8;
                }
                if (n >= 8 && squares[n - 8] != EMPTY && squares[n - 8] < B_PAWN) {
                    moves.emplace_back(Move(i, n - 8, (uint8_t)squares[n - 8]));
                }
                n = i;

                //left
                while (n % 8 != 0 && squares[n - 1] == EMPTY) {
                    moves.emplace_back(Move(i, n - 1));
                    n = n - 1;
                }
                if (n % 8 != 0 && squares[n - 1] != EMPTY && squares[n - 1] < B_PAWN) {
                    moves.emplace_back(Move(i, n - 1, (uint8_t)squares[n - 1]));
                }
                n = i;

                //right
                while (n % 8 != 7 && squares[n + 1] == EMPTY) {
                    moves.emplace_back(Move(i, n + 1));
                    n = n + 1;
                }
                if (n % 8 != 7 && squares[n + 1] != EMPTY && squares[n + 1] < B_PAWN) {
                    moves.emplace_back(Move(i, n + 1, (uint8_t)squares[n + 1]));
                }
                n = i;

                //left up
                while (n + 7 < 64 && n % 8 != 0 && squares[n + 7] == EMPTY) {
                    moves.emplace_back(Move(i, n + 7));
                    n += 7;
                }
                if (n + 7 < 64 && n % 8 != 0 && squares[n + 7] != EMPTY && squares[n + 7] < B_PAWN)   {moves.emplace_back(Move(i, n + 7, (uint8_t)squares[n + 7]));}
                n = i;

                // right down
                while (n >= 7 && n % 8 != 7 && squares[n - 7] == EMPTY) {
                    moves.emplace_back(Move(i, n - 7));
                    n -= 7;
                }
                if (n >= 7 && n % 8 != 7 && squares[n - 7] != EMPTY && squares[n - 7] < B_PAWN)       {moves.emplace_back(Move(i, n - 7, (uint8_t)squares[n - 7]));}
                n = i;

                //right up
                while (n + 9 < 64 && n % 8 != 7 && squares[n + 9] == EMPTY) {
                    moves.emplace_back(Move(i, n + 9));
                    n += 9;
                }
                if (n + 9 < 64 && n % 8 != 7 && squares[n + 9] != EMPTY && squares[n + 9] < B_PAWN)   {moves.emplace_back(Move(i, n + 9, (uint8_t)squares[n + 9]));}
                n = i;

                //left down
                while (n >= 9 && n % 8 != 0 && squares[n - 9] == EMPTY) {
                    moves.emplace_back(Move(i, n - 9));
                    n -= 9;
                }
                if (n >= 9 && n % 8 != 0 && squares[n - 9] != EMPTY && squares[n - 9] < B_PAWN)       {moves.emplace_back(Move(i, n - 9, (uint8_t)squares[n - 9]));}
            }

            else if (p == B_KING) {
                //+ moves
                if (i % 8 != 7 && (squares[i + 1] == EMPTY || squares[i + 1] < B_PAWN))    {moves.emplace_back(Move(i, i + 1, squares[i + 1]));}
                if (i % 8 != 0 && (squares[i - 1] == EMPTY || squares[i - 1] < B_PAWN))    {moves.emplace_back(Move(i, i - 1, squares[i - 1]));}
                if (i + 8 < 64 && (squares[i + 8] == EMPTY || squares[i + 8] < B_PAWN))    {moves.emplace_back(Move(i, i + 8, squares[i + 8]));}
                if (i >= 8     && (squares[i - 8] == EMPTY || squares[i - 8] < B_PAWN))    {moves.emplace_back(Move(i, i - 8, squares[i - 8]));}

                //diagonal
                if (i % 8 != 7) {
                if (i <= 55 && (squares[i + 7] == EMPTY || squares[i + 7] < B_PAWN))       {moves.emplace_back(Move(i, i + 7, squares[i + 7]));}
                if (i >= 8  && (squares[i - 9] == EMPTY || squares[i - 9] < B_PAWN))       {moves.emplace_back(Move(i, i - 9, squares[i - 9]));}}

                if (i % 8 != 0) {
                if (i <= 55 && (squares[i + 9] == EMPTY || squares[i + 9] < B_PAWN))       {moves.emplace_back(Move(i, i + 9, squares[i + 9]));}
                if (i >= 8  && (squares[i - 7] == EMPTY || squares[i - 7] < B_PAWN))       {moves.emplace_back(Move(i, i - 7, squares[i - 7]));}}

                //Castle
                if (!isInCheck(BLACK)) {
                    if (blackCanCastleKingside && squares[61] == EMPTY && squares[62] == EMPTY) {
                        moves.emplace_back(Move(i, i + 2, 0, 0, true));
                    }
                    if (blackCanCastleQueenside && squares[59] == EMPTY && squares[58] == EMPTY && squares[57] == EMPTY) {
                        moves.emplace_back(Move(i, i - 2, 0, 0, true));
                    }
                }
            }
        }

    }

    std::vector<Move> legal;
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