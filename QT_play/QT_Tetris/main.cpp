#include <QApplication>
#include "TetrisGame.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    TetrisGame game;
    game.show();
    return app.exec();
}
