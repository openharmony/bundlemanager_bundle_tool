# **bm工具命令组件**

## 简介

bm是用来方便开发者调试的一个工具。bm工具被hdc工具封装，进入hdc shell命令后，就可以使用bm工具。


## 目录

```
foundation/bundlemanager/bundle_tool
├── frameworks                        # bm工具服务框架代码
└── test						      # 测试目录
```

### bm工具命令

| 命令 | 描述 |
| -------- | -------- |
| help | 帮助命令，显示bm支持的命令信息。 |
| install | 安装命令，用来安装应用。 |
| uninstall | 卸载命令，用来卸载应用。 |
| install-plugin | 安装插件命令，用于安装插件。|
| uninstall-plugin | 卸载插件命令，用于卸载插件。|
| dump | 查询命令，用来查询应用的相关信息。 |
| clean | 清理命令，用来清理应用的缓存和数据。此命令在root版本下可用，在user版本下打开开发者模式可用。其它情况不可用。|
| enable | 使能命令，用来使能应用，使能后应用可以继续使用。此命令在root版本下可用，在user版本下不可用。 |
| disable | 禁用命令，用来禁用应用，禁用后应用无法使用。此命令在root版本下可用，在user版本下不可用。 |
| get | 获取udid命令，用来获取设备的udid。 |
| quickfix | 快速修复相关命令，用来执行补丁相关操作，如补丁安装、补丁查询。 |
| compile | 应用执行编译AOT命令。 |
| copy-ap | 把应用的ap文件拷贝到/data/local/pgo目录下，供shell用户读取文件。 |
| dump-dependencies | 查询应用依赖的模块信息。 |
| dump-shared | 查询应用间HSP应用信息。 |
| dump-overlay | 打印overlay应用的overlayModuleInfo。 |
| dump-target-overlay | 打印目标应用的所有关联overlay应用的overlayModuleInfo。 |


#### 帮助命令（help）
```bash
bm help
```

  **表2** help命令列表

| 命令    | 描述       |
| ------- | ---------- |
| bm help | 显示bm工具的能够支持的命令信息。 |


#### 安装命令（install）

```bash
bm install [-h] [-p filePath] [-r] [-w waitingTime] [-s hspDirPath] [-u userId]
```

  **安装命令参数列表**


| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -p | 可选参数，指定HAP路径，多HAP应用可指定多HAP所在文件夹路径。也可指定APP或APP所在的文件夹路径，每次只能安装一个APP，指定路径安装app时，路径里也只能存在一个app |
| -r | 可选参数，覆盖安装一个HAP。默认值为覆盖安装。 |
| -s | 根据场景判断，安装应用间HSP时为必选参数，其他场景为可选参数。安装应用间共享库， 每个路径目录下只能存在一个同包名的HSP。 |
| -w | 可选参数，安装HAP时指定bm工具等待时间，最小的等待时长为5s，最大的等待时长为600s,&nbsp;默认缺省为180s。 |
| -u | 可选参数，指定[用户](#userid)，默认在当前活跃用户下安装应用。仅支持在当前活跃用户或0用户下安装。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm install -p /data/local/tmp/ohos.app.hap -u 102`安装时，只会在当前活跃用户100下安装应用。 |


示例：
```bash
# 安装一个hap
bm install -p /data/local/tmp/ohos.app.hap
# 在100用户下安装一个hap
bm install -p /data/local/tmp/ohos.app.hap -u 100
# 覆盖安装一个hap
bm install -p /data/local/tmp/ohos.app.hap -r
# 安装一个应用间共享库
bm install -s xxx.hsp
# 同时安装使用方应用和其依赖的应用间共享库
bm install -p aaa.hap -s xxx.hsp yyy.hsp
# 同时安装HAP和应用内共享库
bm install -p /data/local/tmp/hapPath/
# 安装一个hap,等待时间为180s
bm install -p /data/local/tmp/ohos.app.hap -w 180
```

#### 卸载命令（uninstall）

```bash
bm uninstall [-h] [-n bundleName] [-m moduleName] [-k] [-s] [-v versionCode] [-u userId]
```

  **卸载命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -n | 必选参数，指定Bundle名称卸载应用。|
| -m | 可选参数，应用模块名称，指定卸载应用的一个模块。默认卸载所有模块。 |
| -k | 可选参数，卸载应用时保存应用数据。默认卸载应用时不保存应用数据。 |
| -s | 根据场景判断，卸载应用间HSP时必选参数，其他场景为可选参数。卸载指定的共享库。|
| -v | 可选参数，指定共享包的版本号。默认卸载同包名的所有共享包。 |
| -u | 可选参数，指定[用户](#userid)，默认在当前活跃用户下卸载应用。仅支持在当前活跃用户或0用户下卸载应用。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm uninstall -n com.ohos.app -u 102`卸载时，只会在当前活跃用户100下卸载应用。 |


示例：

```bash
# 卸载一个应用
bm uninstall -n com.ohos.app
# 在用户100下卸载一个应用
bm uninstall -n com.ohos.app -u 100
# 卸载应用的一个模块
bm uninstall -n com.ohos.app -m entry
# 卸载一个shared bundle
bm uninstall -n com.ohos.example -s
# 卸载一个shared bundle的指定版本
bm uninstall -n com.ohos.example -s -v 100001
# 卸载一个应用，并保留用户数据
bm uninstall -n com.ohos.app -k
```

#### 安装插件命令（install-plugin）

```bash
bm install-plugin [-h] [-n hostBundleName] [-p filePath]
```

**install-plugin命令参数列表**
| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -n | 必选参数，指定待安装插件的应用包名。|
| -p | 必选参数，指定插件文件路径。|

示例：

```bash
# 安装一个插件
bm install-plugin -n com.ohos.app -p /data/plugin.hsp
```
> **说明：**
>
> 在同一个应用中安装同一个插件，则视作插件版本更新，插件不支持降级安装；插件版本更新后，需要重启应用插件才能生效。
>
> 不推荐安装与宿主应用模块同名的插件，目前运行态暂不支持。

#### 卸载插件命令（uninstall-plugin）

```bash
bm uninstall-plugin [-h] [-n hostBundleName] [-p pluginBundleName]
```

**uninstall-plugin命令参数列表**
| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -n | 必选参数，指定应用包名。|
| -p | 必选参数，指定插件的包名。|

示例：

```bash
# 卸载一个插件
bm uninstall-plugin -n com.ohos.app -p com.ohos.plugin
```

#### 查询应用信息命令（dump）

```bash
bm dump [-h] [-a] [-g] [-n bundleName] [-s shortcutInfo] [-d deviceId] [-l label] [-u userId]
```

  **查询命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -a | 可选参数，查询系统已经安装的所有应用。 |
| -g | 可选参数，查询系统中签名为调试类型的应用包名。 |
| -n | 可选参数，查询指定Bundle名称的详细信息。 |
| -s | 可选参数，查询指定Bundle名称下的快捷方式信息。 |
| -d | 可选参数，查询指定设备中的包信息。默认查询当前设备。 |
| -l | 可选参数，用于查询指定Bundle名称的label值（应用的名称），需要与`-n`或`-a`参数组合使用。<br/>**说明**：<br/>从API version 20开始支持该命令。如果在Windows环境下输出结果包含特殊字符或中文乱码，需在cmd控制台中手动执行命令`chcp 65001`，将cmd控制台编码修改为UTF-8。 |
| -u | 可选参数，查询指定[用户](#userid)下的应用信息，默认在当前活跃用户下查询应用信息。仅支持在当前活跃用户或0用户下查询。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm dump -n com.ohos.app -u 102`查询时，只会在当前活跃用户100下查询应用。 |


示例：

```bash
# 显示所有已安装的Bundle名称
bm dump -a
# 查询系统中签名为调试类型的应用包名
bm dump -g
# 查询该应用的详细信息
bm dump -n com.ohos.app
# 在用户100下查询该应用的详细信息
bm dump -n com.ohos.app -u 100
# 查询该应用的快捷方式信息
bm dump -s -n com.ohos.app
# 查询跨设备应用信息
bm dump -n com.ohos.app -d xxxxx
# 查询该应用的label值（应用的名称）
bm dump -n com.ohos.app -l
# 显示所有已安装应用的bundle名称和label值（应用的名称）
bm dump -a -l
```

#### 清理命令（clean）

```bash
bm clean [-h] [-c] [-n bundleName] [-d] [-i appIndex] [-u userId]
```
**清理命令参数列表**

| 参数 | 参数说明 |
| -------- | --------- |
| -h | 帮助信息。 |
| -c&nbsp;-n | -n为必选参数，-c为可选参数。清除指定Bundle名称的缓存数据。 |
| -d&nbsp;-n | -n为必选参数，-d为可选参数。清除指定Bundle名称的数据目录。 |
| -i | 可选参数，清除分身应用的数据目录。默认为0。|
| -u | 可选参数，清理指定[用户](#userid)下的数据，默认在当前活跃用户下清理数据。仅支持在当前活跃用户或0用户下清理数据。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm clean -c -n com.ohos.app -u 102`清理数据时，只会在当前活跃用户100下清理。 |


示例：

```bash
# 清理该应用下的缓存数据
bm clean -c -n com.ohos.app
# 在用户100下清理该应用下的缓存数据
bm clean -c -n com.ohos.app -u 100
# 清理该应用下的用户数据
bm clean -d -n com.ohos.app
# 执行结果
clean bundle data files successfully.
```


#### 使能命令（enable）

```bash
bm enable [-h] [-n bundleName] [-a abilityName] [-u userId]
```


  **使能命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -n | 必选参数，使能指定Bundle名称的应用。 |
| -a | 可选参数，使能指定Bundle名称下的元能力模块。 |
| -u | 可选参数，使能指定[用户](#userid)下的应用，默认在当前活跃用户下使能应用。仅支持在当前活跃用户或0用户下使能应用。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm enable -n com.ohos.app -u 102`使能应用时，只会在当前活跃用户100下使能应用。 |


示例：

```bash
# 使能该应用
bm enable -n com.ohos.app -a com.ohos.app.EntryAbility
# 在用户100下使能该应用
bm enable -n com.ohos.app -u 100
# 执行结果
enable bundle successfully.
```


#### 禁用命令（disable）

```bash
bm disable [-h] [-n bundleName] [-a abilityName] [-u userId]
```


  **禁用命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -n | 必选参数，禁用指定Bundle名称的应用。 |
| -a | 可选参数，禁用指定Bundle名称下的元能力模块。 |
| -u | 可选参数，禁用指定[用户](#userid)下的应用，默认在当前活跃用户下禁用应用。仅支持在当前活跃用户或0用户下禁用应用。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm disable -n com.ohos.app -u 102`禁用应用时，只会在当前活跃用户100下禁用应用。 |


示例：

```bash
# 禁用该应用
bm disable -n com.ohos.app -a com.ohos.app.EntryAbility
# 在用户100下禁用该应用
bm disable -n com.ohos.app -u 100
# 执行结果
disable bundle successfully.
```


#### 获取udid命令（get）

```bash
bm get [-h] [-u]
```

  **获取udid命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h |帮助信息。 |
| -u | 必选参数，获取设备的udid。|


示例：

```bash
# 获取设备的udid
bm get -u
# 执行结果
udid of current device is :
23CADE0C
```


#### 快速修复命令（quickfix）

```bash
bm quickfix [-h] [-a -f filePath [-t targetPath] [-d] [-o]] [-q -b bundleName] [-r -b bundleName]
```

  **快速修复命令参数列表**
|   参数  | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -a&nbsp;-f | -a为可选参数，指定-a后，-f为必选参数。执行快速修复补丁安装命令，file-path对应hqf文件，支持传递1个或多个hqf文件，或传递hqf文件所在的目录。 |
| -q&nbsp;-b | -q为可选参数，指定-q后，-b为必选参数。根据包名查询补丁信息。 |
| -r&nbsp;-b | -r为可选参数，指定-r后，-b为必选参数。根据包名卸载未使能的补丁。|
| -t | 可选参数，快速修复应用到指定目标路径。|
| -d | 可选参数，应用快速修复调试模式。|
| -o | 可选参数，应用快速修复覆盖模式，该模式下so将被解压覆盖到应用的so目录中。|



示例：

```bash
# 根据包名查询补丁包信息
bm quickfix -q -b com.ohos.app
# 执行结果
# Information as follows:
# ApplicationQuickFixInfo:
#  bundle name: com.ohos.app
#  bundle version code: xxx
#  bundle version name: xxx
#  patch version code: x
#  patch version name:
#  cpu abi:
#  native library path:
#  type:

# 快速修复补丁安装
bm quickfix -a -f /data/app/
# 执行结果
apply quickfix succeed.
# 快速修复补丁卸载
bm quickfix -r -b com.ohos.app
# 执行结果
delete quick fix successfully
```

#### 共享库查询命令（dump-shared）

```bash
bm dump-shared [-h] [-a] [-n bundleName]
```

  **共享库查询命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -a | 可选参数，查询系统中所有已安装的共享库。|
| -n | 可选参数，查询指定包名的共享库详细信息。|


示例：

```bash
# 显示所有已安装共享库包名
bm dump-shared -a
# 显示该共享库的详细信息
bm dump-shared -n com.ohos.lib
```

#### 共享库依赖关系查询命令（dump-dependencies）

```bash
bm dump-dependencies [-h] [-n bundleName] [-m moduleName]
```

**共享库依赖关系查询命令参数列表**
| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -n | 必选参数，查询指定应用依赖的共享库信息。|
| -m | 可选参数，查询指定应用指定模块依赖的共享库信息。|

示例：
```Bash
# 查询指定应用指定模块依赖的共享库信息
bm dump-dependencies -n com.ohos.app -m entry
```


#### 应用执行编译AOT命令（compile）

```bash
bm compile [-h] [-m mode] [-r bundleName] [-a]
```
**compile命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -a | 可选参数，编译所有应用。|
| -m | 可选参数，可选值为partial或者full。根据包名编译应用。|
| -r | 可选参数，移除应用的结果。|

示例：

```bash
# 根据包名编译应用
bm compile -m partial com.example.myapplication
```

#### 拷贝ap文件命令（copy-ap）

拷贝ap文件到指定应用的/data/local/pgo路径。

```bash
bm copy-ap [-h] [-a] [-n bundleName]
```

**copy-ap命令参数列表**

| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -a | 可选参数，默认所有包相关ap文件。拷贝所有包相关ap文件。|
| -n | 可选参数，默认当前应用包名。根据包名拷贝对应包相关的ap文件。|

示例：

```bash
# 根据包名移动对应包相关的ap文件
bm copy-ap -n com.example.myapplication
```

#### 查询overlay应用信息命令（dump-overlay）

```bash
bm dump-overlay [-h] [-b bundleName] [-m moduleName] [-t targetModuleName] [-u userId]
```

**dump-overlay命令参数列表**
| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -b | 必选参数，获取指定Overlay应用的所有OverlayModuleInfo信息。|
| -m | 可选参数，根据指定Overlay特征module的名称查询OverlayModuleInfo信息，默认当前Overlay应用主模块名。|
| -t | 可选参数，根据指定目标module的名称查询OverlayModuleInfo信息，默认参数为空。|
| -u | 可选参数，在指定[用户](#userid)下查询OverlayModuleInfo信息，默认在当前活跃用户下查询。仅支持在当前活跃用户或0用户下查询。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm dump-overlay -b com.ohos.app -u 102`查询OverlayModuleInfo信息，只会返回当前活跃用户100下的OverlayModuleInfo信息。 |

示例：

```bash
# 根据包名来获取overlay应用com.ohos.app中的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app

# 在用户100下，根据包名来获取overlay应用com.ohos.app中的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app -u 100

# 根据包名和module来获取overlay应用com.ohos.app中overlay module为libraryModuleName的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app -m libraryModuleName

# 根据目标包名和module来获取overlay应用com.ohos.app中目标module为entryModuleName的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app -t entryModuleName
```

#### 查询应用的overlay相关信息命令（dump-target-overlay）

查询目标应用的所有关联overlay应用的overlayModuleInfo信息。

```bash
bm dump-target-overlay [-h] [-b bundleName] [-m moduleName] [-u userId]
```

**dump-target-overlay命令参数列表**
| 参数 | 参数说明 |
| -------- | -------- |
| -h | 帮助信息。 |
| -b | 必选参数，获取指定应用的所有OverlayBundleInfo信息。|
| -m | 可选参数，默认当前应用主模块名。根据指定的包名和module名查询OverlayModuleInfo信息。|
| -u | 可选参数，在指定[用户](#userid)下查询OverlayModuleInfo信息，默认在当前活跃用户下查询。仅支持在当前活跃用户或0用户下查询。<br>**说明：**<br> 如果当前活跃用户是100，使用命令`bm dump-target-overlay -b com.ohos.app -u 102`查询目标应用com.ohos.app中的所有关联的OverlayBundleInfo信息，只会返回当前活跃用户100下的OverlayModuleInfo信息。 |

示例：

```bash
# 根据包名来获取目标应用com.ohos.app中的所有关联的OverlayBundleInfo信息
bm dump-target-overlay -b com.ohos.app

# 在用户100下，根据包名来获取目标应用com.ohos.app中的所有关联的OverlayBundleInfo信息
bm dump-target-overlay -b com.ohos.app -u 100

# 根据包名和module来获取目标应用com.ohos.app中目标module为entry的所有关联的OverlayModuleInfo信息
bm dump-target-overlay -b com.ohos.app -m entry
```

## 相关仓

[bundlemanager_bundle_framework](https://gitee.com/openharmony/bundlemanager_bundle_framework)
