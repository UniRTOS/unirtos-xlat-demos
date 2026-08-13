# unirtos-xlat-demos

中文 | [English](README.md)

本仓库推荐通过 unirtos-cli 的 demo 工作流使用，以保证创建、环境拉取和编译流程一致。

## 功能描述

本 Demo 展示 UniRTOS 上基于 464XLAT 的 CLAT（Customer-side Translator，客户侧地址转换）基础开发流程，适合作为 IPv4/IPv6 网络转换与 PING 验证开发的入门样例。

- 演示获取并打印当前 CLAT 开关状态与完整配置参数
- 演示启用 CLAT 功能（设置 `clat_switch` 为 1）
- 演示通过已启用 CLAT 的数据连接（PDP）向 IPv4 服务器（`8.8.8.8`）发起 PING 测试
- 演示等待 PING 完成并打印结果统计（RTT、丢包率等）
- 演示读取 CLAT 完整配置（DNS64 模式、目标前缀、FQDN、TOS、DF 标志等）
- 包含异步消息队列驱动的 PING 回调处理框架

## 快速上手

### 1. 安装 UniRTOS 工具链

- [开发准备](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/开发准备/开发准备.html)
- [安装交叉编译工具链](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/环境搭建/环境搭建.html)
- [安装 Python3](https://www.python.org/downloads/)
- [安装 git](https://git-scm.com)
- 安装 unirtos-cli：`pip install unirtos-cli`

安装完成后，确认以下命令可用：

```bash
python --version       # Python3
git --version
unirtos --version      # 1.0.5 及以上版本
unirtos-cli version    # 1.0.11 及以上版本
```

### 2. 使用 unirtos-cli 拉取 demo

查看可用 demo 与版本：

```bash
unirtos-cli ls-demos
```

创建本 demo 工程：

```bash
unirtos-cli new -r unirtos-xlat-demos
```

如需指定版本：

```bash
unirtos-cli new -r unirtos-xlat-demos -v 1.0.0
```

### 3. 进入工程并编译

```bash
cd unirtos-xlat-demos-1.0.0
unirtos-cli env-setup
unirtos-cli build
```

## 常用命令

```bash
# 打开 SDK 菜单配置
unirtos-cli menuconfig

# 清理构建产物
unirtos-cli clean
```

## 技术社区

技术社区：https://forumschinese.quectel.com/c/66-category/66

## 贡献指南

欢迎参与共建，建议按以下方式提交：
- 提交前先执行一次基础验证：env-setup、build、clean。
- 使用清晰的提交说明，描述改动目的、影响范围和验证结果。
- 新增功能或行为变化时，同步更新 README 与相关文档。
- 通过 Issue 或 Pull Request 提交问题修复与功能改进。
