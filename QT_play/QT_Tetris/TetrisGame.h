#ifndef TETRISGAME_H
#define TETRISGAME_H

#include <QWidget>
#include <QTimer>
#include <QPainter>
#include <QKeyEvent>
#include <QMap>
#include <QVector>

// Board dimensions (visible area)
const int BOARD_COLS = 10;
const int BOARD_ROWS = 20;
const int HIDDEN_ROWS = 2;  // extra rows above visible area
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
    bool m_gameOver = false;

    QTimer *m_timer;

    // Piece shape definitions: shapes[type][rotation][row][col]
    static const int SHAPE[7][4][4];
    static const int PIECE_SIZE = 4;

    // Get shape grid for a given piece type and rotation
    const int (&getShape(TetroType t, int rot) const)[PIECE_SIZE][PIECE_SIZE];

    // Spawn a new piece from top
    void spawnPiece();

    // Check if placing a piece at (row, col) with given rotation is valid
    bool isValid(TetroType t, int row, int col, int rot) const;

    // Lock current piece into the board
    void lockPiece();

    // Clear completed lines and return number cleared
    int clearLines();

    // Draw the grid, blocks, preview, and score
    void drawGrid(QPainter &p);
    void drawBlock(QPainter &p, int row, int col, const QColor &color);
    void drawPreview(QPainter &p);
    void drawScore(QPainter &p);
    void drawGameOver(QPainter &p);
};

#endif // TETRISGAME_H
