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

  **表1** bm工具命令列表

| 命令 | 描述 |
| -------- | -------- |
| help | 帮助命令，显示bm支持的命令信息。 |
| install | 安装命令，用来安装应用。 |
| uninstall | 卸载命令，用来卸载应用。 |
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


#### 帮助命令
```bash
bm help
```

  **表2** help命令列表

| 命令    | 描述       |
| ------- | ---------- |
| bm help | 显示bm工具的能够支持的命令信息。 |

示例：


```bash
# 显示帮助信息
bm help
```


#### 安装命令

```bash
bm install [-h] [-p filePath] [-u userId] [-r] [-w waitingTime] [-s hspDirPath]
```
安装命令可以组合，下面列出部分命令。


  **表3** 安装命令列表

| 命令                                | 描述                       |
| ----------------------------------- | -------------------------- |
| bm install -h | 显示install支持的命令信息。-h为非必选字段。 |
| bm install -p \<filePath\>    | 安装应用，支持指定路径和多个hap、hsp同时安装。安装应用时，-p为必选字段。 |
| bm install -p \<filePath\> -u \<userId\>   |给指定用户安装一个应用。-u非必选字段，默认为当前活跃用户。 |
| bm install -p \<filePath\> -r | 覆盖安装一个应用，-r为非必选字段，默认支持覆盖安装。 |
| bm install -p \<filePath\> -r -u \<userId\> | 给指定用户覆盖安装一个应用。 |
| bm install -p \<filePath\> -r -u \<userId\> -w \<waitingTime\> | 安装时指定bm工具等待时间，-w非必选字段，最小的等待时长为180s，最大的等待时长为600s，默认缺省为5s。 |
| bm install -s \<hspDirPath\> | 安装应用间共享库， 每个路径目录下只能存在一个同包名的HSP。-s为安装应用间HSP时为必选字段，其他场景为可选字段。 |
| bm install -p \<filePath\> -s \<hspDirPath\> | 同时安装使用方应用和其依赖的应用间共享库。 |



示例：
```bash
# 安装一个hap
bm install -p /data/app/ohos.app.hap
# 覆盖安装一个hap
bm install -p /data/app/ohos.app.hap -r
# 安装一个应用间共享库
bm install -s xxx.hsp
# 同时安装多个应用间共享库
bm install -s xxx.hsp yyy.hsp
# 同时安装使用方应用和其依赖的应用间共享库
bm install -p aaa.hap -s xxx.hsp yyy.hsp
```

#### 卸载命令

```bash
bm uninstall [-h] [-n bundleName] [-m moduleName] [-u userId] [-k] [-s] [-v versionCode]
```

命令可以组合，下面列出部分命令。


  **表4** 卸载命令列表

| 命令                          | 描述                     |
| ----------------------------- | ------------------------ |
| bm uninstall -h | 显示uninstall支持的命令信息。-h为非必选字段。 |
| bm uninstall -n \<bundleName\> | 通过指定包名卸载应用。-n为必选字段。 |
| bm uninstall -n \<bundleName\> -u \<userId\>| 通过指定包名和用户卸载应用。-u非必选字段，默认为当前活跃用户。 |
| bm uninstall -n \<bundleName\> -u \<userId\> -k| 通过指定包名和用户以保留用户数据方式卸载应用。-k为非必选字段。 |
| bm uninstall -n \<bundleName\> -m \<moduleName\> | 通过指定包名卸载应用的一个模块。-m为非必选字段。 |
| bm uninstall -n \<bundleName\> -s | 卸载指定的shared bundle。-s为非必选字段，卸载共享库应用时为必选字段。 |
| bm uninstall -n \<bundleName\> -s -v \<versionCode\> | 卸载指定的shared bundle的指定版本。-v为非必选字段。 |

示例：

```bash
# 卸载一个应用
bm uninstall -n com.ohos.app
# 卸载应用的一个模块
bm uninstall -n com.ohos.app -m entry
# 卸载一个shared bundle
bm uninstall -n com.ohos.example -s
# 卸载一个shared bundle的指定版本
bm uninstall -n com.ohos.example -s -v 100001
# 卸载一个应用，并保留用户数据
bm uninstall -n com.ohos.app -k
```


#### 查询应用信息命令

```bash
bm dump [-h] [-a] [-g] [-n bundleName] [-s shortcutInfo] [-u userId] [-d deviceId]
```
命令可以组合，下面列出部分命令。


  **表5** 查询命令列表

| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm dump -h | 显示dump支持的命令信息。-h为非必选字段。 |
| bm dump -a | 查询系统已经安装的所有应用包名。-a为非必选字段。 |
| bm dump -g | 查询系统中签名为调试类型的应用包名。-g为非必选字段。 |
| bm dump -n \<bundleName\> | 查询指定包名的详细信息。-n为非必选字段。 |
| bm dump -n \<bundleName\> -s | 查询指定包名下的快捷方式信息。-s为非必选字段。 |
| bm dump -n \<bundleName\> -d \<deviceId\> | 跨设备查询包信息。-d为非必选字段。 |
| bm dump -n \<bundleName\> -u \<userId\> | 查询指定用户下指定包名的详细信息。-u为非必选字段，默认为所有用户。 |


示例：

```bash
# 显示所有已安装的Bundle名称
bm dump -a
# 查询系统中签名为调试类型的应用包名
bm dump -g
# 查询该应用的详细信息
bm dump -n com.ohos.app -u 100
# 查询该应用的快捷方式信息
bm dump -s -n com.ohos.app -u 100
# 查询跨设备应用信息
bm dump -n com.ohos.app -d xxxxx
```

#### 清理命令

```bash
bm clean [-h] [-c] [-n bundleName] [-d] [-u userId] [-i appIndex]
```

  **表6** 清理命令列表
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm clean -h | 显示clean支持的命令信息。-h为非必选字段。 |
| bm clean -n \<bundleName\> -c | 清除指定包名的缓存数据。-n为必选字段，-c为非必选字段。 |
| bm clean -n \<bundleName\> -d | 清除指定包名的数据目录。-d为非必选字段。 |
| bm clean -n \<bundleName\> -c -u \<userId\> | 清除指定用户下包名的缓存数据。-u为非必选字段，默认为当前活跃用户。 |
| bm clean -n \<bundleName\> -d -u \<userId\> | 清除指定用户下包名的数据目录。 |
| bm clean -n \<bundleName\> -d -u \<userId\> -i \<appIndex\> | 清除指定用户下分身应用的数据目录。-i为非必选字段，默认为0。 |

示例：

```bash
# 清理该应用下的缓存数据
bm clean -c -n com.ohos.app -u 100
# 清理该应用下的用户数据
bm clean -d -n com.ohos.app -u 100
// 执行结果
clean bundle data files successfully.
```


#### 使能命令

```bash
bm enable [-h] [-n bundleName] [-a abilityName] [-u userId]
```


  **表7** 使能命令列表

| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm enable -h | 显示enable支持的命令信息。-h为非必选字段。 |
| bm enable -n \<bundleName\> | 使能指定包名的应用。-n为必选字段。 |
| bm enable -n \<bundleName\> -a \<abilityName\> | 使能指定包名下的元能力模块。-a为非必选字段。 |
| bm enable -n \<bundleName\> -u \<userId\>| 使能指定用户和包名的应用。-u为非必选字段，默认为当前活跃用户。 |


示例：

```bash
# 使能该应用
bm enable -n com.ohos.app -a com.ohos.app.EntryAbility -u 100
// 执行结果
enable bundle successfully.
```


#### 禁用命令

```bash
bm disable [-h] [-n bundleName] [-a abilityName] [-u userId]
```


  **表8** 禁用命令列表

| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm disable -h | 显示disable支持的命令信息。-h为非必选字段。 |
| bm disable -n \<bundleName\> | 禁用指定包名的应用。-n为必选字段。 |
| bm disable -n \<bundleName\> -a \<abilityName\> | 禁用指定包名下的元能力模块。-a为非必选字段。 |
| bm disable -n \<bundleName\> -u \<userId\>| 禁用指定用户和包名下的应用。-u为非必选字段，默认为当前活跃用户。 |


示例：

```bash
# 禁用该应用
bm disable -n com.ohos.app -a com.ohos.app.EntryAbility -u 100
// 执行结果
disable bundle successfully.
```


#### 获取udid命令

```bash
bm get [-h] [-u]
```

  **表9** 获取udid命令列表

| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm get -h | 显示get支持的命令信息。-h为非必选字段。 |
| bm get -u | 获取设备的udid。-u为必选字段。 |


示例：

```bash
# 获取设备的udid
bm get -u
// 执行结果
udid of current device is :
23CADE0C
```


#### 快速修复命令

```bash
bm quickfix [-h] [-a -f filePath [-t targetPath] [-d]] [-q -b bundleName] [-r -b bundleName] 
```

注：hqf文件制作方式可参考[HQF打包指令](packing-tool.md#hqf打包指令)。

  **表10** 快速修复命令列表
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm quickfix -h | 显示quickfix支持的命令信息。-h为非必选字段。 |
| bm quickfix -a -f \<filePath\> | 执行补丁安装命令。-a非必选字段，指定后，-f为必选字段，未指定-a，则-f为非必选字段。 |
| bm quickfix -q -b \<bundleName\> | 根据包名查询补丁包信息。-q为非必选字段，指定后，-b为必选字段，未指定-q，则-b为非必选字段。 |
| bm quickfix -a -f \<filePath\> -d | 选择debug模式执行补丁安装命令。-d为非必选字段。 |
| bm quickfix -a -f \<filePath\> -o | 选择覆盖模式执行补丁安装命令。-o为非必选字段。 |
| bm quickfix -a -f \<filePath\> -t \<target-path\> | 指定补丁安装目录，且不使能。-t为非必选字段。 |
| bm quickfix -r -b \<bundleName\> | 根据包名卸载未使能的补丁。-r为非必选字段，指定后，-b为必选字段，未指定-r，则-b为非必选字段。 |


示例：

```bash
# 根据包名查询补丁包信息
bm quickfix -q -b com.ohos.app
// 执行结果
// Information as follows:            
// ApplicationQuickFixInfo:           
//  bundle name: com.ohos.app 
//  bundle version code: xxx     
//  bundle version name: xxx       
//  patch version code: x            
//  patch version name:              
//  cpu abi:                          
//  native library path:             
//  type:                            
# 快速修复补丁安装
bm quickfix -a -f /data/app/
// 执行结果
apply quickfix succeed.
# 快速修复补丁卸载
bm quickfix -r -b com.ohos.app
// 执行结果
delete quick fix successfully
```

#### 共享库查询命令

```bash
bm dump-shared [-h] [-a] [-n bundleName] [-m moduleName]
```

  **表11** 共享库查询命令列表

| 命令                                             | 描述                                   |
| ------------------------------------------------ | -------------------------------------- |
| bm dump-shared -h  | 显示dump-shared支持的命令信息。-h为非必选字段。          |
| bm dump-shared -a          | 查询系统中已安装所有共享库。-a为非必选字段。     |
| bm dump-shared -n \<bundleName\>            | 查询指定共享库包名的详细信息。-n为非必选字段。           |
| bm dump-shared -n \<bundleName\> -m \<moduleName\>      | 查询指定共享库包名和模块名的详细信息。-m为非必选字段。     |


示例：

```bash
# 显示所有已安装共享库包名
bm dump-shared -a
# 显示该共享库的详细信息
bm dump-shared -n com.ohos.lib
```

#### 共享库依赖关系查询命令

显示指定应用和指定模块依赖的共享库信息
```bash
bm dump-dependencies [-h] [-n bundleName] [-m moduleName]
```

  **表12** 共享库依赖关系查询命令列表
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm dump-dependencies -h | 显示bm dump-dependencies支持的命令信息。-h为非必选字段。 |
| bm dump-dependencies -n \<bundleName\> | 查询指定应用依赖的共享库信息。-n为必选字段。 |
| bm dump-dependencies -n \<bundleName\> -m \<moduleName\> | 查询指定应用指定模块依赖的共享库信息。-m为非必选字段。 |

* 示例
```Bash
# 显示指定应用指定模块依赖的共享库信息
bm dump-dependencies -n com.ohos.app -m entry
```


#### 应用执行编译AOT命令

应用执行编译AOT命令
```bash
bm compile [-h] [-m mode] [-r bundleName]
```
  **表13** compile命令列表

| 命令 | 描述 |
| -------- | -------- |
| bm compile -h| 显示compile支持的命令信息。-h为非必选字段。 |
| bm compile -m \<mode-name\>| 根据包名编译应用。-m为非必选字段，可选值为partial或者full。 |
| bm compile -m \<mode-name\> -a| 编译所有应用。-a为非必选字段。 |
| bm compile -r -a| 移除所有编译应用的结果。-r为非必选字段。 |
| bm compile -r \<bundleName\>| 移除应用的结果。 |

示例：

```bash
# 根据包名编译应用
bm compile -m partial com.example.myapplication
```

#### 拷贝ap文件命令

拷贝ap文件到指定应用的/data/local/pgo路径

```bash
bm copy-ap [-h] [-a] [-n bundleName]
```

**表14** copy-ap命令列表

| 命令 | 描述 |
| -------- | -------- |
| bm copy-ap -h| 显示copy-ap支持的命令信息。-h为非必选字段。 |
| bm copy-ap -a| 拷贝所有包相关ap文件。-a为非必选字段。 |
| bm copy-ap -n \<bundleName\>| 根据包名拷贝对应包相关的ap文件。-n为非必选字段。 |

示例：

```bash
# 根据包名移动对应包相关的ap文件
bm copy-ap -n com.example.myapplication
```

#### 查询overlay应用信息命令

打印overlay应用的overlayModuleInfo
```bash
bm dump-overlay [-h] [-b bundleName] [-m moduleName] [-u userId] [-t targetModuleName]
```

**表15** dump-overlay命令列表
| 命令 | 描述 |
| -------- | -------- |
| bm dump-overlay -h| 显示dump-overlay支持的命令信息。-h为非必选字段。 |
| bm dump-overlay -b \<bundleName\>| 获取指定应用的所有OverlayModuleInfo信息。-b为必选字段。 |
| bm dump-overlay -b \<bundleName\> -m \<moduleName\>| 根据指定的包名和module名查询OverlayModuleInfo信息。-m为非必选字段。 |
| bm dump-overlay -b \<bundleName\> -t \<target-moduleName\>| 根据指定的包名和目标module名查询OverlayModuleInfo信息。-t为非必选字段。 |
| bm dump-overlay -b \<bundleName\> -t \<target-moduleName\> -u \<userId\>| 根据指定的包名\目标module名和用户查询OverlayModuleInfo信息。-u为非必选字段，默认为当前活跃用户。 |

示例：

```bash
# 根据包名来获取overlay应用com.ohos.app中的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app

# 根据包名和module来获取overlay应用com.ohos.app中overlay module为entry的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app -m entry

# 根据包名和module来获取overlay应用com.ohos.app中目标module为feature的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app -m feature
```

#### 查询应用的overlay相关信息命令

查询目标应用的所有关联overlay应用的overlayModuleInfo信息。

```bash
bm dump-target-overlay [-h] [-b bundleName] [-m moduleName] [-u userId]
```

**表16** dump-overlay命令列表
| 命令 | 描述 |
| -------- | -------- |
| bm dump-target-overlay -h| 显示dump-target-overlay支持的命令信息。-h为非必选字段。 |
| bm dump-target-overlay -b \<bundleName\> | 获取指定目标应用的所有OverlayBundleInfo信息。-b为必选字段。 |
| bm dump-target-overlay -b \<bundleName\> -m \<moduleName\> | 根据指定的目标应用的包名和module名查询OverlayModuleInfo信息。-m为非必选字段。 |
| bm dump-target-overlay -b \<bundleName\> -m \<moduleName\> -u \<userId\> | 根据指定的目标应用的包名、module名和用户查询OverlayModuleInfo信息。-u为非必选字段，默认为当前活跃用户。 |

示例：

```bash
# 根据包名来获取目标应用com.ohos.app中的所有关联的OverlayBundleInfo信息
bm dump-target-overlay-b com.ohos.app

# 根据包名和module来获取目标应用com.ohos.app中目标module为entry的所有关联的OverlayModuleInfo信息
bm dump-target-overlay -b com.ohos.app -m entry
```
## 相关仓

[bundlemanager_bundle_framework](https://gitee.com/openharmony/bundlemanager_bundle_framework)
