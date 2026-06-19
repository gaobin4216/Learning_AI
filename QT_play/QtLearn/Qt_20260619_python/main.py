import sys
import os
from PySide6.QtWidgets import QApplication, QWidget
from PySide6.QtUiTools import QUiLoader
from PySide6.QtCore import QFile


class Calculator(QWidget):
    def __init__(self):
        super().__init__()
        
        loader = QUiLoader()
        ui_path = os.path.join(os.path.dirname(__file__), "MyWidget.ui")
        ui_file = QFile(ui_path)
        ui_file.open(QFile.ReadOnly)
        self.ui = loader.load(ui_file, self)
        ui_file.close()
        
        self.setWindowTitle("计算器")
        
        self.accumulator = 0
        self.current_value = 0
        self.pending_operator = ""
        self.waiting_for_operand = True
        self.last_was_equal = False
        self.last_operand = 0
        self.last_operator = ""
        self.process_string = ""
        
        self.setup_connections()
    
    def setup_connections(self):
        self.ui.btn0.clicked.connect(lambda: self.on_digit_clicked("0"))
        self.ui.btn1.clicked.connect(lambda: self.on_digit_clicked("1"))
        self.ui.btn2.clicked.connect(lambda: self.on_digit_clicked("2"))
        self.ui.btn3.clicked.connect(lambda: self.on_digit_clicked("3"))
        self.ui.btn4.clicked.connect(lambda: self.on_digit_clicked("4"))
        self.ui.btn5.clicked.connect(lambda: self.on_digit_clicked("5"))
        self.ui.btn6.clicked.connect(lambda: self.on_digit_clicked("6"))
        self.ui.btn7.clicked.connect(lambda: self.on_digit_clicked("7"))
        self.ui.btn8.clicked.connect(lambda: self.on_digit_clicked("8"))
        self.ui.btn9.clicked.connect(lambda: self.on_digit_clicked("9"))
        
        self.ui.btnAdd.clicked.connect(lambda: self.on_operator_clicked("+"))
        self.ui.btnSubtract.clicked.connect(lambda: self.on_operator_clicked("-"))
        self.ui.btnMultiply.clicked.connect(lambda: self.on_operator_clicked("×"))
        self.ui.btnDivide.clicked.connect(lambda: self.on_operator_clicked("÷"))
        
        self.ui.btnEqual.clicked.connect(self.on_equal_clicked)
        self.ui.btnAC.clicked.connect(self.on_clear_clicked)
        self.ui.btnSign.clicked.connect(self.on_sign_clicked)
        self.ui.btnPercent.clicked.connect(self.on_percent_clicked)
        self.ui.btnDot.clicked.connect(self.on_dot_clicked)
    
    def format_number(self, number):
        if number == int(number) and abs(number) < 1e15:
            return str(int(number))
        return f"{number:.15g}"
    
    def update_process_display(self):
        self.ui.processLabel.setText(self.process_string)
    
    def on_digit_clicked(self, digit):
        if self.waiting_for_operand:
            self.ui.display.setText(digit)
            self.waiting_for_operand = False
        else:
            current_text = self.ui.display.text()
            if current_text == "0" and digit == "0":
                return
            if current_text == "0":
                self.ui.display.setText(digit)
            else:
                self.ui.display.setText(current_text + digit)
        
        self.last_was_equal = False
    
    def on_operator_clicked(self, new_operator):
        operand = float(self.ui.display.text())
        
        if self.pending_operator and not self.waiting_for_operand:
            result = 0
            if self.pending_operator == "+":
                result = self.accumulator + operand
            elif self.pending_operator == "-":
                result = self.accumulator - operand
            elif self.pending_operator == "×":
                result = self.accumulator * operand
            elif self.pending_operator == "÷":
                if operand == 0:
                    self.ui.display.setText("Error")
                    self.process_string = ""
                    self.update_process_display()
                    self.pending_operator = ""
                    self.waiting_for_operand = True
                    return
                result = self.accumulator / operand
            
            self.ui.display.setText(self.format_number(result))
            self.accumulator = result
            self.process_string = self.format_number(result) + " " + new_operator
        else:
            self.accumulator = operand
            self.process_string = self.format_number(operand) + " " + new_operator
        
        self.pending_operator = new_operator
        self.waiting_for_operand = True
        self.last_was_equal = False
        self.update_process_display()
    
    def on_equal_clicked(self):
        operand = float(self.ui.display.text())
        result = 0
        
        if self.last_was_equal and self.pending_operator:
            operand = self.last_operand
        
        if self.pending_operator:
            if self.pending_operator == "+":
                result = self.accumulator + operand
            elif self.pending_operator == "-":
                result = self.accumulator - operand
            elif self.pending_operator == "×":
                result = self.accumulator * operand
            elif self.pending_operator == "÷":
                if operand == 0:
                    self.ui.display.setText("Error")
                    self.process_string = ""
                    self.update_process_display()
                    self.pending_operator = ""
                    self.waiting_for_operand = True
                    self.last_was_equal = False
                    return
                result = self.accumulator / operand
            
            if self.last_was_equal:
                self.process_string = (self.format_number(self.accumulator) + " " + 
                                      self.last_operator + " " + 
                                      self.format_number(self.last_operand) + " =")
            else:
                self.process_string = (self.format_number(self.accumulator) + " " + 
                                      self.pending_operator + " " + 
                                      self.format_number(operand) + " =")
            
            self.last_operand = operand
            self.last_operator = self.pending_operator
            self.ui.display.setText(self.format_number(result))
            self.accumulator = result
            self.pending_operator = ""
            self.update_process_display()
        
        self.waiting_for_operand = True
        self.last_was_equal = True
    
    def on_clear_clicked(self):
        self.accumulator = 0
        self.current_value = 0
        self.pending_operator = ""
        self.waiting_for_operand = True
        self.last_was_equal = False
        self.last_operand = 0
        self.last_operator = ""
        self.process_string = ""
        self.ui.display.setText("0")
        self.update_process_display()
    
    def on_sign_clicked(self):
        text = self.ui.display.text()
        value = float(text)
        
        if value != 0:
            value = -value
            self.ui.display.setText(self.format_number(value))
    
    def on_percent_clicked(self):
        text = self.ui.display.text()
        value = float(text)
        
        value = value / 100.0
        self.ui.display.setText(self.format_number(value))
    
    def on_dot_clicked(self):
        text = self.ui.display.text()
        
        if self.waiting_for_operand:
            self.ui.display.setText("0.")
            self.waiting_for_operand = False
        elif "." not in text:
            self.ui.display.setText(text + ".")


if __name__ == "__main__":
    app = QApplication(sys.argv)
    calculator = Calculator()
    calculator.show()
    sys.exit(app.exec())
