# ohos-bm

## 1. 概述

`ohos-bm` 是 OpenHarmony 系统的包管理器命令行工具，用于管理应用程序包。它提供了应用包的卸载、信息查看、依赖关系查询、共享库管理、缓存/数据清理以及克隆应用处置规则管理等功能。

### 文件结构

```
ohos_bm/
├── include/                      # 头文件目录
│   ├── bundle_command.h          # 命令处理主类
│   ├── bundle_command_common.h   # 公共工具类（代理获取、用户ID处理）
│   ├── error_code_utils.h        # 错误码转换工具
│   ├── shell_command.h           # CLI命令解析基类
│   └── status_receiver_impl.h    # 异步状态接收器实现
├── src/                          # 源文件目录
│   ├── main.cpp                  # 主入口
│   ├── bundle_command.cpp        # 命令处理实现
│   ├── bundle_command_common.cpp # 公共工具实现
│   ├── error_code_utils.cpp      # 错误码转换实现
│   ├── shell_command.cpp         # 命令解析与分发实现
│   └── status_receiver_impl.cpp  # 异步结果处理实现
├── test/                         # 测试代码目录
│   ├── unittest/                 # 单元测试
│   │   └── ohos_bm_command_test.cpp
│   └── BUILD.gn                  # 测试构建配置
├── docs/                         # 文档目录
│   └ readme.md                   # 参考文档
├── config.json                   # CLI配置文件（Claw规范）
└── BUILD.gn                      # 构建配置
```

## 2. CLI子命令表

| 子命令 | 作用 | 可选参数 | 所需权限 |
|-------|------|---------|---------|
| `--help` | 查看ohos-bm帮助信息 | 无 | 无 |
| `uninstall` | 卸载应用包 | `--bundleName <bundle-name>`：指定要卸载的包名<br>`--keepData`：卸载后保留用户数据<br>`--shared`：卸载应用间共享库<br>`--version <version-code>`：指定共享库版本号卸载 | `ohos.permission.cli.UNINSTALL_BUNDLE` |
| `dump` | 查看应用包信息 | `--all`：列出系统中所有应用包<br>`--bundleName <bundle-name>`：查看指定包的信息<br>`--shortcutInfo`：查看快捷方式信息<br>`--deviceId <device-id>`：指定设备ID查看分布式应用信息<br>`--debugBundle`：列出调试应用包<br>`--label`：查看标签信息 | `ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED` |
| `dump-dependencies` | 查看指定应用和模块的依赖关系 | `--bundleName <bundle-name>`：指定包名<br>`--moduleName <module-name>`：指定模块名 | `ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED` |
| `dump-shared` | 查看应用间共享库信息 | `--all`：列出所有共享库名称<br>`--bundleName <bundle-name>`：查看指定共享库信息 | `ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED` |
| `clean` | 清理应用缓存或数据文件 | `--bundleName <bundle-name>`：指定包名<br>`--cache`：清理缓存文件<br>`--data`：清理数据文件<br>`--appIndex <app-index>`：指定应用索引 | `ohos.permission.cli.REMOVE_BUNDLE_DATA_AND_CACHE_FILES` |
| `set-disposed-rule` | 为克隆应用设置处置规则 | `--appId <app-id>`：应用ID或标识符（必选）<br>`--appIndex <app-index>`：克隆应用索引<br>`--priority <priority>`：处置规则优先级（必选）<br>`--componentType <type>`：组件类型（必选）：1=UI_ABILITY, 2=UI_EXTENSION<br>`--disposedType <type>`：处置类型（必选）：1=BLOCK_APPLICATION, 2=BLOCK_ABILITY, 3=NON_BLOCK<br>`--controlType <type>`：控制类型（必选）：1=ALLOWED_LIST, 2=DISALLOWED_LIST<br>`--elements <element-uri>`：要控制的元素，格式：/bundleName/moduleName/abilityName，可多次使用<br>`--wantBundleName <name>`：重定向目标包名（必选）<br>`--wantModuleName <name>`：重定向目标模块名<br>`--wantAbilityName <name>`：重定向目标Ability名（必选）<br>`--wantParamsStrings <json>`：Want字符串参数<br>`--wantParamsInts <json>`：Want整数参数<br>`--wantParamsBools <json>`：Want布尔参数 | `ohos.permission.cli.MANAGE_DISPOSED_APP_STATUS` |
| `delete-disposed-rule` | 删除克隆应用的处置规则 | `--appId <app-id>`：应用ID或标识符（必选）<br>`--appIndex <app-index>`：克隆应用索引 | `ohos.permission.cli.MANAGE_DISPOSED_APP_STATUS` |

## 3. Claw规范遵循情况

### 命令命名规范

- **主命令命名**：使用小写字母和连字符组合，格式为 `ohos-<module>`，例如 `ohos-bm`（bm代表bundle manager）
- **子命令命名**：使用小写字母，多个单词时使用连字符连接，例如 `dump-dependencies`、`set-disposed-rule`
- **参数命名**：使用小驼峰命名法（camelCase）或连字符命名法，例如 `--bundleName` 或 `--bundle-name`
- **遵循度**：✅ 完全遵循

### 输入格式规范

通过 `config.json` 文件定义输入参数的 JSON Schema：

```json
{
  "inputSchema": {
    "type": "object",
    "required": ["参数列表"],
    "properties": {
      "参数名": {
        "type": "数据类型",
        "description": "参数描述",
        "default": "默认值"
      }
    }
  }
}
```

- **参数类型定义**：明确指定每个参数的数据类型（string、integer、boolean、array等）
- **必选参数标记**：通过 `required` 字段明确标识必选参数
- **参数描述**：每个参数都有详细的 `description` 字段说明用途
- **默认值设置**：可选参数通过 `default` 字段设置默认值
- **遵循度**：✅ 完全遵循，所有子命令的输入参数都在 `config.json` 中有完整的 JSON Schema 定义

### 输出格式规范

所有命令的输出均采用 JSON 格式，包含以下统一字段：

```json
{
  "type": "result",
  "status": "success | failed",
  "data": {
    // 返回的具体数据
  },
  "errCode": "错误码字符串",
  "errMsg": "错误消息",
  "suggestion": "错误解决建议"
}
```

- **type字段**：固定为 `"result"`，标识这是执行结果
- **status字段**：取值为 `"success"` 或 `"failed"`，表示执行状态
- **data字段**：成功时包含返回的数据对象，失败时为空对象或包含错误详情
- **errCode字段**：失败时返回具体的错误码字符串（如 `ERR_DUMP_PARAM_ERROR`）
- **errMsg字段**：失败时返回详细的错误消息说明
- **suggestion字段**：失败时提供解决问题的建议或帮助信息
- **遵循度**：✅ 完全遵循，所有命令的输出都在 `config.json` 中有完整的 JSON Schema 定义

## 4. 使用示例

### 4.1 查看帮助信息

```bash
ohos-bm --help
```

### 4.2 卸载应用包

```bash
# 卸载指定包名的应用
ohos-bm uninstall --bundleName com.example.test

# 卸载应用但保留用户数据
ohos-bm uninstall --bundleName com.example.test --keepData

# 卸载应用间共享库
ohos-bm uninstall --bundleName com.example.sharedlib --shared

# 卸载指定版本的共享库
ohos-bm uninstall --bundleName com.example.sharedlib --shared --version 1000000
```

### 4.3 查看应用包信息

```bash
# 列出系统中所有应用包
ohos-bm dump --all

# 查看指定应用包的详细信息
ohos-bm dump --bundleName com.example.test

# 查看应用的快捷方式信息
ohos-bm dump --bundleName com.example.test --shortcutInfo

# 查看指定设备的分布式应用信息
ohos-bm dump --bundleName com.example.test --deviceId device123

# 列出调试应用包
ohos-bm dump --debugBundle

# 查看所有应用的标签信息
ohos-bm dump --all --label

# 查看指定应用的标签信息
ohos-bm dump --bundleName com.example.test --label
```

### 4.4 查看依赖关系

```bash
# 查看指定应用和模块的依赖关系
ohos-bm dump-dependencies --bundleName com.example.test --moduleName entry
```

### 4.5 查看共享库信息

```bash
# 列出所有应用间共享库
ohos-bm dump-shared --all

# 查看指定共享库的详细信息
ohos-bm dump-shared --bundleName com.example.sharedlib
```

### 4.6 清理应用缓存或数据

```bash
# 清理应用缓存文件
ohos-bm clean --bundleName com.example.test --cache

# 清理应用数据文件
ohos-bm clean --bundleName com.example.test --data

# 清理指定应用索引的缓存
ohos-bm clean --bundleName com.example.test --cache --appIndex 1001
```

### 4.7 设置处置规则

```bash
# 阻止整个应用并重定向
ohos-bm set-disposed-rule \
    --appId com.example.test \
    --priority 100 \
    --componentType 1 \
    --disposedType 1 \
    --controlType 1 \
    --wantBundleName com.example.redirect \
    --wantAbilityName MainAbility

# 阻止特定Ability并重定向（使用允许列表）
ohos-bm set-disposed-rule \
    --appId com.example.test \
    --priority 100 \
    --componentType 2 \
    --disposedType 2 \
    --controlType 1 \
    --elements /com.example.test/entry/MainAbility \
    --elements /com.example.test/feature/SubAbility \
    --wantBundleName com.example.redirect \
    --wantModuleName entry \
    --wantAbilityName MainAbility

# 设置带参数的处置规则
ohos-bm set-disposed-rule \
    --appId com.example.test \
    --priority 50 \
    --componentType 1 \
    --disposedType 3 \
    --controlType 2 \
    --wantBundleName com.example.target \
    --wantAbilityName MainAbility \
    --wantParamsStrings '{"key1":"value1","key2":"value2"}' \
    --wantParamsInts '{"count":10,"index":5}' \
    --wantParamsBools '{"enabled":true}'

# 为克隆应用设置处置规则
ohos-bm set-disposed-rule \
    --appId com.example.test \
    --appIndex 1001 \
    --priority 100 \
    --componentType 1 \
    --disposedType 1 \
    --controlType 1 \
    --wantBundleName com.example.redirect \
    --wantAbilityName MainAbility
```

### 4.8 删除处置规则

```bash
# 根据appId删除处置规则
ohos-bm delete-disposed-rule --appId com.example.test

# 删除克隆应用的处置规则
ohos-bm delete-disposed-rule --appId com.example.test --appIndex 1001
```