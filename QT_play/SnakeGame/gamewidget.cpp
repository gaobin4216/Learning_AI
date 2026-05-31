#include "gamewidget.h"
#include <QPainter>
#include <QFont>

GameWidget::GameWidget(QWidget *parent)
    : QWidget(parent), highScore(0)
{
    setFixedSize(CELL_SIZE * BOARD_WIDTH, CELL_SIZE * BOARD_HEIGHT);
    setFocusPolicy(Qt::StrongFocus);
    setStyleSheet("background-color: #1a1a2e;");

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &GameWidget::gameUpdate);

    initGame();
}

void GameWidget::initGame()
{
    snake.clear();
    snake.append(QPoint(BOARD_WIDTH / 2, BOARD_HEIGHT / 2));
    snake.append(QPoint(BOARD_WIDTH / 2 - 1, BOARD_HEIGHT / 2));
    snake.append(QPoint(BOARD_WIDTH / 2 - 2, BOARD_HEIGHT / 2));

    direction = Right;
    nextDirection = Right;
    score = 0;
    state = Ready;
    timer->stop();
    spawnFood();
    update();
}

void GameWidget::startGame()
{
    if (state == Ready || state == GameOver) {
        initGame();
        state = Playing;
        timer->start(120);
    } else if (state == Paused) {
        state = Playing;
        timer->start(120);
    }
}

void GameWidget::pauseGame()
{
    if (state == Playing) {
        state = Paused;
        timer->stop();
        update();
    }
}

void GameWidget::spawnFood()
{
    QVector<QPoint> freeCells;
    for (int x = 0; x < BOARD_WIDTH; ++x) {
        for (int y = 0; y < BOARD_HEIGHT; ++y) {
            QPoint p(x, y);
            if (!snake.contains(p))
                freeCells.append(p);
        }
    }
    if (freeCells.isEmpty()) {
        state = GameOver;
        timer->stop();
        return;
    }
    food = freeCells[QRandomGenerator::global()->bounded(freeCells.size())];
}

void GameWidget::moveSnake()
{
    direction = nextDirection;
    QPoint head = snake.first();

    switch (direction) {
    case Up:    head.ry()--; break;
    case Down:  head.ry()++; break;
    case Left:  head.rx()--; break;
    case Right: head.rx()++; break;
    }

    snake.prepend(head);

    if (head == food) {
        score += 10;
        if (score > highScore) highScore = score;
        spawnFood();
        // Speed up slightly
        int interval = qMax(50, 120 - score / 2);
        timer->setInterval(interval);
    } else {
        snake.removeLast();
    }
}

bool GameWidget::checkCollision()
{
    const QPoint &head = snake.first();

    // Wall collision
    if (head.x() < 0 || head.x() >= BOARD_WIDTH ||
        head.y() < 0 || head.y() >= BOARD_HEIGHT)
        return true;

    // Self collision
    for (int i = 1; i < snake.size(); ++i) {
        if (snake[i] == head)
            return true;
    }
    return false;
}

void GameWidget::gameUpdate()
{
    if (state != Playing) return;

    moveSnake();
    if (checkCollision()) {
        state = GameOver;
        timer->stop();
    }
    update();
}

void GameWidget::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_W:
        if (direction != Down) nextDirection = Up;
        break;
    case Qt::Key_Down:
    case Qt::Key_S:
        if (direction != Up) nextDirection = Down;
        break;
    case Qt::Key_Left:
    case Qt::Key_A:
        if (direction != Right) nextDirection = Left;
        break;
    case Qt::Key_Right:
    case Qt::Key_D:
        if (direction != Left) nextDirection = Right;
        break;
    case Qt::Key_Space:
        if (state == Ready || state == GameOver)
            startGame();
        else if (state == Playing)
            pauseGame();
        else if (state == Paused)
            startGame();
        break;
    default:
        QWidget::keyPressEvent(event);
    }
}

void GameWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    drawGrid(painter);
    drawFood(painter);
    drawSnake(painter);
    drawScore(painter);

    if (state == Ready)
        drawCenterMessage(painter, "Press SPACE to Start");
    else if (state == Paused)
        drawCenterMessage(painter, "PAUSED", -20);
    else if (state == GameOver)
        drawCenterMessage(painter, "GAME OVER", -30);
}

void GameWidget::drawGrid(QPainter &painter)
{
    painter.setPen(QPen(QColor(0x16, 0x21, 0x3e), 0.5));
    for (int x = 0; x <= BOARD_WIDTH; ++x)
        painter.drawLine(x * CELL_SIZE, 0, x * CELL_SIZE, height());
    for (int y = 0; y <= BOARD_HEIGHT; ++y)
        painter.drawLine(0, y * CELL_SIZE, width(), y * CELL_SIZE);
}

void GameWidget::drawSnake(QPainter &painter)
{
    for (int i = 0; i < snake.size(); ++i) {
        QRect rect(snake[i].x() * CELL_SIZE + 1,
                   snake[i].y() * CELL_SIZE + 1,
                   CELL_SIZE - 2, CELL_SIZE - 2);

        if (i == 0) {
            painter.setBrush(QColor(0x00, 0xd2, 0xff));
            painter.setPen(QPen(QColor(0x00, 0xb4, 0xd8), 1));
        } else {
            painter.setBrush(QColor(0x00, 0x96, 0xc7));
            painter.setPen(QPen(QColor(0x00, 0x77, 0xb6), 1));
        }
        painter.drawRoundedRect(rect, 4, 4);
    }
}

void GameWidget::drawFood(QPainter &painter)
{
    QRect rect(food.x() * CELL_SIZE + 2,
               food.y() * CELL_SIZE + 2,
               CELL_SIZE - 4, CELL_SIZE - 4);

    painter.setBrush(QColor(0xff, 0x6b, 0x6b));
    painter.setPen(QPen(QColor(0xee, 0x5a, 0x5a), 1));
    painter.drawEllipse(rect);
}

void GameWidget::drawScore(QPainter &painter)
{
    painter.setPen(Qt::white);
    painter.setFont(QFont("Consolas", 11, QFont::Bold));
    painter.drawText(10, 20, QString("Score: %1").arg(score));
    painter.drawText(width() - 130, 20, QString("Best: %1").arg(highScore));
}

void GameWidget::drawCenterMessage(QPainter &painter, const QString &msg, int yOffset)
{
    painter.setPen(QColor(0xe0, 0xe0, 0xe0));
    painter.setFont(QFont("Consolas", 18, QFont::Bold));
    painter.drawText(rect(), Qt::AlignCenter, msg);

    if (state == GameOver || state == Ready) {
        painter.setFont(QFont("Consolas", 11));
        painter.setPen(QColor(0xa0, 0xa0, 0xa0));
        QRect sub = rect().adjusted(0, 40 + yOffset, 0, 0);
        painter.drawText(sub, Qt::AlignHCenter | Qt::AlignTop,
                         state == GameOver
                             ? QString("Score: %1  |  Press SPACE to restart").arg(score)
                             : "Use Arrow Keys or WASD to move");
    }
}
