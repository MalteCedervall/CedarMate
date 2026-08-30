#include "Board.h"
#include <iostream>
#include <cassert>

int passed = 0;
int failed = 0;

void check(bool cond, const char* msg) {
    if (cond) { std::cout << "  PASS: " << msg << "\n"; passed++; }
    else       { std::cout << "  FAIL: " << msg << "\n"; failed++; }
}

// Take a snapshot of the board and compare after undo
struct Snapshot {
    Piece sq[64];
    Color side;
    uint8_t ep;
    bool wck, wcq, bck, bcq;
    uint8_t wk, bk;
};

Snapshot snap(Board& b) {
    Snapshot s;
    for (int i = 0; i < 64; i++) s.sq[i] = b.getPiece(i);
    s.side = b.getSideToMove();
    s.wk = b.getWhiteKingSquare();
    s.bk = b.getBlackKingSquare();
    return s;
}

bool snapEq(Board& b, const Snapshot& s) {
    for (int i = 0; i < 64; i++) if (b.getPiece(i) != s.sq[i]) return false;
    if (b.getSideToMove() != s.side) return false;
    if (b.getWhiteKingSquare() != s.wk) return false;
    if (b.getBlackKingSquare() != s.bk) return false;
    return true;
}

int main() {
    // --- Test 1: white pawn single push ---
    std::cout << "Test 1: white pawn single push\n";
    { Board b; auto s = snap(b); UndoInfo u(0,0);
      b.makeMove(Move(12, 20), u); // e2->e3
      check(b.getPiece(20) == W_PAWN, "e3=W_PAWN");
      check(b.getPiece(12) == EMPTY,  "e2=EMPTY");
      check(b.getSideToMove() == BLACK, "side=BLACK");
      b.undoMove(u);
      check(snapEq(b, s), "undo restores state"); }

    // --- Test 2: white pawn double push (en passant square) ---
    std::cout << "Test 2: white pawn double push\n";
    { Board b; auto s = snap(b); UndoInfo u(0,0);
      b.makeMove(Move(12, 28), u); // e2->e4
      check(b.getPiece(28) == W_PAWN, "e4=W_PAWN");
      check(b.getPiece(12) == EMPTY,  "e2=EMPTY");
      b.undoMove(u);
      check(snapEq(b, s), "undo restores state"); }

    // --- Test 3: black pawn single push ---
    std::cout << "Test 3: black pawn single push\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12, 28), u); // e2->e4 (white)
      auto s = snap(b); UndoInfo u2(0,0);
      b.makeMove(Move(52, 44), u2); // e7->e6
      check(b.getPiece(44) == B_PAWN, "e6=B_PAWN");
      check(b.getPiece(52) == EMPTY,  "e7=EMPTY");
      b.undoMove(u2);
      check(snapEq(b, s), "undo restores state"); }

    // --- Test 4: capture ---
    std::cout << "Test 4: capture\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12, 28), u); // e2->e4
      UndoInfo u2(0,0);
      b.makeMove(Move(51, 35), u2); // d7->d5
      auto s = snap(b); UndoInfo u3(0,0);
      b.makeMove(Move(28, 35, B_PAWN), u3); // e4xd5
      check(b.getPiece(35) == W_PAWN, "d5=W_PAWN after capture");
      check(b.getPiece(28) == EMPTY,  "e4=EMPTY");
      b.undoMove(u3);
      check(b.getPiece(35) == B_PAWN, "d5=B_PAWN restored");
      check(b.getPiece(28) == W_PAWN, "e4=W_PAWN restored");
      check(snapEq(b, s), "undo restores full state"); }

    // --- Test 5: white en passant ---
    std::cout << "Test 5: white en passant\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12, 28), u); // e2->e4
      b.makeMove(Move(48, 32), u); // a7->a5
      b.makeMove(Move(28, 36), u); // e4->e5
      b.makeMove(Move(51, 35), u); // d7->d5
      auto s = snap(b); UndoInfo uep(0,0);
      b.makeMove(Move(36, 43, B_PAWN, 0, false, true), uep); // e5xd6 e.p.
      check(b.getPiece(43) == W_PAWN, "d6=W_PAWN after e.p.");
      check(b.getPiece(35) == EMPTY,  "d5=EMPTY (captured pawn gone)");
      check(b.getPiece(36) == EMPTY,  "e5=EMPTY");
      b.undoMove(uep);
      check(b.getPiece(35) == B_PAWN, "d5=B_PAWN restored");
      check(b.getPiece(36) == W_PAWN, "e5=W_PAWN restored");
      check(b.getPiece(43) == EMPTY,  "d6=EMPTY restored");
      check(snapEq(b, s), "undo restores full state"); }

    // --- Test 6: black en passant ---
    std::cout << "Test 6: black en passant\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12, 28), u); // e2->e4
      b.makeMove(Move(55, 39), u); // h7->h5
      b.makeMove(Move(28, 36), u); // e4->e5
      b.makeMove(Move(55-8, 39-8), u); // h5->h4? no...
      // Redo: get black pawn to e4 via e7->e5->e4? No, need white to play b4 first
      // Setup: 1.e4 a5 2.e5 a4 3.b4 a4xb3 e.p.
      Board b2; UndoInfo v(0,0);
      b2.makeMove(Move(12,28), v); // e2->e4
      b2.makeMove(Move(48,32), v); // a7->a5
      b2.makeMove(Move(28,36), v); // e4->e5
      b2.makeMove(Move(32,24), v); // a5->a4
      b2.makeMove(Move(9,25), v);  // b2->b4
      auto s = snap(b2); UndoInfo uep(0,0);
      b2.makeMove(Move(24,17,W_PAWN,0,false,true), uep); // a4xb3 e.p.
      check(b2.getPiece(17) == B_PAWN, "b3=B_PAWN after e.p.");
      check(b2.getPiece(25) == EMPTY,  "b4=EMPTY (captured pawn gone)");
      check(b2.getPiece(24) == EMPTY,  "a4=EMPTY");
      b2.undoMove(uep);
      check(b2.getPiece(17) == EMPTY,  "b3=EMPTY restored");
      check(b2.getPiece(25) == W_PAWN, "b4=W_PAWN restored");
      check(b2.getPiece(24) == B_PAWN, "a4=B_PAWN restored");
      check(snapEq(b2, s), "undo restores full state"); }

    // --- Test 7: white kingside castle ---
    std::cout << "Test 7: white kingside castle\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12,28), u); // e2->e4
      b.makeMove(Move(52,44), u); // e7->e6
      b.makeMove(Move(5,12), u);  // f1->e2? no, clear f1 and g1
      // Just clear f1,g1 manually by moving pieces out
      // Easier: use starting pos after 1.e4 e5 2.Nf3 Nc6 3.Bc4 Bc5 - complex
      // Simpler: force it by making moves that clear the path
      Board b2; UndoInfo v(0,0);
      b2.makeMove(Move(13,29), v); // f2->f4
      b2.makeMove(Move(52,44), v); // e7->e6
      b2.makeMove(Move(6,21), v);  // g1->f3
      b2.makeMove(Move(51,35), v); // d7->d5
      b2.makeMove(Move(5,26), v);  // f1->c4 (bishop slides to c4? no, need empty d3,e2... )
      // Simplest: just move pieces off f1 and g1 manually
      // 1.e4 e6 2.Nf3 d5 3.Bc4 -- f1 bishop moves
      Board b3; UndoInfo w(0,0);
      b3.makeMove(Move(12,28), w); // e2->e4
      b3.makeMove(Move(52,44), w); // e7->e6
      b3.makeMove(Move(6,21), w);  // g1->f3
      b3.makeMove(Move(51,35), w); // d7->d5
      b3.makeMove(Move(5,33), w);  // f1->b5 (diagonal: f1=5, b5=33? 5+28=33, file b=1, rank5=4*8+1=33 yes)
      b3.makeMove(Move(50,42), w); // c7->c6
      auto s = snap(b3); UndoInfo uc(0,0);
      b3.makeMove(Move(4,6,0,0,true), uc); // e1->g1 castle
      check(b3.getPiece(6)  == W_KING, "g1=W_KING");
      check(b3.getPiece(5)  == W_ROOK, "f1=W_ROOK");
      check(b3.getPiece(4)  == EMPTY,  "e1=EMPTY");
      check(b3.getPiece(7)  == EMPTY,  "h1=EMPTY");
      check(b3.getWhiteKingSquare() == 6, "whiteKingSquare=g1");
      b3.undoMove(uc);
      check(b3.getPiece(4)  == W_KING, "e1=W_KING restored");
      check(b3.getPiece(7)  == W_ROOK, "h1=W_ROOK restored");
      check(b3.getPiece(6)  == EMPTY,  "g1=EMPTY restored");
      check(b3.getPiece(5)  == EMPTY,  "f1=EMPTY restored");
      check(b3.getWhiteKingSquare() == 4, "whiteKingSquare=e1 restored");
      check(snapEq(b3, s), "undo restores full state"); }

    // --- Test 8: white queenside castle ---
    std::cout << "Test 8: white queenside castle\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(11,27), u); // d2->d4
      b.makeMove(Move(52,44), u); // e7->e6
      b.makeMove(Move(3,11), u);  // d1->d2
      b.makeMove(Move(51,43), u); // d7->d6
      b.makeMove(Move(2,20), u);  // c1->e3 (c1=2, e3=20: 2+18=20, file e=4... actually c1=2, e3=rank2*8+4=20 yes)
      b.makeMove(Move(50,42), u); // c7->c6
      b.makeMove(Move(1,18), u);  // b1->c3
      b.makeMove(Move(49,41), u); // b7->b6
      auto s = snap(b); UndoInfo uc(0,0);
      b.makeMove(Move(4,2,0,0,true), uc); // e1->c1 queenside castle
      check(b.getPiece(2)  == W_KING, "c1=W_KING");
      check(b.getPiece(3)  == W_ROOK, "d1=W_ROOK");
      check(b.getPiece(4)  == EMPTY,  "e1=EMPTY");
      check(b.getPiece(0)  == EMPTY,  "a1=EMPTY");
      check(b.getWhiteKingSquare() == 2, "whiteKingSquare=c1");
      b.undoMove(uc);
      check(b.getPiece(4)  == W_KING, "e1=W_KING restored");
      check(b.getPiece(0)  == W_ROOK, "a1=W_ROOK restored");
      check(b.getPiece(2)  == EMPTY,  "c1=EMPTY restored");
      check(b.getPiece(3)  == EMPTY,  "d1=EMPTY? (was moved earlier)");
      check(b.getWhiteKingSquare() == 4, "whiteKingSquare=e1 restored");
      check(snapEq(b, s), "undo restores full state"); }

    // --- Test 9: white promotion ---
    std::cout << "Test 9: white promotion\n";
    { Board b; UndoInfo u(0,0);
      // Get a white pawn to rank 7 by moving it there via captures
      // Easier: clear path and push
      // 1.a4 b5 2.axb5 c6 3.bxc6 d5 4.cxd7 e6 5.dxc8=Q
      b.makeMove(Move(8,24), u);  // a2->a4
      b.makeMove(Move(49,33), u); // b7->b5
      b.makeMove(Move(24,33,B_PAWN), u); // a4xb5
      b.makeMove(Move(50,42), u); // c7->c6
      b.makeMove(Move(33,42,B_PAWN), u); // b5xc6
      b.makeMove(Move(51,35), u); // d7->d5? no d7=51 is still there
      b.makeMove(Move(51,43), u); // d7->d6
      b.makeMove(Move(42,51,B_PAWN), u); // c6xd7
      b.makeMove(Move(52,44), u); // e7->e6
      auto s = snap(b); UndoInfo up(0,0);
      b.makeMove(Move(51,58,B_BISHOP,W_QUEEN), up); // d7xc8=Q
      check(b.getPiece(58) == W_QUEEN, "c8=W_QUEEN after promotion");
      check(b.getPiece(51) == EMPTY,   "d7=EMPTY");
      b.undoMove(up);
      check(b.getPiece(51) == W_PAWN,  "d7=W_PAWN restored");
      check(b.getPiece(58) == B_BISHOP,"c8=B_BISHOP restored");
      check(snapEq(b, s), "undo restores full state"); }

    // --- Test 10: black promotion ---
    std::cout << "Test 10: black promotion\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12,28), u); // e2->e4
      b.makeMove(Move(55,39), u); // h7->h5
      b.makeMove(Move(13,21), u); // f2->f3
      b.makeMove(Move(39,31), u); // h5->h4
      b.makeMove(Move(14,22), u); // g2->g3
      b.makeMove(Move(31,22,W_PAWN), u); // h4xg3
      b.makeMove(Move(15,31), u); // h2->h4? h2=15, h4=31
      b.makeMove(Move(22,13,W_PAWN), u); // g3xf2
      b.makeMove(Move(8,16), u);  // a2->a3
      auto s = snap(b); UndoInfo up(0,0);
      b.makeMove(Move(13,5,W_ROOK,B_QUEEN), up); // f2xe1=q (black promotes)
      check(b.getPiece(5)  == B_QUEEN, "f1=B_QUEEN after black promotion");
      check(b.getPiece(13) == EMPTY,   "f2=EMPTY");
      b.undoMove(up);
      check(b.getPiece(13) == B_PAWN,  "f2=B_PAWN restored");
      check(b.getPiece(5)  == W_ROOK,  "f1=W_ROOK restored");
      check(snapEq(b, s), "undo restores full state"); }

    // --- Test 11: castle rights revoked on king move ---
    std::cout << "Test 11: castle rights revoked on king move\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12,28), u); b.makeMove(Move(52,44), u);
      b.makeMove(Move(6,21), u);  b.makeMove(Move(51,35), u);
      b.makeMove(Move(5,33), u);  b.makeMove(Move(50,42), u);
      // Now castle kingside available, move king instead
      b.makeMove(Move(4,5), u);   // e1->f1 king move
      b.makeMove(Move(49,41), u);
      auto s = snap(b);
      // Try to generate castle - should not exist
      auto moves = b.generateLegalMoves();
      bool foundCastle = false;
      for (auto& m : moves) if (m.isCastle) { foundCastle = true; break; }
      check(!foundCastle, "no castle after king moved"); }

    // --- Test 12: castle rights revoked on rook move ---
    std::cout << "Test 12: castle rights revoked on rook move\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(12,28), u); b.makeMove(Move(52,44), u);
      b.makeMove(Move(6,21), u);  b.makeMove(Move(51,35), u);
      b.makeMove(Move(5,33), u);  b.makeMove(Move(50,42), u);
      b.makeMove(Move(7,6), u);   // h1->g1 rook move (clears h1)
      b.makeMove(Move(49,41), u);
      b.makeMove(Move(6,7), u);   // g1->h1 rook back
      b.makeMove(Move(48,40), u);
      auto moves = b.generateLegalMoves();
      bool foundKingside = false;
      for (auto& m : moves) if (m.isCastle && m.to == 6) { foundKingside = true; break; }
      check(!foundKingside, "no kingside castle after h1 rook moved"); }

    // --- Test 13: white promotion move generation has W_QUEEN, no W_PAWN ---
    std::cout << "Test 13: white promotion move generation\n";
    { Board b; UndoInfo u(0,0);
      // Advance white a-pawn to a7 via captures: a2->a4->a5->a6->a7 (clear path)
      b.makeMove(Move(8,24), u);   // a2->a4
      b.makeMove(Move(49,33), u);  // b7->b5
      b.makeMove(Move(24,33,B_PAWN), u); // a4xb5
      b.makeMove(Move(50,42), u);  // c7->c6
      b.makeMove(Move(33,42,B_PAWN), u); // b5xc6
      b.makeMove(Move(51,43), u);  // d7->d6
      b.makeMove(Move(42,51,B_PAWN), u); // c6xd7
      b.makeMove(Move(52,44), u);  // e7->e6
      // White pawn is now at d7 (square 51). It's white's turn.
      auto moves = b.generateLegalMoves();
      bool hasQueen = false, hasRook = false, hasBishop = false, hasKnight = false, hasPawn = false;
      for (auto& m : moves) {
          if (m.from == 51) {
              if (m.promoted == W_QUEEN)  hasQueen  = true;
              if (m.promoted == W_ROOK)   hasRook   = true;
              if (m.promoted == W_BISHOP) hasBishop = true;
              if (m.promoted == W_KNIGHT) hasKnight = true;
              if (m.promoted == W_PAWN)   hasPawn   = true;
          }
      }
      check(hasQueen,   "white promotion generates W_QUEEN");
      check(hasRook,    "white promotion generates W_ROOK");
      check(hasBishop,  "white promotion generates W_BISHOP");
      check(hasKnight,  "white promotion generates W_KNIGHT");
      check(!hasPawn,   "white promotion does NOT generate W_PAWN"); }

    // --- Test 14: black promotion move generation has B_QUEEN, no B_PAWN ---
    std::cout << "Test 14: black promotion move generation\n";
    { Board b; UndoInfo u(0,0);
      // Advance black h-pawn to h2 via captures
      b.makeMove(Move(12,28), u);  // e2->e4
      b.makeMove(Move(55,39), u);  // h7->h5
      b.makeMove(Move(13,21), u);  // f2->f3
      b.makeMove(Move(39,31), u);  // h5->h4
      b.makeMove(Move(14,22), u);  // g2->g3
      b.makeMove(Move(31,22,W_PAWN), u); // h4xg3
      b.makeMove(Move(15,31), u);  // h2->h4
      b.makeMove(Move(22,13,W_PAWN), u); // g3xf2
      b.makeMove(Move(8,16), u);   // a2->a3 (filler)
      // Black pawn at f2 (square 13), black's turn
      auto moves = b.generateLegalMoves();
      bool hasQueen = false, hasRook = false, hasBishop = false, hasKnight = false, hasPawn = false;
      for (auto& m : moves) {
          if (m.from == 13) {
              if (m.promoted == B_QUEEN)  hasQueen  = true;
              if (m.promoted == B_ROOK)   hasRook   = true;
              if (m.promoted == B_BISHOP) hasBishop = true;
              if (m.promoted == B_KNIGHT) hasKnight = true;
              if (m.promoted == B_PAWN)   hasPawn   = true;
          }
      }
      check(hasQueen,   "black promotion generates B_QUEEN");
      check(hasRook,    "black promotion generates B_ROOK");
      check(hasBishop,  "black promotion generates B_BISHOP");
      check(hasKnight,  "black promotion generates B_KNIGHT");
      check(!hasPawn,   "black promotion does NOT generate B_PAWN"); }

    // --- Test 15: promoted queen can move after promotion ---
    std::cout << "Test 15: promoted queen moves correctly\n";
    { Board b; UndoInfo u(0,0);
      b.makeMove(Move(8,24), u);
      b.makeMove(Move(49,33), u);
      b.makeMove(Move(24,33,B_PAWN), u);
      b.makeMove(Move(50,42), u);
      b.makeMove(Move(33,42,B_PAWN), u);
      b.makeMove(Move(51,43), u);
      b.makeMove(Move(42,51,B_PAWN), u);
      b.makeMove(Move(52,44), u);
      // Promote to W_QUEEN: d7->c8 capturing B_BISHOP (c8=58, B_BISHOP)
      UndoInfo up(0,0);
      b.makeMove(Move(51,58,B_BISHOP,W_QUEEN), up);
      check(b.getPiece(58) == W_QUEEN, "c8=W_QUEEN");
      // Queen at c8 should have legal moves
      auto moves = b.generateLegalMoves();
      bool queenMoves = false;
      for (auto& m : moves) if (m.from == 58) { queenMoves = true; break; }
      check(queenMoves, "promoted queen generates moves");
      b.undoMove(up);
      check(b.getPiece(51) == W_PAWN,   "d7=W_PAWN restored after undo");
      check(b.getPiece(58) == B_BISHOP, "c8=B_BISHOP restored after undo"); }

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed > 0 ? 1 : 0;
}
