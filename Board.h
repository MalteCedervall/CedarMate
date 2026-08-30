#pragma once

#include "Move.h"
#include <vector>
#include <string>

enum Piece: uint8_t { 
    EMPTY=0, W_PAWN, W_KNIGHT, W_BISHOP, W_ROOK, W_QUEEN, W_KING,
    B_PAWN, B_KNIGHT, B_BISHOP, B_ROOK, B_QUEEN, B_KING };


enum Color: uint8_t { WHITE, BLACK };

struct UndoInfo {
    uint8_t from;
    uint8_t to;
    uint8_t captured;
    uint8_t promoted;
    bool isCastle;
    bool isEnPassant;
    int prevEnPassantSquare;
    bool prevWhiteCanCastleKingside;
    bool prevWhiteCanCastleQueenside;
    bool prevBlackCanCastleKingside;
    bool prevBlackCanCastleQueenside;
    uint8_t prevWhiteKingSquare;
    uint8_t prevBlackKingSquare;

    UndoInfo(uint8_t from, uint8_t to,
         uint8_t captured = 0, uint8_t promoted = 0,
         bool isCastle = false, bool isEnPassant = false) {
            this->from = from;
            this->to = to;
            this->captured = captured;
            this->promoted = promoted;
            this->isCastle = isCastle;
            this->isEnPassant = isEnPassant;
            this->prevEnPassantSquare = -1;
            this->prevWhiteCanCastleKingside = false;
            this->prevWhiteCanCastleQueenside = false;
            this->prevBlackCanCastleKingside = false;
            this->prevBlackCanCastleQueenside = false;
         }
};

class Board {
    public:
        Board();
        void makeMove(const Move& move, UndoInfo& undoInfo);
        void undoMove(const UndoInfo& undoInfo);
        std::vector<Move> generateLegalMoves();
        bool isInCheck(Color color) const;
        Piece getPiece(uint8_t square) const { return squares[square]; }
        Color getSideToMove() const { return sideToMove; }
        uint8_t getWhiteKingSquare() const { return whiteKingSquare; }
        uint8_t getBlackKingSquare() const { return blackKingSquare; }

    private:
        // Pseudo-legal generators for the piece standing on `from`, which is
        // assumed to belong to the side to move. Each appends to `moves`;
        // generateLegalMoves() filters out moves that leave the king in check.
        void generatePawnMoves(uint8_t from, std::vector<Move>& moves) const;
        void generateKnightMoves(uint8_t from, std::vector<Move>& moves) const;
        void generateBishopMoves(uint8_t from, std::vector<Move>& moves) const;
        void generateRookMoves(uint8_t from, std::vector<Move>& moves) const;
        void generateQueenMoves(uint8_t from, std::vector<Move>& moves) const;
        void generateKingMoves(uint8_t from, std::vector<Move>& moves) const;

        // Shared building blocks: `deltas` are square offsets, `maxFileShift`
        // is how far a single step may move sideways before it has wrapped
        // around a board edge.
        void generateStepMoves(uint8_t from, const int* deltas, int count,
                               int maxFileShift, std::vector<Move>& moves) const;
        void generateSlidingMoves(uint8_t from, const int* deltas, int count,
                                  std::vector<Move>& moves) const;

        Piece squares[64];
        Color sideToMove;
        bool whiteCanCastleKingside;
        bool whiteCanCastleQueenside;
        bool blackCanCastleKingside;
        bool blackCanCastleQueenside;
        uint8_t enPassantSquare;
        uint8_t whiteKingSquare;
        uint8_t blackKingSquare;
};