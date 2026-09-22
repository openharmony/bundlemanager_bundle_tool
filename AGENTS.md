# AGENTS.md

本文件是给 AI 编码助手使用的仓库工作指南。所有在本仓库中进行的分析、修改、测试和提交说明，都应优先遵循这里的约定。

## 项目定位

本仓库属于 OpenHarmony Bundle Manager Bundle Tool（bm 命令行工具），提供 `bm` 和 `ohos-bm` 两个命令行工具，支持应用的安装、卸载、查询、清理、使能/禁用、快速修复、编译 AOT、Overlay 查询等操作。同时提供 `bundle_test_tool` 测试工具，用于调试和验证包管理能力。

工作时请把它视为调试工具组件：命令参数兼容性、错误码映射、IPC 调用可靠性和用户输出格式一致性都很重要。工具通过 IPC 调用 BMS（SA 401），不直接操作文件系统或数据库。

## 工作原则

- 修改前先阅读相关目录、调用链和已有测试，优先沿用现有风格。
- 保持改动范围收敛，不做与任务无关的重构、格式化或命名调整。
- 不回滚用户已有改动；遇到工作区脏文件时，只处理与当前任务直接相关的文件。
- 对命令参数解析、IPC 调用、错误码映射、userId 语义等逻辑保持谨慎。
- 新增行为应尽量有测试或至少说明无法测试的原因。
- 日志、错误码、用户输出消息应遵循仓库已有模式。
- `bm` 工具使用短选项（`-h`/`-p`/`-n`），`ohos-bm` 工具使用长选项（`--help`/`--bundleName`），不要混用风格。

## 代码结构

### 目录与分层

```
用户 (hdc shell → bm / ohos-bm 命令)
  ↓
frameworks/ bm 工具实现层 (C++ · bm 可执行文件)
  ├─ BundleManagerShellCommand (Shell 命令框架 · 命令分发)
  ├─ BundleCommand (bm 命令实现 · install/uninstall/dump/clean/enable/disable/get/quickfix/compile/copy-ap/dump-overlay/...)
  ├─ BundleCommandCommon (公共工具 · GetBundleMgrProxy/GetCurrentUserId)
  ├─ QuickFixCommand (快速修复命令)
  ├─ StatusReceiverImpl (安装状态接收 · promise/future 异步等待)
  ├─ ShellCommand (Shell 命令基类 · getopt 参数解析)
  └─ BundleTestTool (测试工具 · bundle_test_tool 可执行文件 · 大量调试命令)
ohos_bm/ ohos-bm 工具实现层 (C++ · ohos-bm 可执行文件 · CLI 工具)
  ├─ BundleManagerShellCommand (Shell 命令框架 · 长选项风格)
  ├─ BundleCommand (ohos-bm 命令实现 · uninstall/dump/clean/set-disposed-rule/recover/createCliSandboxApp/...)
  ├─ ErrorCodeUtils (错误码工具 · JSON 格式错误输出)
  ├─ ShellCommand (Shell 命令基类)
  └─ StatusReceiverImpl (安装状态接收)
test/ 测试层
  ├─ unittest/bm/ (单元测试)
  ├─ moduletest/bm/ (模块测试)
  ├─ systemtest/bm/ (系统测试)
  └─ mock/ (Mock 对象 · mock_bundle_mgr_host/mock_bundle_installer_host)
```

### 关键目录

- `frameworks/include/`：bm 工具头文件（`BundleCommand`、`BundleCommandCommon`、`BundleTestTool`、`QuickFixCommand`、`ShellCommand`、`StatusReceiverImpl`）。
- `frameworks/include/bundle_tool_callback/`：工具回调接口（`IBundleToolCallback`、`BundleToolCallbackStub`）。
- `frameworks/src/`：bm 工具实现代码（含 `main.cpp` 入口）。
- `ohos_bm/include/`：ohos-bm 工具头文件。
- `ohos_bm/src/`：ohos-bm 工具实现代码（含 `main.cpp` 入口和 `error_code_utils.cpp`）。
- `ohos_bm/config.json`：ohos-bm CLI 工具配置。
- `ohos_bm/docs/`：ohos-bm 文档。
- `test/unittest/bm/`：单元测试。
- `test/moduletest/bm/`：模块测试（install/uninstall/dump）。
- `test/systemtest/bm/`：系统测试（install/uninstall/dump）。
- `test/mock/`：Mock 对象（`mock_bundle_mgr_host`、`mock_bundle_installer_host`）。
- `test/sceneProject/`：测试场景工程（HAP 测试包）。
- `bundletool.gni`：编译功能开关定义。
- `bundle.json`：组件元信息和依赖声明。

### 查找路径

按任务类型快速定位关键文件：

| 任务类型 | 关键文件 |
|----------|---------|
| 新增/修改 bm 命令 | `frameworks/include/bundle_command.h` + `frameworks/src/bundle_command.cpp` + `frameworks/src/main.cpp` |
| 新增/修改 ohos-bm 命令 | `ohos_bm/include/bundle_command.h` + `ohos_bm/src/bundle_command.cpp` + `ohos_bm/src/main.cpp` |
| 修改命令参数解析 | `frameworks/src/shell_command.cpp`（bm）或 `ohos_bm/src/shell_command.cpp`（ohos-bm） |
| 修改公共工具 | `frameworks/include/bundle_command_common.h` + `frameworks/src/bundle_command_common.cpp` |
| 修改快速修复命令 | `frameworks/include/quick_fix_command.h` + `frameworks/src/quick_fix_command.cpp` |
| 修改安装状态接收 | `frameworks/include/status_receiver_impl.h` + `frameworks/src/status_receiver_impl.cpp` |
| 修改测试工具 | `frameworks/include/bundle_test_tool.h` + `frameworks/src/bundle_test_tool.cpp` + `frameworks/src/main_test_tool.cpp` |
| 修改错误码工具 | `ohos_bm/include/error_code_utils.h` + `ohos_bm/src/error_code_utils.cpp` |
| 修改工具回调 | `frameworks/include/bundle_tool_callback/i_bundle_tool_callback.h` + `bundle_tool_callback_stub.h` |
| 修改编译开关 | `bundletool.gni` + `frameworks/BUILD.gn` + `ohos_bm/BUILD.gn` |
| 编写单元测试 | `test/unittest/bm/` |
| 编写模块测试 | `test/moduletest/bm/` |
| 编写系统测试 | `test/systemtest/bm/` |
| 修改 Mock 对象 | `test/mock/mock_bundle_mgr_host.cpp` + `mock_bundle_installer_host.cpp` |

## 知识路由

本工程无独立 skill 目录，但依赖 `bundle_framework` 仓的 IPC 接口和数据结构。命中以下场景时，建议参考 `bundle_framework` 仓的对应 skill 或文档。

### 按场景路由

| 任务场景 | 参考方向 |
|----------|---------|
| 安装/卸载流程、InstallParam、BundleInstaller IPC | `bms-install-flow` |
| IBundleMgr IPC 接口、查询能力 | `bms-add-ipc` |
| 权限校验、系统应用校验 | `bms-security-verify` |
| userId 语义、多用户、GetCurrentUserId | `bms-user-model` |
| 日志新增/修改/审查、APP_LOG_TAG | `bms-logging` |
| 测试编写、mock 选择、GN 测试 target | `bms-testing-patterns` |
| 源码定位、模块职责、调用链 | `bms-navigation` |

### 按路径路由

当变更涉及以下路径时，编辑前必须先阅读对应文件和关联知识：

| 变更路径 | 需先阅读 | 原因 |
|----------|---------|------|
| `frameworks/src/bundle_command.cpp` | 命令分发逻辑（`CreateCommandMap`）、`RunAs*Command` 方法、getopt 参数解析、`IBundleMgr`/`IBundleInstaller` IPC 调用 | bm 命令核心实现，变更影响所有 bm 命令行为 |
| `ohos_bm/src/bundle_command.cpp` | ohos-bm 命令分发、长选项解析、`CreateSuccessResult`/`CreateErrorResult` JSON 输出格式 | ohos-bm 命令核心实现，变更影响 ohos-bm 命令行为和输出格式 |
| `frameworks/src/shell_command.cpp` | `ShellCommand` 基类、`OnCommand`/`ExecCommand` 流程、getopt_long 用法 | Shell 命令框架，变更影响所有命令的参数解析 |
| `frameworks/src/bundle_command_common.cpp` | `GetBundleMgrProxy` SA 获取逻辑、`GetCurrentUserId` 用户 ID 推导、`errCodeMap_` 错误码映射 | 公共工具逻辑，变更影响所有命令的 IPC 获取和错误码 |
| `frameworks/src/status_receiver_impl.cpp` | `StatusReceiverImpl` promise/future 异步等待、`OnFinished` 回调、`waittingTime` 超时 | 安装状态接收，变更影响安装命令的异步结果获取 |
| `frameworks/src/quick_fix_command.cpp` | `ApplyQuickFix`/`GetApplyedQuickFixInfo`、`QuickFixResult` 结构 | 快速修复命令，变更影响补丁安装和查询 |
| `frameworks/src/bundle_test_tool.cpp` | `BundleTestTool` 大量调试命令、`IBundleMgr`/`IBundleResource` IPC 调用 | 测试工具核心，变更影响调试能力 |
| `ohos_bm/src/error_code_utils.cpp` | `GetErrorCodeString` 错误码到字符串映射、JSON 输出格式 | 错误码工具，变更影响 ohos-bm 错误输出 |
| `bundletool.gni` | 功能开关（`account_enable_bm`/`overlay_install_bm`/`quick_fix_bm`/`distributed_bundle_framework_bm`）、`global_parts_info` 依赖检查 | 编译开关，变更影响构建范围和功能可用性 |
| `frameworks/BUILD.gn` | `tools_bm_source_set`/`tools_test_bm_source_set` 编译目标、条件编译分支（`ACCOUNT_ENABLE`/`BUNDLE_FRAMEWORK_OVERLAY_INSTALLATION`/`BUNDLE_FRAMEWORK_QUICK_FIX`/`DISTRIBUTED_BUNDLE_FRAMEWORK`） | 构建配置，变更影响编译范围 |
| `ohos_bm/BUILD.gn` | `tools_ohos_bm_source_set` 编译目标、`ohos_cli_executable` 配置、`config.json` CLI 配置 | ohos-bm 构建配置，变更影响 CLI 工具打包 |

### 领域词汇路由

当任务描述、issue、日志、API 名称或变更文件涉及以下术语时，应先了解其语义再规划：

| 术语 | 风险提示 | 说明 |
|------|---------|------|
| bm / ohos-bm | 两个不同的命令行工具 | `bm` 使用短选项（`-h`/`-p`/`-n`），`ohos-bm` 使用长选项（`--help`/`--bundleName`） |
| BundleManagerShellCommand | Shell 命令主类 | 继承 `ShellCommand`，实现 `CreateCommandMap`/`CreateMessageMap`/`Init` |
| ShellCommand | Shell 命令基类 | 基于 getopt/getopt_long 的参数解析框架 |
| BundleCommandCommon | 公共工具类 | `GetBundleMgrProxy` 获取 BMS SA 401 代理、`GetCurrentUserId` 推导用户 ID |
| StatusReceiverImpl | 安装状态接收器 | 继承 `StatusReceiverHost`，使用 `promise<int32_t>` 异步等待安装结果 |
| IBundleMgr / IBundleInstaller | BMS IPC 接口 | 工具通过 `samgr` 获取 SA 401 代理，调用查询和安装接口 |
| errCodeMap_ | 错误码映射表 | 将 BMS 内部错误码映射为用户可读消息 |
| bundle_test_tool | 测试工具可执行文件 | 大量调试命令（sandbox/install rule/quick fix/资源查询等），不随正式版本安装 |
| account_enable_bm | 账号功能开关 | `bundletool.gni` 中控制 `ACCOUNT_ENABLE` 条件编译 |
| overlay_install_bm | Overlay 安装开关 | `bundletool.gni` 中控制 `BUNDLE_FRAMEWORK_OVERLAY_INSTALLATION` |
| quick_fix_bm | 快速修复开关 | `bundletool.gni` 中控制 `BUNDLE_FRAMEWORK_QUICK_FIX` |
| distributed_bundle_framework_bm | 分布式包管理开关 | `bundletool.gni` 中控制 `DISTRIBUTED_BUNDLE_FRAMEWORK`，影响跨设备查询命令 |
| APP_LOG_TAG | 日志标签 | `frameworks/BUILD.gn` 中定义为 `"BMSTool"`，`ohos_bm/BUILD.gn` 中为 `"OHOS_BMSTool"` |
| InstallParam | 安装参数 | 包含 userId/flag/waittingTime 等，传给 `IBundleInstaller` |
| UninstallParam | 卸载参数 | 包含 versionCode 等，用于共享库卸载 |
| QuickFixResult | 快速修复结果 | 包含补丁部署/切换/删除的结果信息 |

### 规划声明

在开始编辑前，必须明确：
- 任务属于哪类场景（新增命令/修改参数解析/修改 IPC 调用/修改错误码/修改测试工具/...）
- 变更影响 `bm` 还是 `ohos-bm` 还是 `bundle_test_tool`
- 已读取哪些相关文件或文档
- 发现了哪些约束或边界
- 是否涉及 IPC 调用或错误码映射

## 约束与边界

### 架构与业务不变量

- **三可执行文件架构**：`bm`（`frameworks/`，短选项风格，正式版本安装）、`ohos-bm`（`ohos_bm/`，长选项风格，CLI 工具）、`bundle_test_tool`（`frameworks/`，测试工具，不随正式版本安装）。三者共享 `BundleCommandCommon` 和 `ShellCommand` 基类，但命令集和参数风格不同。
- **IPC 客户端定位**：工具是 BMS（SA 401）的 IPC 客户端，通过 `samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID)` 获取 `IBundleMgr` 代理。工具不直接操作文件系统或数据库，所有操作通过 IPC 委托给 BMS。
- **异步安装等待**：`StatusReceiverImpl` 使用 `promise<int32_t>` 等待安装结果，超时由 `waittingTime` 控制（默认 180s，最小 5s，最大 600s）。修改超时逻辑须评估对用户体验的影响。
- **错误码映射**：`BundleCommandCommon::errCodeMap_` 将 BMS 内部错误码映射为用户可读消息。`ohos-bm` 使用 `ErrorCodeUtils` 输出 JSON 格式错误（`CreateSuccessResult`/`CreateErrorResult`）。
- **功能开关条件编译**：`bundletool.gni` 定义 4 个开关（`account_enable_bm`/`overlay_install_bm`/`quick_fix_bm`/`distributed_bundle_framework_bm`），对应 `BUILD.gn` 中的 `defines` 条件编译分支。修改开关默认值须验证所有条件分支。
- **userId 语义**：`BundleCommandCommon::GetCurrentUserId` 推导当前活跃用户。`-u` 参数仅支持当前活跃用户或 0 用户，不支持指定用户（会输出警告并使用当前活跃用户）。
- **命令参数兼容性**：`bm` 使用短选项（`-h`/`-p`/`-n`/`-m`/`-u`/`-r`/`-s`/`-w`/`-d`/`-g`/`-k`/`-v`/`-a`/`-c`/`-l`/`-q`/`-f`/`-t`/`-o`/`-i`），`ohos-bm` 使用长选项（`--help`/`--bundleName`/`--moduleName`/`--userId`/...）。新增命令须遵循对应风格，不要混用。
- `bundle_test_tool` 不随正式版本安装（`install_enable = false`）。

### Do not

- 不要在工具中直接操作文件系统或数据库；所有操作必须通过 IPC 委托给 BMS。
- 不要混用 `bm` 短选项和 `ohos-bm` 长选项风格。
- 不要修改已有命令的参数语义或默认值，除非任务明确要求。
- 不要修改或删除已有错误码映射；新增错误码须同步更新 `errCodeMap_` 和 `ErrorCodeUtils`。
- 不要跳过 `GetCurrentUserId` 的用户推导逻辑来使测试通过。
- 不要修改 `bundletool.gni` 中的功能开关默认值，除非任务明确要求。
- 不要做与任务无关的重构、格式化或命名调整。
- 不要回滚用户已有改动；遇到工作区脏文件时，只处理与当前任务直接相关的文件。
- 不要在 `bundle_test_tool` 中引入正式版本不允许的依赖。

### Ask before

以下变更必须先向用户确认，不得自行决定：

- 新增/修改/删除 `bm` 或 `ohos-bm` 命令（影响用户可见接口）。
- 修改已有命令的参数语义、默认值或输出格式。
- 修改 `ShellCommand` 基类的参数解析逻辑。
- 修改 `StatusReceiverImpl` 的异步等待逻辑或超时默认值。
- 修改 `BundleCommandCommon::errCodeMap_` 或 `ErrorCodeUtils` 的错误码映射。
- 修改 `bundletool.gni` 中的功能开关默认值。
- 新增第三方依赖或修改已有依赖版本。
- 修改 `bundle_test_tool` 的命令集（影响测试团队能力）。

### 已知易错点

- 新增命令只在 `bm` 或 `ohos-bm` 一侧添加，忘记同步另一侧（如果功能相同）。
- 新增错误码但未更新 `errCodeMap_` 和 `ErrorCodeUtils`，导致用户看到未知错误。
- 修改 `ShellCommand` 基类但未验证所有子类的命令分发不受影响。
- 修改 `GetCurrentUserId` 逻辑但未测试多用户场景（当前活跃用户与 0 用户）。
- 修改功能开关后未验证条件编译分支覆盖（`ACCOUNT_ENABLE`/`BUNDLE_FRAMEWORK_OVERLAY_INSTALLATION`/`BUNDLE_FRAMEWORK_QUICK_FIX`/`DISTRIBUTED_BUNDLE_FRAMEWORK`）。
- 修改 `StatusReceiverImpl` 超时逻辑但未评估对长时间安装操作的影响。
- `bm` 和 `ohos-bm` 的 `BundleManagerShellCommand` 类同名但实现不同，修改时选错了文件。

## 验证闭环

### 最小验证命令

构建命令从 OpenHarmony 源码根目录执行，不在本子目录执行。

```bash
# 编译验证
./build.sh --product-name rk3568 --build-target bundle_tool

# 编译 bm 工具
./build.sh --product-name rk3568 --build-target frameworks:bm

# 编译 ohos-bm 工具
./build.sh --product-name rk3568 --build-target ohos_bm:cli_tools_bm

# 单元测试
./build.sh --product-name rk3568 --build-target test:unittest

# 模块测试
./build.sh --product-name rk3568 --build-target test:moduletest

# 系统测试
./build.sh --product-name rk3568 --build-target test:systemtest
```

如果当前环境缺少 OpenHarmony 构建链、产品配置或依赖仓库，请不要伪造构建结果；在最终说明中明确写出未能运行的命令和原因。

### 静态分析与 Sanitize 检查

项目在 `frameworks/BUILD.gn` 和 `ohos_bm/BUILD.gn` 中已启用以下 sanitize 选项，编译时自动生效：

- `boundary_sanitize`（边界检查）
- `cfi` + `cfi_cross_dso`（控制流完整性）
- `integer_overflow`（整数溢出检查）
- `ubsan`（未定义行为检查）
- `fstack-protector-strong`（栈溢出保护）

`-Oz` 优化体积，`-fvisibility=hidden` 控制符号导出。如环境支持，可运行 `cppcheck` 补充静态分析：

```bash
cppcheck --enable=warning,performance,portability --std=c++17 frameworks/src/ ohos_bm/src/
```

关注 sanitize 编译报错和 `cppcheck` 警告，特别是空指针解引用、整数溢界和 IPC 参数校验。

### 按变更类型验证

| 变更类型 | 最小验证 |
|----------|---------|
| 修改 bm 命令实现（frameworks/src/bundle_command.cpp） | 编译通过 + 相关命令单测 + 模块测试 |
| 修改 ohos-bm 命令实现（ohos_bm/src/bundle_command.cpp） | 编译通过 + ohos_bm 单测 |
| 新增命令 | 编译通过 + 新命令单测 + 帮助信息验证 + `CreateCommandMap` 注册确认 |
| 修改参数解析（shell_command.cpp） | 编译通过 + 全量单测 + 所有命令参数解析验证 |
| 修改 IPC 调用 | 编译通过 + 相关单测 + Mock 对象验证 |
| 修改错误码映射 | 编译通过 + 错误码相关单测 + 搜索所有 `GetMessageFromCode` 调用方 |
| 修改 StatusReceiverImpl | 编译通过 + 安装/卸载相关模块测试 + 超时场景验证 |
| 修改功能开关 | 编译通过 + 条件编译分支验证 + 开关开关两种状态编译 |
| 修改测试工具 | 编译通过 + `bundle_test_tool` 单测 |
| 测试变更 | 运行变更的测试 + 至少一个相邻相关测试 |

### Done 定义

一个任务只有同时满足以下条件才算完成：

1. 请求的行为已实现。
2. 相关编译、测试、兼容性验证已运行，或已说明无法运行的原因。
3. `git diff` 仅包含预期改动，无无关重构或格式化。
4. 新增或修改的错误路径有清晰返回值和用户可读消息。
5. 新增命令已在 `CreateCommandMap` 中注册，帮助信息已更新。
6. `bm` 和 `ohos-bm` 同功能变更已同步（如适用）。
7. 修改 IPC 调用时，已确认参数序列化和反序列化兼容。

### 最终回复格式

向用户汇报时请包含：

- 改了哪些文件。
- 行为上解决了什么问题。
- 运行了哪些验证及结果。
- 哪些验证因环境限制未运行，以及残留风险。
- 如涉及用户可见接口变更，说明兼容性影响。
