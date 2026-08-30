# CedarMate

A chess engine written from scratch in C++20, playable from the terminal.

## Build

```sh
make          # builds ./program
make clean
```

Needs `clang++` with C++20 support — change `CXX` in the `Makefile` to use `g++`.

## Play

```sh
./program
```

You play White. Enter a move as two squares separated by a space:

```
Your move: e2 e4
Engine plays: d7 -> d5 (score: -80)
```

| Input   | Effect                          |
| ------- | ------------------------------- |
| `e2 e4` | move the piece on e2 to e4      |
| `print` | draw the current board          |
| `quit`  | exit                            |

Scores are in centipawns from White's point of view, so `-80` means the engine
thinks Black is a little better than three quarters of a pawn ahead.

## Layout

| File                    | Role                                                    |
| ----------------------- | ------------------------------------------------------- |
| `Board.h` / `Board.cpp` | board state, make/undo, legal move generation           |
| `Move.h`                | one move: from, to, capture, promotion, castle, en passant |
| `Eval.h` / `Eval.cpp`   | static evaluation — material plus piece-square tables   |
| `Search.h` / `Search.cpp` | negamax search with alpha-beta pruning                |
| `main.cpp`              | terminal game loop                                      |

## How it works

The board is a flat 64-entry array where square 0 is a1 and square 63 is h8, so
a square's file is `sq % 8` and its rank is `sq / 8`. Moves are offsets on that
index: `+8` is one rank up, `+1` one file right, `+7` and `+9` the diagonals.
Because the array wraps, every generator checks that a step does not change the
file by more than it should — that is what stops a rook on h4 sliding onto a5.

`generateLegalMoves()` walks the 64 squares and dispatches each piece of the
side to move to its own generator (`generatePawnMoves`, `generateKnightMoves`,
and so on). Those produce *pseudo-legal* moves; the function then plays each
one, keeps it only if the mover's king is not left in check, and takes it back.

Knights and kings share `generateStepMoves`, and bishops, rooks and queens
share `generateSlidingMoves` — each is driven by a small table of offsets, so
the queen is just the rook's offsets plus the bishop's.

`search()` runs negamax with alpha-beta pruning to a fixed depth, set by
`DEPTH` in `main.cpp` (currently 6).

## Known limitations

- Castling checks that the squares between king and rook are empty and that the
  king is not currently in check, but not that it passes *through* an attacked
  square, nor that the rook is still on its home square.
- `undoMove` does not fully restore state after a black promotion.
- You cannot pick a promotion piece from the terminal — the first generated
  promotion wins, which is a knight.
