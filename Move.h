#pragma once

#include <cstdint>

struct Move {
    uint8_t from;
    uint8_t to;
    uint8_t captured;
    uint8_t promoted;
    bool isCastle;
    bool isEnPassant;

    Move() : from(0), to(0), captured(0), promoted(0), isCastle(false), isEnPassant(false) {}

    Move(uint8_t from, uint8_t to,
         uint8_t captured = 0, uint8_t promoted = 0,
         bool isCastle = false, bool isEnPassant = false) {
            this->from = from;
            this->to = to;
            this->captured = captured;
            this->promoted = promoted;
            this->isCastle = isCastle;
            this->isEnPassant = isEnPassant;
         }

};
