from pathlib import Path
from PySide6.QtUiTools import QUiLoader
from PySide6.QtWidgets import QApplication
from PySide6.QtCore import QFile


class Calculator:
    def __init__(self):
        loader = QUiLoader()
        ui_path = str(Path(__file__).parent / "calculator.ui")
        ui_file = QFile(ui_path)
        ui_file.open(QFile.ReadOnly)
        self.ui = loader.load(ui_file, None)
        ui_file.close()

        self._current = ""
        self._prev = ""
        self._op = ""

        self._wire_buttons()

    def _wire_buttons(self):
        digits = {"btn0": "0", "btn1": "1", "btn2": "2", "btn3": "3", "btn4": "4",
                  "btn5": "5", "btn6": "6", "btn7": "7", "btn8": "8", "btn9": "9"}
        for name, text in digits.items():
            getattr(self.ui, name).clicked.connect(lambda _, t=text: self._on_digit(t))

        ops = {"btnAdd": "+", "btnSub": "-", "btnMul": "*", "btnDiv": "/"}
        for name, text in ops.items():
            getattr(self.ui, name).clicked.connect(lambda _, t=text: self._on_op(t))

        self.ui.btnDot.clicked.connect(self._on_dot)
        self.ui.btnEq.clicked.connect(self._on_eq)
        self.ui.btnC.clicked.connect(self._on_clear)
        self.ui.btnBack.clicked.connect(self._on_back)
        self.ui.btnNeg.clicked.connect(self._on_neg)

    def _show(self, text):
        self.ui.display.setText(text)

    def _on_digit(self, d):
        self._current += d
        self._show(self._current)

    def _on_dot(self):
        if "." not in self._current:
            self._current += "." if self._current else "0."
            self._show(self._current)

    def _on_op(self, op):
        if self._current or op == "-":
            self._compute()
            self._prev = self._current if self._current else "0"
            self._current = ""
            self._op = op

    def _on_eq(self):
        self._compute()
        self._op = ""

    def _on_clear(self):
        self._current = ""
        self._prev = ""
        self._op = ""
        self._show("0")

    def _on_back(self):
        self._current = self._current[:-1]
        self._show(self._current or "0")

    def _on_neg(self):
        if self._current:
            self._current = str(-float(self._current))
            if self._current.endswith(".0"):
                self._current = self._current[:-2]
            self._show(self._current)

    def _compute(self):
        if not self._prev or not self._op or not self._current:
            return
        try:
            a, b = float(self._prev), float(self._current)
            if self._op == "+":
                result = a + b
            elif self._op == "-":
                result = a - b
            elif self._op == "*":
                result = a * b
            elif self._op == "/":
                result = "错误" if b == 0 else a / b
            self._current = str(result) if not isinstance(result, str) else result
            self._prev = ""
            self._show(self._current)
        except Exception:
            self._show("错误")
            self._current = ""


if __name__ == "__main__":
    app = QApplication()
    calc = Calculator()
    calc.ui.show()
    app.exec()
