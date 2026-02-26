# NU17112 PowerBank Platform - Agent Team

这是 NU17112 PowerBank 嵌入式平台的 Claude Agent 团队配置。

## 📊 Team 组成

- **1 个 Leader**: powerbank-leader（项目总协调）
- **10 个功能域 Agent**: 分别管理 HAL、FML、USB、DPDM、WPC协议、WPC硬件、Buck-Boost、Gauge、应用层、端口管理

## 📁 目录结构

```
.claude/
├── agents/                        # 11个Agent定义文件 (.md)
│   ├── powerbank-leader.md
│   ├── platform-hal-agent.md
│   ├── platform-fml-agent.md
│   └── ... (共11个)
├── agent_knowledge_*.md           # 11个Agent知识库文件
├── KNOWLEDGE_CHECKLIST.md         # 学习资料需求清单 (31条)
└── references/                    # 学习资料存放目录
    ├── datasheets/                # 芯片数据手册
    ├── specs/                     # 协议规范
    ├── schematics/                # 原理图
    ├── app_notes/                 # 应用笔记
    ├── test_reports/              # 测试报告
    ├── lessons_learned/           # 经验教训
    └── feedback/                  # 硬件测试反馈
```

## 🚀 使用方法

1. **加载 Agent**: Agent 定义文件已放入 `.claude/agents/`，Claude Code 会自动加载
2. **提供学习资料**: 将芯片手册、协议规范等放入 `references/` 对应子目录
3. **更新 Checklist**: 提供资料后，在 `KNOWLEDGE_CHECKLIST.md` 中更新状态 ❌ → 📋 → ✅
4. **Agent 学习**: Agent 会自动 Read 新提供的资料并更新知识库

## 📖 主要文档

- `ARCHITECTURE_ANALYSIS.md` - 完整架构分析 (1237行)
- `TEAM_DESIGN.md` - Team 设计方案 (1340行)
- `KNOWLEDGE_CHECKLIST.md` - 学习资料清单 (31条需求)

## 🔧 Agent 能力

每个 Agent 都具备：
- ✅ 深度代码知识（Level 3-4）
- ✅ 接口协作定义
- ✅ 双闭环验证机制
- ✅ 自我迭代能力
- ✅ 定制化指南

## 📊 统计

- **源文件管辖**: 169个 (.c/.h/startup 文件)
- **Knowledge 深度**: Level 3-4 (逻辑理解 + 专家洞察)
- **接口定义**: 22个跨Agent接口
- **学习资料需求**: 31条（按优先级分类）

---

生成时间: 2026-02-15  
生成工具: init-orchestrator
