#include "TetrisGame.h"
#include <QFont>
#include <QString>
#include <algorithm>
#include <random>
#include <ctime>

// ── Piece shape definitions ──────────────────────────────────────────────────
// Each shape[type] is a 4×4 grid. 1 = filled cell.
//   type 0=I  1=O  2=T  3=S  4=Z  5=J  6=L
const int TetrisGame::SHAPE[7][4][4] = {
    // I
    { {0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0} },
    // O
    { {0,0,0,0},{0,1,1,0},{0,1,1,0},{0,0,0,0} },
    // T
    { {0,0,0,0},{0,1,0,0},{1,1,1,0},{0,0,0,0} },
    // S
    { {0,0,0,0},{0,1,1,0},{1,1,0,0},{0,0,0,0} },
    // Z
    { {0,0,0,0},{1,1,0,0},{0,1,1,0},{0,0,0,0} },
    // J
    { {0,0,0,0},{1,0,0,0},{1,1,1,0},{0,0,0,0} },
    // L
    { {0,0,0,0},{0,0,1,0},{1,1,1,0},{0,0,0,0} },
};

// ── Helper: rotate a 4×4 grid 90° clockwise ─────────────────────────────────
static void rotateCW(const int src[4][4], int dst[4][4])
{
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            dst[c][3 - r] = src[r][c];
}

// ── Helper: pick a random Tetromino type ────────────────────────────────────
static TetroType randomType()
{
    static bool seeded = false;
    if (!seeded) {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
        seeded = true;
    }
    return static_cast<TetroType>(std::rand() % 7);
}

// ── Constructor ──────────────────────────────────────────────────────────────
TetrisGame::TetrisGame(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(500, 700);
    setWindowTitle("Tetris");

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TetrisGame::tick);
    m_timer->setInterval(500);

    m_nextType = randomType();
    spawnPiece();

    m_timer->start();
    setFocusPolicy(Qt::StrongFocus);
}

// ── Access shape grid ────────────────────────────────────────────────────────
const int (&TetrisGame::getShape(TetroType t, int rot) const)[4][4]
{
    // Compute the rotated shape on the fly from the base definition.
    // We store the result in a thread-local static cache to avoid reallocation.
    static int rotated[4][4];
    const int (&base)[4][4] = SHAPE[static_cast<int>(t)];

    int tmp[4][4];
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            tmp[r][c] = base[r][c];

    for (int i = 0; i < rot; ++i) {
        rotateCW(tmp, rotated);
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                tmp[r][c] = rotated[r][c];
    }

    // Copy final result into the static buffer via the reference.
    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            rotated[r][c] = tmp[r][c];

    return rotated;
}

// ── Collision check ──────────────────────────────────────────────────────────
bool TetrisGame::isValid(TetroType t, int row, int col, int rot) const
{
    const auto &shape = getShape(t, rot);
    for (int r = 0; r < PIECE_SIZE; ++r) {
        for (int c = 0; c < PIECE_SIZE; ++c) {
            if (shape[r][c]) {
                int br = row + r;
                int bc = col + c;
                // Out of bounds
                if (bc < 0 || bc >= BOARD_COLS || br >= TOTAL_ROWS)
                    return false;
                // Above the top is ok
                if (br < 0)
                    continue;
                // Collision with locked block
                if (m_board[br][bc] != 0)
                    return false;
            }
        }
    }
    return true;
}

// ── Spawn new piece ──────────────────────────────────────────────────────────
void TetrisGame::spawnPiece()
{
    m_currentType = m_nextType;
    m_nextType = randomType();
    m_rotation = 0;
    m_currentRow = HIDDEN_ROWS - 1;   // start partially hidden
    m_currentCol = (BOARD_COLS - PIECE_SIZE) / 2;

    if (!isValid(m_currentType, m_currentRow, m_currentCol, m_rotation)) {
        m_gameOver = true;
        m_timer->stop();
    }
}

// ── Lock piece into board ────────────────────────────────────────────────────
void TetrisGame::lockPiece()
{
    const auto &shape = getShape(m_currentType, m_rotation);
    for (int r = 0; r < PIECE_SIZE; ++r) {
        for (int c = 0; c < PIECE_SIZE; ++c) {
            if (shape[r][c]) {
                int br = m_currentRow + r;
                int bc = m_currentCol + c;
                if (br >= 0 && br < TOTAL_ROWS && bc >= 0 && bc < BOARD_COLS)
                    m_board[br][bc] = static_cast<int>(m_currentType) + 1;
            }
        }
    }

    int cleared = clearLines();
    if (cleared == 1) m_score += 100;
    else if (cleared == 2) m_score += 300;
    else if (cleared == 3) m_score += 500;
    else if (cleared >= 4) m_score += 800;

    spawnPiece();
    update();
}

// ── Clear completed lines ────────────────────────────────────────────────────
int TetrisGame::clearLines()
{
    int cleared = 0;
    for (int r = TOTAL_ROWS - 1; r >= 0; ) {
        bool full = true;
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_board[r][c] == 0) {
                full = false;
                break;
            }
        }
        if (full) {
            // Shift everything down
            for (int rr = r; rr > 0; --rr)
                for (int c = 0; c < BOARD_COLS; ++c)
                    m_board[rr][c] = m_board[rr-1][c];
            for (int c = 0; c < BOARD_COLS; ++c)
                m_board[0][c] = 0;
            ++cleared;
            // Stay on same row index since rows shifted down
        } else {
            --r;
        }
    }
    return cleared;
}

// ── Game tick ────────────────────────────────────────────────────────────────
void TetrisGame::tick()
{
    if (m_gameOver) return;

    // Try to move down
    if (isValid(m_currentType, m_currentRow + 1, m_currentCol, m_rotation)) {
        ++m_currentRow;
    } else {
        lockPiece();
    }
    update();
}

// ── Key input ────────────────────────────────────────────────────────────────
void TetrisGame::keyPressEvent(QKeyEvent *event)
{
    if (m_gameOver) {
        if (event->key() == Qt::Key_Space || event->key() == Qt::Key_Return) {
            // Reset game
            for (int r = 0; r < TOTAL_ROWS; ++r)
                for (int c = 0; c < BOARD_COLS; ++c)
                    m_board[r][c] = 0;
            m_score = 0;
            m_gameOver = false;
            m_nextType = randomType();
            spawnPiece();
            m_timer->start();
            update();
        }
        return;
    }

    switch (event->key()) {
    case Qt::Key_Left:
        if (isValid(m_currentType, m_currentRow, m_currentCol - 1, m_rotation))
            --m_currentCol;
        break;
    case Qt::Key_Right:
        if (isValid(m_currentType, m_currentRow, m_currentCol + 1, m_rotation))
            ++m_currentCol;
        break;
    case Qt::Key_Down:
        if (isValid(m_currentType, m_currentRow + 1, m_currentCol, m_rotation))
            ++m_currentRow;
        break;
    case Qt::Key_Up: {
        int newRot = (m_rotation + 1) % 4;
        if (isValid(m_currentType, m_currentRow, m_currentCol, newRot))
            m_rotation = newRot;
        break;
    }
    case Qt::Key_Space:
        // Hard drop
        while (isValid(m_currentType, m_currentRow + 1, m_currentCol, m_rotation))
            ++m_currentRow;
        lockPiece();
        break;
    default:
        break;
    }
    update();
}

// ── Paint ────────────────────────────────────────────────────────────────────
void TetrisGame::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    int cellSize = 30;
    int boardX = 30;
    int boardY = 30;
    int boardW = BOARD_COLS * cellSize;
    int boardH = BOARD_ROWS * cellSize;

    // ── Background ───────────────────────────────────────────────────────
    p.fillRect(rect(), QColor(30, 30, 30));

    // ── Board background ──────────────────────────────────────────────────
    p.setPen(QPen(QColor(80, 80, 80), 2));
    p.setBrush(QColor(20, 20, 20));
    p.drawRect(boardX - 2, boardY - 2, boardW + 4, boardH + 4);
    p.fillRect(boardX, boardY, boardW, boardH, QColor(20, 20, 20));

    // ── Grid lines ────────────────────────────────────────────────────────
    p.setPen(QColor(40, 40, 40));
    for (int c = 1; c < BOARD_COLS; ++c) {
        int x = boardX + c * cellSize;
        p.drawLine(x, boardY, x, boardY + boardH);
    }
    for (int r = 1; r < BOARD_ROWS; ++r) {
        int y = boardY + r * cellSize;
        p.drawLine(boardX, y, boardX + boardW, y);
    }

    // ── Locked blocks ─────────────────────────────────────────────────────
    for (int r = HIDDEN_ROWS; r < TOTAL_ROWS; ++r) {
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_board[r][c] != 0) {
                int typeIdx = m_board[r][c] - 1;
                QColor col = TETRO_COLORS[typeIdx];
                drawBlock(p, r - HIDDEN_ROWS, c, col);
            }
        }
    }

    // ── Current falling piece ─────────────────────────────────────────────
    if (!m_gameOver && m_currentType != TetroType::None) {
        const auto &shape = getShape(m_currentType, m_rotation);
        QColor col = TETRO_COLORS[static_cast<int>(m_currentType)];
        for (int r = 0; r < PIECE_SIZE; ++r) {
            for (int c = 0; c < PIECE_SIZE; ++c) {
                if (shape[r][c]) {
                    int br = m_currentRow + r - HIDDEN_ROWS;
                    int bc = m_currentCol + c;
                    if (br >= 0)
                        drawBlock(p, br, bc, col);
                }
            }
        }
    }

    // ── Next piece preview ────────────────────────────────────────────────
    drawPreview(p);

    // ── Score ─────────────────────────────────────────────────────────────
    drawScore(p);

    // ── Game Over overlay ─────────────────────────────────────────────────
    if (m_gameOver)
        drawGameOver(p);
}

// ── Draw single cell ─────────────────────────────────────────────────────────
void TetrisGame::drawBlock(QPainter &p, int row, int col, const QColor &color)
{
    int cellSize = 30;
    int x = 30 + col * cellSize;
    int y = 30 + row * cellSize;
    int m = 1;  // margin for the "gap" effect

    // Main fill with gradient-like highlight
    p.setPen(Qt::NoPen);
    p.setBrush(color);
    p.drawRect(x + m, y + m, cellSize - 2*m, cellSize - 2*m);

    // Lighter top-left highlight
    QColor light = color.lighter(150);
    p.setPen(QPen(light, 1));
    p.drawLine(x + m, y + m, x + cellSize - m, y + m);           // top
    p.drawLine(x + m, y + m, x + m, y + cellSize - m);           // left

    // Darker bottom-right shadow
    QColor dark = color.darker(150);
    p.setPen(QPen(dark, 1));
    p.drawLine(x + cellSize - m, y + m, x + cellSize - m, y + cellSize - m); // right
    p.drawLine(x + m, y + cellSize - m, x + cellSize - m, y + cellSize - m); // bottom
}

// ── Next piece preview ───────────────────────────────────────────────────────
void TetrisGame::drawPreview(QPainter &p)
{
    int previewX = 340;
    int previewY = 40;
    int cellSize = 25;
    int previewSize = PIECE_SIZE * cellSize;

    // Background box
    p.setPen(QPen(QColor(80, 80, 80), 2));
    p.setBrush(QColor(40, 40, 40));
    p.drawRect(previewX, previewY, previewSize + 20, previewSize + 20);

    if (m_nextType == TetroType::None) return;

    // Title
    p.setPen(QColor(200, 200, 200));
    QFont titleFont("Arial", 12, QFont::Bold);
    p.setFont(titleFont);
    p.drawText(previewX, previewY - 10, 100, 20, Qt::AlignLeft, "NEXT");

    // Draw the next piece centered in the preview box
    const auto &shape = getShape(m_nextType, 0);
    QColor col = TETRO_COLORS[static_cast<int>(m_nextType)];

    int startX = previewX + 10 + (PIECE_SIZE - 4) * cellSize / 2; // N/A since all are 4x4
    int startY = previewY + 10;

    for (int r = 0; r < PIECE_SIZE; ++r) {
        for (int c = 0; c < PIECE_SIZE; ++c) {
            if (shape[r][c]) {
                int x = startX + c * cellSize;
                int y = startY + r * cellSize;
                p.setPen(Qt::NoPen);
                p.setBrush(col);
                p.drawRect(x + 1, y + 1, cellSize - 2, cellSize - 2);

                QColor light = col.lighter(150);
                p.setPen(QPen(light, 1));
                p.drawLine(x + 1, y + 1, x + cellSize - 1, y + 1);
                p.drawLine(x + 1, y + 1, x + 1, y + cellSize - 1);

                QColor dark = col.darker(150);
                p.setPen(QPen(dark, 1));
                p.drawLine(x + cellSize - 1, y + 1, x + cellSize - 1, y + cellSize - 1);
                p.drawLine(x + 1, y + cellSize - 1, x + cellSize - 1, y + cellSize - 1);
            }
        }
    }
}

// ── Score ────────────────────────────────────────────────────────────────────
void TetrisGame::drawScore(QPainter &p)
{
    int x = 340;
    int y = 200;

    p.setPen(QColor(200, 200, 200));
    QFont labelFont("Arial", 12, QFont::Bold);
    p.setFont(labelFont);
    p.drawText(x, y, 150, 20, Qt::AlignLeft, "SCORE");

    QFont scoreFont("Arial", 32, QFont::Bold);
    p.setFont(scoreFont);
    p.setPen(QColor(255, 255, 255));
    p.drawText(x, y + 25, 150, 40, Qt::AlignLeft, QString::number(m_score));

    // Controls hint
    p.setPen(QColor(140, 140, 140));
    QFont hintFont("Arial", 10);
    p.setFont(hintFont);
    int hy = 300;
    p.drawText(x, hy, 150, 20, Qt::AlignLeft, "CONTROLS");
    p.setPen(QColor(100, 100, 100));
    p.drawText(x, hy + 20, 150, 120, Qt::AlignLeft,
               "Left/Right: Move\n"
               "Down: Soft drop\n"
               "Up: Rotate\n"
               "Space: Hard drop\n"
               "Enter: Restart");
}

// ── Game Over overlay ────────────────────────────────────────────────────────
void TetrisGame::drawGameOver(QPainter &p)
{
    p.fillRect(rect(), QColor(0, 0, 0, 160));

    p.setPen(QColor(255, 50, 50));
    QFont goFont("Arial", 36, QFont::Bold);
    p.setFont(goFont);
    p.drawText(rect(), Qt::AlignCenter, "GAME OVER");

    p.setPen(QColor(200, 200, 200));
    QFont restartFont("Arial", 14);
    p.setFont(restartFont);
    QRect r = rect();
    p.drawText(r.adjusted(0, 80, 0, 0), Qt::AlignCenter, "Press ENTER to restart");
}
