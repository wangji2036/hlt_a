# Agent 测试清单

运行以下命令测试 agents 是否正常工作：

## 1. 列出所有 agents
输入: `/agents`
预期: 显示 11 个 agents（powerbank-leader + 10 个 platform-*-agent）

## 2. 测试 Leader
输入: `@powerbank-leader 介绍一下你的团队`
预期: Leader 回复团队组成

## 3. 测试具体 agent
输入: `@platform-usb-agent USB PD 协商流程是什么？`
预期: USB agent 回复 PD 协商相关知识

## 4. 测试协作
输入: `@platform-hal-agent OSAL 调度器如何工作？`
预期: HAL agent 回复 OSAL 相关知识

---

如果以上命令都无响应，可能需要：
1. 重新加载 VSCode 窗口 (Ctrl+Shift+P → "Reload Window")
2. 确认当前工作区是项目根目录
3. 检查 Claude Code 扩展版本
