#pragma once

#include "Board.h"
#include "Move.h"

struct SearchResult {
    Move bestMove;
    int score;
    uint8_t depth;
};

SearchResult search(Board& board, uint8_t depth);
