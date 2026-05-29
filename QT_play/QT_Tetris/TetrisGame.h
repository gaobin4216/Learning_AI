#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QVector>
#include <random>

// Board dimensions (visible area)
const int BOARD_COLS = 10;
const int BOARD_ROWS = 20;
const int HIDDEN_ROWS = 2;
const int TOTAL_ROWS = BOARD_ROWS + HIDDEN_ROWS;

// 7 tetromino types
enum class TetroType { I, O, T, S, Z, J, L, None };

// Colors for each piece type
static const QColor TETRO_COLORS[] = {
    QColor(0, 255, 255),    // I - cyan
    QColor(255, 255, 0),    // O - yellow
    QColor(170, 0, 255),    // T - purple
    QColor(0, 255, 0),      // S - green
    QColor(255, 0, 0),      // Z - red
    QColor(0, 0, 255),      // J - blue
    QColor(255, 136, 0),    // L - orange
    QColor(0, 0, 0)         // None - unused
};

class TetrisGame : public QWidget
{
    Q_OBJECT

public:
    explicit TetrisGame(QWidget *parent = nullptr);
    ~TetrisGame() override = default;

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void tick();

private:
    // Game board state: 0 = empty, 1..7 = filled by piece type
    int m_board[TOTAL_ROWS][BOARD_COLS] = {};

    // Current piece
    TetroType m_currentType = TetroType::None;
    int m_currentRow = 0;
    int m_currentCol = 0;
    int m_rotation = 0;

    // Next piece and score
    TetroType m_nextType = TetroType::None;
    int m_score = 0;
    int m_level = 1;
    int m_linesCleared = 0;
    bool m_gameOver = false;
    bool m_paused = false;

    // Line clear animation
    QVector<int> m_clearingRows;
    int m_flashFrames = 0;
    static const int FLASH_TOTAL = 8;

    QTimer *m_timer;

    // Modern random engine
    std::mt19937 m_rng;

    // 7-bag randomizer
    QVector<TetroType> m_bag;
    TetroType nextFromBag();

    // Piece shape definitions: shapes[type][row][col] (base rotation only)
    static const int SHAPE[7][4][4];
    static const int PIECE_SIZE = 4;

    // Get shape grid for a given piece type and rotation
    const int (&getShape(TetroType t, int rot) const)[PIECE_SIZE][PIECE_SIZE];

    void spawnPiece();
    bool isValid(TetroType t, int row, int col, int rot) const;
    void lockPiece();
    int clearLines();

    // Ghost piece: compute the drop row for current piece
    int ghostRow() const;

    // Wall kick: try rotating with horizontal offsets
    bool tryRotate();

    // Update timer interval based on level
    void updateSpeed();

    // Drawing
    void drawGrid(QPainter &p);
    void drawBlock(QPainter &p, int row, int col, const QColor &color, int alpha = 255);
    void drawPreview(QPainter &p);
    void drawScore(QPainter &p);
    void drawGameOver(QPainter &p);
    void drawPaused(QPainter &p);
    void drawGhost(QPainter &p);
};

#endif // TETRISGAME_H
