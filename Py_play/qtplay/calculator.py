import sys
from PyQt6.QtWidgets import QApplication, QWidget, QGridLayout, QVBoxLayout, QPushButton, QLineEdit
from PyQt6.QtCore import Qt
from PyQt6.QtGui import QFont

class Calculator(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("计算器")
        self.resize(320, 420)
        self._current = ""
        self._prev = ""
        self._op = ""
        self._reset_on_next = False
        self._init_ui()

    def _init_ui(self):
        layout = QVBoxLayout()
        layout.setSpacing(8)

        # Display
        self.display = QLineEdit("0")
        self.display.setReadOnly(True)
        self.display.setAlignment(Qt.AlignmentFlag.AlignRight)
        self.display.setFixedHeight(64)
        font = QFont("Segoe UI", 24)
        font.setBold(True)
        self.display.setFont(font)
        self.display.setStyleSheet("""
            QLineEdit {
                background: #1e1e2e;
                color: #cdd6f4;
                border: 2px solid #313244;
                border-radius: 8px;
                padding: 0 12px;
            }
        """)
        layout.addWidget(self.display)

        # Button grid
        grid = QGridLayout()
        grid.setSpacing(6)

        buttons = [
            ("C",  0, 0, "#f38ba8"), ("(",  0, 1, "#89b4fa"), (")",  0, 2, "#89b4fa"), ("/",  0, 3, "#f9e2af"),
            ("7",  1, 0, "#585b70"), ("8",  1, 1, "#585b70"), ("9",  1, 2, "#585b70"), ("*",  1, 3, "#f9e2af"),
            ("4",  2, 0, "#585b70"), ("5",  2, 1, "#585b70"), ("6",  2, 2, "#585b70"), ("-",  2, 3, "#f9e2af"),
            ("1",  3, 0, "#585b70"), ("2",  3, 1, "#585b70"), ("3",  3, 2, "#585b70"), ("+",  3, 3, "#f9e2af"),
            ("+/-",4, 0, "#585b70"), ("0",  4, 1, "#585b70"), (".",  4, 2, "#585b70"), ("=",  4, 3, "#a6e3a1"),
        ]

        self._btns = {}
        for text, row, col, color in buttons:
            btn = QPushButton(text)
            btn.setFixedHeight(52)
            f = QFont("Segoe UI", 16)
            f.setBold(True)
            btn.setFont(f)
            btn.setStyleSheet(f"""
                QPushButton {{
                    background: {color};
                    color: #1e1e2e;
                    border: none;
                    border-radius: 8px;
                }}
                QPushButton:hover {{
                    filter: brightness(1.2);
                }}
                QPushButton:pressed {{
                    background: {color}cc;
                }}
            """)
            btn.clicked.connect(lambda _, t=text: self._on_btn(t))
            grid.addWidget(btn, row, col)
            self._btns[text] = btn

        layout.addLayout(grid)
        self.setLayout(layout)
        self.setStyleSheet("background: #11111b;")

    def _on_btn(self, key):
        if key.isdigit():
            if self._reset_on_next:
                self._current = ""
                self._reset_on_next = False
            self._current += key
            self._show(self._current)

        elif key == ".":
            if self._reset_on_next:
                self._current = "0"
                self._reset_on_next = False
            if "." not in self._current:
                self._current += "." if self._current else "0."
            self._show(self._current)

        elif key == "+/-":
            if self._current and self._current != "0":
                self._current = str(-float(self._current)).rstrip("0").rstrip(".")
            self._show(self._current)

        elif key in "+-*/":
            if self._current:
                if self._prev and self._op:
                    self._compute()
                else:
                    self._prev = self._current
            self._op = key
            self._reset_on_next = True

        elif key == "=":
            if self._prev and self._op and self._current:
                self._compute()
                self._op = ""
                self._prev = ""
            self._reset_on_next = True

        elif key == "C":
            self._current = ""
            self._prev = ""
            self._op = ""
            self._reset_on_next = False
            self._show("0")

    def _compute(self):
        try:
            a = float(self._prev)
            b = float(self._current)
            if self._op == "+":
                r = a + b
            elif self._op == "-":
                r = a - b
            elif self._op == "*":
                r = a * b
            elif self._op == "/":
                r = a / b if b != 0 else "Error"
            else:
                return
            if r == "Error":
                self._current = "Error"
                self._show("Error")
                return
            if r == int(r):
                r = int(r)
            self._current = str(r)
            self._prev = str(r)
            self._show(self._current)
        except Exception:
            self._show("Error")

    def _show(self, text):
        self.display.setText(text)

    def keyPressEvent(self, event):
        kmap = {
            Qt.Key.Key_0: "0", Qt.Key.Key_1: "1", Qt.Key.Key_2: "2",
            Qt.Key.Key_3: "3", Qt.Key.Key_4: "4", Qt.Key.Key_5: "5",
            Qt.Key.Key_6: "6", Qt.Key.Key_7: "7", Qt.Key.Key_8: "8",
            Qt.Key.Key_9: "9",
            Qt.Key.Key_Plus: "+", Qt.Key.Key_Minus: "-",
            Qt.Key.Key_Asterisk: "*", Qt.Key.Key_Slash: "/",
            Qt.Key.Key_Period: ".", Qt.Key.Key_Comma: ".",
            Qt.Key.Key_Enter: "=", Qt.Key.Key_Return: "=",
            Qt.Key.Key_Backspace: "C", Qt.Key.Key_Escape: "C",
        }
        key = kmap.get(event.key())
        if key:
            self._on_btn(key)


if __name__ == "__main__":
    app = QApplication(sys.argv)
    w = Calculator()
    w.show()
    sys.exit(app.exec())
