# OpenCode

## 概述

OpenCode 是一款开源的 AI 编程代理，旨在通过终端、桌面应用或 IDE 扩展帮助开发者编写代码。它支持多种大语言模型（LLM），包括 Claude、GPT、Gemini 等，并提供隐私保护、多会话管理等功能。

## 主要特点

- **多模型支持**：支持 75+ 种 LLM 提供商，包括本地模型
- **多平台可用**：提供终端界面、桌面应用和 IDE 扩展
- **隐私优先**：不存储任何代码或上下文数据
- **LSP 集成**：自动为 LLM 加载合适的 LSP
- **多会话管理**：可在同一项目上并行启动多个代理
- **共享链接**：可分享会话链接用于参考或调试
- **GitHub Copilot 集成**：可通过 GitHub 账户使用 Copilot
- **ChatGPT 集成**：支持 OpenAI 的 ChatGPT Plus/Pro 账户

## 安装方式

### 通用安装（推荐）
```bash
curl -fsSL https://opencode.ai/install | bash
```

### 通过包管理器安装

| 平台 | 命令 |
|------|------|
| npm | `npm install -g opencode-ai` |
| Bun | `bun install -g opencode-ai` |
| Homebrew | `brew install anomalyco/tap/opencode` |
| Arch Linux | `sudo pacman -S opencode` |
| Windows (Chocolatey) | `choco install opencode` |
| Windows (Scoop) | `scoop install opencode` |
| Docker | `docker run -it --rm ghcr.io/anomalyco/opencode` |

## 配置与使用

1. **配置 API 密钥**：通过 `/connect` 命令配置 LLM 提供商
2. **初始化项目**：在项目目录运行 `opencode`，然后执行 `/init` 命令
3. **开始使用**：提问、添加功能、修改代码或撤销更改

## 使用场景

| 场景 | 说明 |
|------|------|
| 代码解释 | 询问代码库的工作原理 |
| 功能开发 | 通过计划模式规划并实现新功能 |
| 代码重构 | 直接请求代码修改或重构 |
| 错误修复 | 描述问题并获取修复建议 |
| 学习辅助 | 理解不熟悉的代码库部分 |

## 相关资源

- 官方网站: https://opencode.ai
- GitHub 仓库: https://github.com/anomalyco/opencode
- 文档: https://opencode.ai/docs
- Discord 社区: https://opencode.ai/discord