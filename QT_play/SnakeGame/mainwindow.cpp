#include "mainwindow.h"
#include "gamewidget.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    gameWidget = new GameWidget(this);
    setCentralWidget(gameWidget);

    setWindowTitle("Snake Game - Qt");
    setFixedSize(600, 500 + menuBar()->height());

    QMenu *gameMenu = menuBar()->addMenu("&Game");

    QAction *newAction = gameMenu->addAction("&New Game");
    newAction->setShortcut(QKeySequence("Ctrl+N"));
    connect(newAction, &QAction::triggered, gameWidget, &GameWidget::startGame);

    QAction *pauseAction = gameMenu->addAction("&Pause");
    pauseAction->setShortcut(QKeySequence("P"));
    connect(pauseAction, &QAction::triggered, gameWidget, &GameWidget::pauseGame);

    gameMenu->addSeparator();

    QAction *quitAction = gameMenu->addAction("&Quit");
    quitAction->setShortcut(QKeySequence("Ctrl+Q"));
    connect(quitAction, &QAction::triggered, this, &QWidget::close);

    QMenu *helpMenu = menuBar()->addMenu("&Help");
    QAction *aboutAction = helpMenu->addAction("&About");
    connect(aboutAction, &QAction::triggered, this, [this]() {
        QMessageBox::about(this, "About Snake Game",
            "A classic Snake game built with Qt.\n\n"
            "Controls:\n"
            "  Arrow Keys / WASD - Move\n"
            "  SPACE - Start / Pause / Restart\n"
            "  P - Pause\n"
            "  Ctrl+N - New Game");
    });
}
