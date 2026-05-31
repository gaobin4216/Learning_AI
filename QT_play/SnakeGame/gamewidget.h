#ifndef GAMEWIDGET_H
#define GAMEWIDGET_H

#include <QWidget>
#include <QVector>
#include <QPoint>
#include <QTimer>
#include <QKeyEvent>
#include <QPainter>
#include <QRandomGenerator>

class GameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GameWidget(QWidget *parent = nullptr);
    void startGame();
    void pauseGame();

protected:
    void paintEvent(QPaintEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void gameUpdate();

private:
    static const int CELL_SIZE = 20;
    static const int BOARD_WIDTH = 30;
    static const int BOARD_HEIGHT = 25;

    enum Direction { Up, Down, Left, Right };
    enum GameState { Ready, Playing, Paused, GameOver };

    QVector<QPoint> snake;
    QPoint food;
    Direction direction;
    Direction nextDirection;
    GameState state;
    QTimer *timer;
    int score;
    int highScore;

    void initGame();
    void spawnFood();
    void moveSnake();
    bool checkCollision();
    void drawGrid(QPainter &painter);
    void drawSnake(QPainter &painter);
    void drawFood(QPainter &painter);
    void drawScore(QPainter &painter);
    void drawCenterMessage(QPainter &painter, const QString &msg, int yOffset = 0);
};

#endif // GAMEWIDGET_H
