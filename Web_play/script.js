const API = "/api";

// ========== 时钟与问候 ==========
function updateClock() {
  const now = new Date();
  const h = String(now.getHours()).padStart(2, "0");
  const m = String(now.getMinutes()).padStart(2, "0");
  const s = String(now.getSeconds()).padStart(2, "0");
  document.getElementById("clock").textContent = `${h}:${m}:${s}`;

  const weekdays = ["日", "一", "二", "三", "四", "五", "六"];
  const dateStr = `${now.getFullYear()} 年 ${now.getMonth() + 1} 月 ${now.getDate()} 日  星期${weekdays[now.getDay()]}`;
  document.getElementById("date").textContent = dateStr;

  const hour = now.getHours();
  let greet = "夜深了，注意休息 🌙";
  if (hour >= 5 && hour < 9) greet = "早上好，新的一天开始了 ☀️";
  else if (hour >= 9 && hour < 12) greet = "上午好，保持专注 💪";
  else if (hour >= 12 && hour < 14) greet = "中午好，记得吃饭 🍱";
  else if (hour >= 14 && hour < 18) greet = "下午好，继续加油 🚀";
  else if (hour >= 18 && hour < 22) greet = "晚上好，放松一下 🎵";
  document.getElementById("greeting").textContent = greet;
}

updateClock();
setInterval(updateClock, 1000);

// ========== 待办清单（调用后端 API） ==========
const todoInput = document.getElementById("todoInput");
const addBtn = document.getElementById("addBtn");
const todoList = document.getElementById("todoList");

async function fetchTodos() {
  const res = await fetch(`${API}/todos`);
  return res.json();
}

async function renderTodos() {
  const todos = await fetchTodos();
  todoList.innerHTML = "";
  todos.forEach((todo) => {
    const li = document.createElement("li");
    if (todo.done) li.classList.add("done");

    const checkBtn = document.createElement("button");
    checkBtn.className = "check-btn";
    checkBtn.addEventListener("click", () => toggleTodo(todo.id));

    const span = document.createElement("span");
    span.textContent = todo.text;

    const delBtn = document.createElement("button");
    delBtn.className = "del-btn";
    delBtn.textContent = "✕";
    delBtn.addEventListener("click", () => deleteTodo(todo.id));

    li.append(checkBtn, span, delBtn);
    todoList.appendChild(li);
  });
}

async function addTodo() {
  const text = todoInput.value.trim();
  if (!text) return;
  await fetch(`${API}/todos`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ text }),
  });
  todoInput.value = "";
  renderTodos();
}

async function toggleTodo(id) {
  await fetch(`${API}/todos/${id}`, { method: "PUT" });
  renderTodos();
}

async function deleteTodo(id) {
  await fetch(`${API}/todos/${id}`, { method: "DELETE" });
  renderTodos();
}

addBtn.addEventListener("click", addTodo);
todoInput.addEventListener("keydown", (e) => {
  if (e.key === "Enter") addTodo();
});

renderTodos();

// ========== 番茄钟 ==========
const timerDisplay = document.getElementById("timerDisplay");
const startBtn = document.getElementById("startBtn");
const resetBtn = document.getElementById("resetBtn");

let timerSeconds = 25 * 60;
let timerInterval = null;
let isRunning = false;

function formatTime(sec) {
  const m = String(Math.floor(sec / 60)).padStart(2, "0");
  const s = String(sec % 60).padStart(2, "0");
  return `${m}:${s}`;
}

function updateTimerDisplay() {
  timerDisplay.textContent = formatTime(timerSeconds);
}

startBtn.addEventListener("click", () => {
  if (isRunning) {
    clearInterval(timerInterval);
    isRunning = false;
    startBtn.textContent = "继续";
  } else {
    isRunning = true;
    startBtn.textContent = "暂停";
    timerInterval = setInterval(() => {
      if (timerSeconds <= 0) {
        clearInterval(timerInterval);
        isRunning = false;
        startBtn.textContent = "开始";
        alert("🍅 番茄钟结束！休息一下吧。");
        timerSeconds = 25 * 60;
        updateTimerDisplay();
        return;
      }
      timerSeconds--;
      updateTimerDisplay();
    }, 1000);
  }
});

resetBtn.addEventListener("click", () => {
  clearInterval(timerInterval);
  isRunning = false;
  timerSeconds = 25 * 60;
  startBtn.textContent = "开始";
  updateTimerDisplay();
});

// ========== 每日名言（调用后端 API） ==========
async function loadQuote() {
  const res = await fetch(`${API}/quote`);
  const data = await res.json();
  document.getElementById("quoteText").textContent = data.text;
}

loadQuote();
