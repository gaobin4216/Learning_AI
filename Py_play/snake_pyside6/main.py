import sys
import random
from PySide6.QtCore import Qt, QTimer, QRect
from PySide6.QtGui import QPainter, QColor, QFont
from PySide6.QtWidgets import QApplication, QWidget, QMessageBox


GRID_SIZE = 20
CELL_SIZE = 30
WIDTH = GRID_SIZE * CELL_SIZE
HEIGHT = GRID_SIZE * CELL_SIZE

UP = (0, -1)
DOWN = (0, 1)
LEFT = (-1, 0)
RIGHT = (1, 0)


class SnakeGame(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("贪吃蛇 - PySide6")
        self.setFixedSize(WIDTH, HEIGHT)
        self.setStyleSheet("background-color: #1a1a2e;")

        self.snake = [(GRID_SIZE // 2, GRID_SIZE // 2)]
        self.direction = RIGHT
        self.next_direction = RIGHT
        self.food = self._random_food()
        self.score = 0
        self.running = False

        self.timer = QTimer(self)
        self.timer.timeout.connect(self._tick)

        self.setFocusPolicy(Qt.StrongFocus)

    def _random_food(self):
        occupied = set(self.snake)
        free = [(x, y) for x in range(GRID_SIZE) for y in range(GRID_SIZE) if (x, y) not in occupied]
        return random.choice(free) if free else None

    def start_game(self):
        self.snake = [(GRID_SIZE // 2, GRID_SIZE // 2)]
        self.direction = RIGHT
        self.next_direction = RIGHT
        self.score = 0
        self.food = self._random_food()
        self.running = True
        self.timer.start(150)
        self.update()

    def _tick(self):
        if not self.running:
            return

        self.direction = self.next_direction
        head = self.snake[0]
        dx, dy = self.direction
        new_head = (head[0] + dx, head[1] + dy)

        if not (0 <= new_head[0] < GRID_SIZE and 0 <= new_head[1] < GRID_SIZE):
            self._game_over()
            return

        if new_head in self.snake:
            self._game_over()
            return

        self.snake.insert(0, new_head)

        if new_head == self.food:
            self.score += 1
            self.food = self._random_food()
            if self.food is None:
                self._game_over()
                return
        else:
            self.snake.pop()

        self.update()

    def _game_over(self):
        self.running = False
        self.timer.stop()
        reply = QMessageBox.question(
            self,
            "游戏结束",
            f"得分：{self.score}\n再来一局吗？",
            QMessageBox.Yes | QMessageBox.No,
            QMessageBox.Yes,
        )
        if reply == QMessageBox.Yes:
            self.start_game()
        else:
            self.close()

    def keyPressEvent(self, event):
        key = event.key()
        if key == Qt.Key_Up and self.direction != DOWN:
            self.next_direction = UP
        elif key == Qt.Key_Down and self.direction != UP:
            self.next_direction = DOWN
        elif key == Qt.Key_Left and self.direction != RIGHT:
            self.next_direction = LEFT
        elif key == Qt.Key_Right and self.direction != LEFT:
            self.next_direction = RIGHT
        elif key == Qt.Key_Space:
            if not self.running:
                self.start_game()
        super().keyPressEvent(event)

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        painter.setPen(QColor(30, 30, 70))
        for x in range(0, WIDTH, CELL_SIZE):
            painter.drawLine(x, 0, x, HEIGHT)
        for y in range(0, HEIGHT, CELL_SIZE):
            painter.drawLine(0, y, WIDTH, y)

        for i, (x, y) in enumerate(self.snake):
            rect = QRect(x * CELL_SIZE + 1, y * CELL_SIZE + 1, CELL_SIZE - 2, CELL_SIZE - 2)
            if i == 0:
                painter.setBrush(QColor(0, 230, 118))
            else:
                t = i / len(self.snake)
                g = int(180 + 50 * (1 - t))
                r = int(50 * t)
                painter.setBrush(QColor(r, g, 80))
            painter.setPen(Qt.NoPen)
            painter.drawRoundedRect(rect, 4, 4)

        if self.food:
            fx, fy = self.food
            center = QRect(
                fx * CELL_SIZE + 4, fy * CELL_SIZE + 4,
                CELL_SIZE - 8, CELL_SIZE - 8,
            )
            painter.setBrush(QColor(255, 82, 82))
            painter.setPen(Qt.NoPen)
            painter.drawEllipse(center)

        painter.setPen(QColor(255, 255, 255))
        painter.setFont(QFont("Segoe UI", 14, QFont.Bold))
        painter.drawText(10, 30, f"得分: {self.score}")

        if not self.running and not self.snake:
            painter.setPen(QColor(200, 200, 200))
            painter.setFont(QFont("Segoe UI", 18, QFont.Bold))
            painter.drawText(
                self.rect(), Qt.AlignCenter, "按 空格键 开始游戏"
            )


if __name__ == "__main__":
    app = QApplication(sys.argv)
    game = SnakeGame()
    game.show()
    sys.exit(app.exec())
