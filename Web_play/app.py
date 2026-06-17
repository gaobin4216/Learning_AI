"""
仪表盘后端 - Flask REST API
提供待办清单的增删改查，前端静态文件托管
"""

import json
import os
from datetime import datetime
from flask import Flask, jsonify, request, send_from_directory
from flask_cors import CORS

app = Flask(__name__, static_folder=".", static_url_path="")
CORS(app)

DATA_FILE = os.path.join(os.path.dirname(__file__), "todos.json")


# ── 数据读写 ──────────────────────────────────────────────

def load_todos():
    if not os.path.exists(DATA_FILE):
        return []
    with open(DATA_FILE, "r", encoding="utf-8") as f:
        return json.load(f)


def save_todos(todos):
    with open(DATA_FILE, "w", encoding="utf-8") as f:
        json.dump(todos, f, ensure_ascii=False, indent=2)


# ── 静态页面 ──────────────────────────────────────────────

@app.route("/")
def index():
    return send_from_directory(".", "index.html")


# ── 待办 API ──────────────────────────────────────────────

@app.route("/api/todos", methods=["GET"])
def get_todos():
    """获取全部待办"""
    return jsonify(load_todos())


@app.route("/api/todos", methods=["POST"])
def add_todo():
    """新增待办"""
    data = request.get_json()
    text = data.get("text", "").strip()
    if not text:
        return jsonify({"error": "text 不能为空"}), 400

    todos = load_todos()
    new_id = max((t["id"] for t in todos), default=0) + 1
    todo = {
        "id": new_id,
        "text": text,
        "done": False,
        "created_at": datetime.now().isoformat(),
    }
    todos.append(todo)
    save_todos(todos)
    return jsonify(todo), 201


@app.route("/api/todos/<int:todo_id>", methods=["PUT"])
def update_todo(todo_id):
    """切换待办完成状态"""
    todos = load_todos()
    for todo in todos:
        if todo["id"] == todo_id:
            todo["done"] = not todo["done"]
            save_todos(todos)
            return jsonify(todo)
    return jsonify({"error": "未找到该待办"}), 404


@app.route("/api/todos/<int:todo_id>", methods=["DELETE"])
def delete_todo(todo_id):
    """删除待办"""
    todos = load_todos()
    new_todos = [t for t in todos if t["id"] != todo_id]
    if len(new_todos) == len(todos):
        return jsonify({"error": "未找到该待办"}), 404
    save_todos(new_todos)
    return jsonify({"message": "已删除"})


# ── 名言 API ──────────────────────────────────────────────

QUOTES = [
    "千里之行，始于足下。—— 老子",
    "学而不思则罔，思而不学则殆。—— 孔子",
    "天行健，君子以自强不息。—— 《周易》",
    "不积跬步，无以至千里。—— 荀子",
    "业精于勤，荒于嬉；行成于思，毁于随。—— 韩愈",
    "知之者不如好之者，好之者不如乐之者。—— 孔子",
    "吾生也有涯，而知也无涯。—— 庄子",
    "三人行，必有我师焉。—— 孔子",
    "志不强者智不达。—— 墨子",
    "书山有路勤为径，学海无涯苦作舟。—— 韩愈",
]


@app.route("/api/quote")
def get_quote():
    """根据日期返回每日名言"""
    today = datetime.now()
    index = (today.year * 366 + today.month * 31 + today.day) % len(QUOTES)
    return jsonify({"text": QUOTES[index]})


# ── 启动 ──────────────────────────────────────────────────

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)
