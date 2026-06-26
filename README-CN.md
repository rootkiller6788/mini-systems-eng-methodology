# Mini Systems Engineering Methodology（迷你系统工程方法论）

**从零开始、零依赖的 C 语言实现**，涵盖系统工程方法论框架。从硬系统（Hall 形态学）到软系统（Checkland 的 SSM）再到批判系统思维（Flood & Jackson），横跨完整的方法论谱系。每个模块对应 MIT、Stanford、INCOSE 等顶尖机构的课程参考，将形式化方法转化为可运行的 C 代码。

## 子模块

| 子模块 | 主题 | 参考课程 |
|------------|--------|-------------|
| [mini-critical-systems-thinking-flood](mini-critical-systems-thinking-flood/) | 批判意识、边界判断、权力结构、社会解放、TSI（全面系统干预）、SoSM、对话伦理、干预设计 | 赫尔大学（Flood/Jackson），MIT ESD.34 |
| [mini-hard-systems-methodology-hall](mini-hard-systems-methodology-hall/) | 三维形态学（时间 / 逻辑 / 知识维度）、7 个生命周期阶段、7 个问题解决步骤、需求工程、权衡分析、验证规划 | 贝尔实验室（Hall 1962），MIT 16.842，IEEE 1220 |
| [mini-model-based-systems-eng-mbse](mini-model-based-systems-eng-mbse/) | SysML v2 块建模、行为图、参数化约束、需求追溯、架构框架、OPM（对象过程方法论） | INCOSE SE 手册，ISO 15288，Stanford AA222 |
| [mini-soft-systems-methodology-checkland](mini-soft-systems-methodology-checkland/) | CATWOE 分析、根定义、概念建模、世界观比较、文化/政治流分析、干预设计 | 兰卡斯特大学（Checkland），MIT ESD.34 |
| [mini-stakeholder-analysis-tradeoff](mini-stakeholder-analysis-tradeoff/) | 利益相关方权力-利益-紧迫性映射、影响力网络、多准则决策（MCDM）、帕累托前沿分析、需求优先级排序、协商共识 | MIT ESD.36，Stanford MS&E 252，CMU 18-660 |
| [mini-system-of-systems-engineering](mini-system-of-systems-engineering/) | SoS 架构类型（层次化 / 网络化 / 中心辐射 / 全连通）、涌现分析、互操作性（LISI 模型）、治理、成本-能力权衡、GST 基础 | MIT 16.842，Stevens SoS，Georgia Tech AE 8803 |
| [mini-v-model-spiral-lifecycle](mini-v-model-spiral-lifecycle/) | V 模型分解-集成-验证追溯、螺旋模型风险驱动迭代、生命周期阶段门禁、里程碑指标、Boehm 风险分析 | MIT 16.842，CMU 18-660，Forsberg & Mooz (1991) |
| [mini-verification-validation-uncertainty](mini-verification-validation-uncertainty/) | 需求追溯矩阵、合规性测试、利益相关方需求对齐、蒙特卡洛仿真、拉丁超立方抽样、敏感性分析、风险评分 | MIT 6.241J，NASA SE 手册，AIAA V&V 指南 |

## 设计理念

- **零外部依赖** — 纯 C（C99/C11），仅使用 `libc` 和 `libm`
- **模块自包含** — 每个目录自带 `Makefile`、`include/`、`src/`、`examples/`、`demos/`、`tests/`
- **理论到代码的映射** — 每个模块包含 `docs/` 目录，内有方法论到实现的对照说明
- **实用演示程序** — 生命周期仿真器、权衡可视化、利益相关方网络分析器、CATWOE 建模工具、蒙特卡洛引擎等

## 构建方式

每个模块相互独立。进入模块目录后运行：

```bash
cd mini-hard-systems-methodology-hall
make all    # 构建全部
make test   # 运行测试
```

需要 **GCC** 和 **GNU Make**。

## 项目结构

```
mini-systems-eng-methodology/
├── mini-critical-systems-thinking-flood/   # 批判系统思维（Flood & Jackson）
├── mini-hard-systems-methodology-hall/     # 硬系统方法论（Hall）
├── mini-model-based-systems-eng-mbse/      # 基于模型的系统工程（MBSE）
├── mini-soft-systems-methodology-checkland/ # 软系统方法论（Checkland）
├── mini-stakeholder-analysis-tradeoff/     # 利益相关方分析与权衡
├── mini-system-of-systems-engineering/     # 体系工程
├── mini-v-model-spiral-lifecycle/          # V 模型与螺旋模型生命周期
└── mini-verification-validation-uncertainty/ # 验证、确认与不确定性量化
```

## 许可证

MIT
