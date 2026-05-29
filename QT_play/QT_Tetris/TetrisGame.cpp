#include "TetrisGame.h"
#include <QFont>
#include <QString>
#include <algorithm>

// ── Piece shape definitions ──────────────────────────────────────────────────
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

// ── 7-bag randomizer ────────────────────────────────────────────────────────
TetroType TetrisGame::nextFromBag()
{
    if (m_bag.isEmpty()) {
        m_bag = { TetroType::I, TetroType::O, TetroType::T,
                  TetroType::S, TetroType::Z, TetroType::J, TetroType::L };
        std::shuffle(m_bag.begin(), m_bag.end(), m_rng);
    }
    return m_bag.takeLast();
}

// ── Constructor ──────────────────────────────────────────────────────────────
TetrisGame::TetrisGame(QWidget *parent)
    : QWidget(parent), m_rng(std::random_device{}())
{
    setFixedSize(500, 700);
    setWindowTitle("Tetris");

    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &TetrisGame::tick);
    updateSpeed();

    m_nextType = nextFromBag();
    spawnPiece();

    m_timer->start();
    setFocusPolicy(Qt::StrongFocus);
}

// ── Access shape grid ────────────────────────────────────────────────────────
const int (&TetrisGame::getShape(TetroType t, int rot) const)[4][4]
{
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
                if (bc < 0 || bc >= BOARD_COLS || br >= TOTAL_ROWS)
                    return false;
                if (br < 0)
                    continue;
                if (m_board[br][bc] != 0)
                    return false;
            }
        }
    }
    return true;
}

// ── Wall kick: try rotation with horizontal offsets ──────────────────────────
bool TetrisGame::tryRotate()
{
    int newRot = (m_rotation + 1) % 4;
    // Try offsets: 0, -1, +1, -2, +2
    int offsets[] = { 0, -1, 1, -2, 2 };
    for (int off : offsets) {
        if (isValid(m_currentType, m_currentRow, m_currentCol + off, newRot)) {
            m_currentCol += off;
            m_rotation = newRot;
            return true;
        }
    }
    return false;
}

// ── Ghost piece: find the lowest valid row ───────────────────────────────────
int TetrisGame::ghostRow() const
{
    int row = m_currentRow;
    while (isValid(m_currentType, row + 1, m_currentCol, m_rotation))
        ++row;
    return row;
}

// ── Update speed based on level ──────────────────────────────────────────────
void TetrisGame::updateSpeed()
{
    // Level 1=500ms, level 2=430ms, ... faster as level increases
    int interval = std::max(100, 500 - (m_level - 1) * 50);
    m_timer->setInterval(interval);
}

// ── Spawn new piece ──────────────────────────────────────────────────────────
void TetrisGame::spawnPiece()
{
    m_currentType = m_nextType;
    m_nextType = nextFromBag();
    m_rotation = 0;
    m_currentRow = HIDDEN_ROWS - 1;
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

    // Find which rows are now full (for flash animation)
    m_clearingRows.clear();
    for (int r = TOTAL_ROWS - 1; r >= 0; --r) {
        bool full = true;
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_board[r][c] == 0) { full = false; break; }
        }
        if (full)
            m_clearingRows.append(r);
    }

    if (!m_clearingRows.isEmpty()) {
        // Start flash animation, delay actual clearing
        m_flashFrames = FLASH_TOTAL;
        m_timer->stop();
    } else {
        spawnPiece();
    }
    update();
}

// ── Clear completed lines ────────────────────────────────────────────────────
int TetrisGame::clearLines()
{
    int cleared = 0;
    for (int r = TOTAL_ROWS - 1; r >= 0; ) {
        bool full = true;
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_board[r][c] == 0) { full = false; break; }
        }
        if (full) {
            for (int rr = r; rr > 0; --rr)
                for (int c = 0; c < BOARD_COLS; ++c)
                    m_board[rr][c] = m_board[rr-1][c];
            for (int c = 0; c < BOARD_COLS; ++c)
                m_board[0][c] = 0;
            ++cleared;
        } else {
            --r;
        }
    }
    return cleared;
}

// ── Game tick ────────────────────────────────────────────────────────────────
void TetrisGame::tick()
{
    if (m_gameOver || m_paused) return;

    // Handle line clear flash animation
    if (m_flashFrames > 0) {
        --m_flashFrames;
        if (m_flashFrames == 0) {
            // Actually clear the lines
            int cleared = 0;
            for (int row : m_clearingRows) {
                for (int rr = row; rr > 0; --rr)
                    for (int c = 0; c < BOARD_COLS; ++c)
                        m_board[rr][c] = m_board[rr-1][c];
                for (int c = 0; c < BOARD_COLS; ++c)
                    m_board[0][c] = 0;
                ++cleared;
            }
            m_clearingRows.clear();

            // Score
            if (cleared == 1) m_score += 100 * m_level;
            else if (cleared == 2) m_score += 300 * m_level;
            else if (cleared == 3) m_score += 500 * m_level;
            else if (cleared >= 4) m_score += 800 * m_level;

            // Level up every 10 lines
            m_linesCleared += cleared;
            int newLevel = m_linesCleared / 10 + 1;
            if (newLevel != m_level) {
                m_level = newLevel;
                updateSpeed();
            }

            spawnPiece();
            m_timer->start();
        }
        update();
        return;
    }

    // Normal tick: move piece down
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
            m_level = 1;
            m_linesCleared = 0;
            m_gameOver = false;
            m_paused = false;
            m_clearingRows.clear();
            m_flashFrames = 0;
            updateSpeed();
            m_nextType = nextFromBag();
            spawnPiece();
            m_timer->start();
            update();
        }
        return;
    }

    // Pause toggle
    if (event->key() == Qt::Key_P) {
        m_paused = !m_paused;
        if (m_paused)
            m_timer->stop();
        else
            m_timer->start();
        update();
        return;
    }

    if (m_paused) return;

    // Don't process movement during flash animation
    if (m_flashFrames > 0) return;

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
        if (isValid(m_currentType, m_currentRow + 1, m_currentCol, m_rotation)) {
            ++m_currentRow;
            m_score += 1;  // soft drop bonus
        }
        break;
    case Qt::Key_Up:
        tryRotate();
        break;
    case Qt::Key_Space:
        // Hard drop
        {
            int dropDist = 0;
            while (isValid(m_currentType, m_currentRow + 1, m_currentCol, m_rotation)) {
                ++m_currentRow;
                ++dropDist;
            }
            m_score += dropDist * 2;  // hard drop bonus
            lockPiece();
        }
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

    // Background
    p.fillRect(rect(), QColor(30, 30, 30));

    // Board background
    p.setPen(QPen(QColor(80, 80, 80), 2));
    p.setBrush(QColor(20, 20, 20));
    p.drawRect(boardX - 2, boardY - 2, boardW + 4, boardH + 4);
    p.fillRect(boardX, boardY, boardW, boardH, QColor(20, 20, 20));

    // Grid lines
    p.setPen(QColor(40, 40, 40));
    for (int c = 1; c < BOARD_COLS; ++c) {
        int x = boardX + c * cellSize;
        p.drawLine(x, boardY, x, boardY + boardH);
    }
    for (int r = 1; r < BOARD_ROWS; ++r) {
        int y = boardY + r * cellSize;
        p.drawLine(boardX, y, boardX + boardW, y);
    }

    // Locked blocks (with flash effect for clearing rows)
    for (int r = HIDDEN_ROWS; r < TOTAL_ROWS; ++r) {
        bool isClearing = m_clearingRows.contains(r) && m_flashFrames > 0;
        for (int c = 0; c < BOARD_COLS; ++c) {
            if (m_board[r][c] != 0) {
                if (isClearing) {
                    // Flash white
                    int alpha = 120 + 135 * (m_flashFrames % 2);
                    drawBlock(p, r - HIDDEN_ROWS, c, QColor(255, 255, 255), alpha);
                } else {
                    int typeIdx = m_board[r][c] - 1;
                    drawBlock(p, r - HIDDEN_ROWS, c, TETRO_COLORS[typeIdx]);
                }
            }
        }
    }

    // Ghost piece
    if (!m_gameOver && !m_paused && m_flashFrames == 0 && m_currentType != TetroType::None) {
        drawGhost(p);
    }

    // Current falling piece
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

    // Next piece preview
    drawPreview(p);

    // Score & Level
    drawScore(p);

    // Game Over overlay
    if (m_gameOver)
        drawGameOver(p);

    // Paused overlay
    if (m_paused)
        drawPaused(p);
}

// ── Draw single cell ─────────────────────────────────────────────────────────
void TetrisGame::drawBlock(QPainter &p, int row, int col, const QColor &color, int alpha)
{
    int cellSize = 30;
    int x = 30 + col * cellSize;
    int y = 30 + row * cellSize;
    int m = 1;

    QColor fill = color;
    fill.setAlpha(alpha);
    p.setPen(Qt::NoPen);
    p.setBrush(fill);
    p.drawRect(x + m, y + m, cellSize - 2*m, cellSize - 2*m);

    QColor light = color.lighter(150);
    light.setAlpha(alpha);
    p.setPen(QPen(light, 1));
    p.drawLine(x + m, y + m, x + cellSize - m, y + m);
    p.drawLine(x + m, y + m, x + m, y + cellSize - m);

    QColor dark = color.darker(150);
    dark.setAlpha(alpha);
    p.setPen(QPen(dark, 1));
    p.drawLine(x + cellSize - m, y + m, x + cellSize - m, y + cellSize - m);
    p.drawLine(x + m, y + cellSize - m, x + cellSize - m, y + cellSize - m);
}

// ── Ghost piece ──────────────────────────────────────────────────────────────
void TetrisGame::drawGhost(QPainter &p)
{
    int gr = ghostRow();
    if (gr == m_currentRow) return;

    const auto &shape = getShape(m_currentType, m_rotation);
    QColor col = TETRO_COLORS[static_cast<int>(m_currentType)];

    int cellSize = 30;
    for (int r = 0; r < PIECE_SIZE; ++r) {
        for (int c = 0; c < PIECE_SIZE; ++c) {
            if (shape[r][c]) {
                int br = gr + r - HIDDEN_ROWS;
                int bc = m_currentCol + c;
                if (br >= 0) {
                    int x = 30 + bc * cellSize;
                    int y = 30 + br * cellSize;
                    p.setPen(QPen(col, 2));
                    p.setBrush(Qt::NoBrush);
                    p.drawRect(x + 2, y + 2, cellSize - 4, cellSize - 4);
                }
            }
        }
    }
}

// ── Next piece preview ───────────────────────────────────────────────────────
void TetrisGame::drawPreview(QPainter &p)
{
    int previewX = 340;
    int previewY = 40;
    int cellSize = 25;
    int previewSize = PIECE_SIZE * cellSize;

    p.setPen(QPen(QColor(80, 80, 80), 2));
    p.setBrush(QColor(40, 40, 40));
    p.drawRect(previewX, previewY, previewSize + 20, previewSize + 20);

    if (m_nextType == TetroType::None) return;

    p.setPen(QColor(200, 200, 200));
    QFont titleFont("Arial", 12, QFont::Bold);
    p.setFont(titleFont);
    p.drawText(previewX, previewY - 10, 100, 20, Qt::AlignLeft, "NEXT");

    const auto &shape = getShape(m_nextType, 0);
    QColor col = TETRO_COLORS[static_cast<int>(m_nextType)];

    int startX = previewX + 10;
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

// ── Score & Level ────────────────────────────────────────────────────────────
void TetrisGame::drawScore(QPainter &p)
{
    int x = 340;
    int y = 200;

    p.setPen(QColor(200, 200, 200));
    QFont labelFont("Arial", 12, QFont::Bold);
    p.setFont(labelFont);
    p.drawText(x, y, 150, 20, Qt::AlignLeft, "SCORE");

    QFont scoreFont("Arial", 28, QFont::Bold);
    p.setFont(scoreFont);
    p.setPen(QColor(255, 255, 255));
    p.drawText(x, y + 25, 150, 35, Qt::AlignLeft, QString::number(m_score));

    // Level
    p.setPen(QColor(200, 200, 200));
    p.setFont(labelFont);
    p.drawText(x, y + 70, 150, 20, Qt::AlignLeft, "LEVEL");

    p.setFont(scoreFont);
    p.setPen(QColor(255, 200, 50));
    p.drawText(x, y + 90, 150, 35, Qt::AlignLeft, QString::number(m_level));

    // Lines
    p.setPen(QColor(200, 200, 200));
    p.setFont(labelFont);
    p.drawText(x, y + 135, 150, 20, Qt::AlignLeft, "LINES");

    QFont midFont("Arial", 20, QFont::Bold);
    p.setFont(midFont);
    p.setPen(QColor(180, 255, 180));
    p.drawText(x, y + 155, 150, 30, Qt::AlignLeft, QString::number(m_linesCleared));

    // Controls hint
    p.setPen(QColor(140, 140, 140));
    QFont hintFont("Arial", 10);
    p.setFont(hintFont);
    int hy = 440;
    p.drawText(x, hy, 150, 20, Qt::AlignLeft, "CONTROLS");
    p.setPen(QColor(100, 100, 100));
    p.drawText(x, hy + 20, 150, 150, Qt::AlignLeft,
               "Left/Right: Move\n"
               "Down: Soft drop\n"
               "Up: Rotate\n"
               "Space: Hard drop\n"
               "P: Pause\n"
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

// ── Paused overlay ───────────────────────────────────────────────────────────
void TetrisGame::drawPaused(QPainter &p)
{
    p.fillRect(rect(), QColor(0, 0, 0, 120));

    p.setPen(QColor(255, 255, 100));
    QFont pauseFont("Arial", 36, QFont::Bold);
    p.setFont(pauseFont);
    p.drawText(rect(), Qt::AlignCenter, "PAUSED");

    p.setPen(QColor(200, 200, 200));
    QFont hintFont("Arial", 14);
    p.setFont(hintFont);
    QRect r = rect();
    p.drawText(r.adjusted(0, 80, 0, 0), Qt::AlignCenter, "Press P to resume");
}
