# SOE103 Group Project: Pizza Ordering System

SOE103 C Programming and Software Engineering 小组作业，截止时间 **2026-12-13 23:59**（计划 12-12 提交）。

## 文档
- `docs/2026-27_GroupProject.pdf`：作业说明（含评分标准和贡献表）
- `docs/SOE103_GroupProject_Outline.pdf`：项目大纲
- `docs/design_draft.md`：需求分析与设计草稿（标"提议"的内容需组会确认）
- `docs/SDP Report Example.pdf`：SDP 报告样例

## 模块与分工（待组会确认）
| 成员 | 负责文件 | 内容 |
|---|---|---|
| A（组长） | `main.c` `auth.c` `utils.c` `common.h` | 主菜单、登录、安全输入、合并与编译 exe |
| B | `menu.c` | 菜单编辑、定价、算价 |
| C | `customer.c` | 顾客账户增删改、注册、充值 |
| D | `order.c` | 购物车、结账、订单历史 |
| E | `fileio.c` `stats.c` | 数据文件读写、销售统计 |

## 编译
- Linux / macOS：`make`
- Windows（MinGW）：双击或运行 `build.bat`，生成 `.exe`

## 协作约定
1. 每人在自己的分支上开发（如 `feature/customer`），完成后提 Pull Request 合并到 `main`。
2. `.h` 接口冻结后要修改，先在群里说。
3. 输入一律用 `fgets`，不用 `scanf`；每个函数写注释。
4. 提交前确保 `gcc -Wall -Wextra -std=c99` 无警告。
