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
| 命令    | 描述       |
| ------- | ---------- |
|  help | 帮助命令，显示bm支持的命令信息 |
| install | 安装命令，用来安装应用|
| uninstall | 卸载命令，用来卸载应用|
| dump | 查询命令，用来查询应用的相关信息|
| clean | 清理命令，用来清理应用的缓存和数据 |
| enable | 使能命令，用来使能应用 |
| disable | 禁用命令，用来禁用应用 |
| get | 获取udid命令，用来获取设备的udid |
| quickfix | 快速修复相关命令，用来执行补丁相关操作，如补丁安装、补丁查询 |
| compile | 应用执行编译AOT命令 |
| copy-ap | 把应用的ap文件拷贝到/data/local/pgo目录下，供shell用户读取文件 |
| dump-dependencies | 查询应用依赖的模块信息 |
| dump-shared | 查询应用间HSP应用信息 |
| dump-overlay | 打印overlay应用的的overlayModuleInfo |
| dump-target-overlay | 打印目标应用的所有关联overlay应用的overlayModuleInfo |
#### 帮助命令
| 命令    | 描述       |
| ------- | ---------- |
| bm help | 显示bm工具的能够支持的命令信息 |

* 示例
```Bash
# 显示帮助信息
bm help
```
#### 安装命令
命令可以组合，下面列出部分命令。
| 命令                                | 描述                       |
| ----------------------------------- | -------------------------- |
| bm install -h | 显示install支持的命令信息 |
| bm install -p <file-path>    | 安装应用，支持指定路径和多个hap、hsp同时安装 |
| bm install -p <file-path> -u <user-id>   |给指定用户安装一个应用 |
| bm install -r -p <file-path> | 覆盖安装一个应用 |
| bm install -r -p <file-path> -u <user-id> | 给指定用户覆盖安装一个应用 |
| bm install -r -p <file-path> -u <user-id> -w <waitting-time> | 安装时指定bm工具等待时间，最小的等待时长为180s，最大的等待时长为600s, 默认缺省为5s |
| bm install -s <hsp-dir-path> | 安装应用间共享库， 每个路径目录下只能存在一个hsp|
| bm install -p <file-path> -s <hsp-dir-path> | 同时安装使用方应用和其依赖的应用间共享库 |

* 示例
```Bash
# 安装一个hap
bm install -p /data/app/ohosapp.hap
# 覆盖安装一个hap
bm install -p /data/app/ohosapp.hap -r
# 安装一个应用间共享库
bm install -s xxx.hsp
# 同时安装多个应用间共享库
bm install -s xxx.hsp yyy.hsp
# 同时安装使用方应用和其依赖的应用间共享库
bm install -p aaa.hap -s xxx.hsp yyy.hsp
```
#### 卸载命令
命令可以组合，下面列出部分命令。-u未指定情况下，默认为所有用户。
| 命令                          | 描述                     |
| ----------------------------- | ------------------------ |
| bm uninstall -h | 显示uninstall支持的命令信息 |
| bm uninstall -n <bundle-name> | 通过指定包名卸载应用 |
| bm uninstall -n <bundle-name> -u <user-id>| 通过指定包名和用户卸载应用 |
| bm uninstall -n <bundle-name> -m <moudle-name> | 通过指定包名卸载应用的一个模块 |
| bm uninstall -s <hsp-dir-path> -n <bundle-name> | 卸载指定的shared bundle |
| bm uninstall -s  <hsp-dir-path> -n <bundle-name> -v <version-code> | 卸载指定的shared bundle的指定版本 |

* 示例
```Bash
# 卸载一个应用
bm uninstall -n com.ohos.app
# 卸载应用的一个模块
bm uninstall -n com.ohos.app -m com.ohos.app.MainAbility
# 卸载一个shared bundle
bm uninstall -s -n com.ohos.example
# 卸载一个shared bundle的指定版本
bm uninstall -s -n com.ohos.example -v 100001
```
#### 查询命令
命令可以组合，下面列出部分命令。-u未指定情况下，默认为所有用户。
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm dump -h | 显示dump支持的命令信息 |
| bm dump -a | 查询系统已经安装的所有应用 |
| bm dump -n <bundle-name> | 查询指定包名的详细信息 |
| bm dump -n <bundle-name> -s | 查询指定包名下的快捷方式信息 |
| bm dump -n <bundle-name> -d <device-id> | 跨设备查询包信息 |
| bm dump -n <bundle-name> -u <user-id> | 查询指定用户下指定包名的详细信息 |
| bm dump-shared -h | 显示dump-shared支持的命令信息 |
| bm dump-shared -a | 查询系统中已安装所有共享库 |
| bm dump-shared -n  <bundle-name> | 查询指定共享库包名的详细信息 |
| bm dump-dependencies -h | 显示bm dump-dependencies支持的命令信息 |
| bm dump-dependencies -n <bundle-name> -m <moudle-name> | 查询指定应用指定模块依赖的共享库信息 |

* 示例
```Bash
# 显示所有已安装的包名
bm dump -a
# 显示该应用的详细信息
bm dump -n com.ohos.app
# 显示所有已安装共享库包名
bm dump-shared -a
# 显示该共享库的详细信息
bm dump-shared -n com.ohos.lib
# 显示指定应用指定模块依赖的共享库信息
bm dump-dependencies -n com.ohos.app -m entry
```
#### 清理命令
-u未指定情况下，默认为当前活跃用户。
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm clean -h | 显示clean支持的命令信息 |
| bm clean -n <bundle-name> -c | 清除指定包名的缓存数据 |
| bm clean -n <bundle-name> -d | 清除指定包名的数据目录 |
| bm clean -n <bundle-name> -c -u <user-id> | 清除指定用户下包名的缓存数据 |
| bm clean -n <bundle-name> -d -u <user-id> | 清除指定用户下包名的数据目录 |

* 示例
```Bash
# 清理该应用下的缓存数据
bm clean -n com.ohos.app -c
# 清理该应用下的用户数据
bm clean -n com.ohos.app -d
```
#### 使能命令
-u未指定情况下，默认为当前活跃用户。
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm enable -h | 显示enable支持的命令信息 |
| bm enable -n <bundle-name> | 使能指定包名的应用 |
| bm enable -n <bundle-name> -a <ability-name> | 使能指定包名下的元能力模块 |
| bm enable -n <bundle-name> -u <user-id>| 使能指定用户和包名的应用 |

* 示例
```Bash
# 使能该应用
bm enable -n com.ohos.app
```
#### 禁用命令
-u未指定情况下，默认为当前活跃用户。
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm disable -h | 显示disable支持的命令信息 |
| bm disable -n <bundle-name> | 禁用指定包名的应用 |
| bm disable -n <bundle-name> -a <ability-name> | 禁用指定包名下的元能力模块 |
| bm disable -n <bundle-name> -u <user-id>| 禁用指定用户和包名下的应用 |

* 示例
```Bash
# 禁用该应用
bm disable -n com.ohos.app
```
#### 获取udid命令
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm get -h | 显示get支持的命令信息 |
| bm get -u | 获取设备的udid |

* 示例
```Bash
# 获取设备的udid
bm get -u
```

#### 快速修复命令
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm quickfix -h | 显示quickfix支持的命令信息 |
| bm quickfix -a -f <file-path> | 执行补丁安装命令 |
| bm quickfix -q -b <bundle-name> | 根据包名查询补丁包信息 |
| bm quickfix -a -f <file-path> -d | 选择debug模式执行补丁安装命令 |

* 示例
```Bash
# 根据包名查询补丁包信息
bm quickfix -q -b <bundle-name>
```

#### 应用编译AOT命令
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm compile -h | 显示compile支持的命令信息 |
| bm compile -m partial <bundle-name> | 根据包名编译应用 |
| bm compile -m partial -a | 编译所有应用 |
| bm compile -r -a | 移除所有编译应用的结果 |
| bm compile -r <bundle-name> | 移除应用的结果 |

* 示例
```Bash
# 根据包名编译应用
bm compile -m partial <bundle-name>
```

#### 拷贝ap文件到/data/local/pgo路径
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm copy-ap -h | 显示copy-ap支持的命令信息 |
| bm copy-ap -a | 拷贝所有包相关ap文件 |
| bm copy-ap -n <bundle-name> | 根据包名拷贝对应包相关的ap文件 |

* 示例
```Bash
# 根据包名移动对应包相关的ap文件
bm copy-ap -n <bundle-name>
```

#### 获取overlay应用的Overlay信息命令
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm dump-overlay -h | 显示dump-overlay支持的命令信息 |
| bm dump-overlay -b <bundle-name> | 获取指定应用的所有OverlayModuleInfo信息 |
| bm dump-overlay -b <bundle-name> -m <module-name> | 根据指定的包名和module名查询OverlayModuleInfo信息 |
| bm dump-overlay -b <bundle-name> -t <target-module-name> | 根据指定的包名和目标module名查询OverlayModuleInfo信息 |

* 示例
```Bash
* 示例
# 根据包名来获取overlay应用com.ohos.app中的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app

# 根据包名和module来获取overlay应用com.ohos.app中overlay module为entry的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app -m entry

# 根据包名和module来获取overlay应用com.ohos.app中目标module为feature的所有OverlayModuleInfo信息
bm dump-overlay -b com.ohos.app -m feature
```

#### 获取目标应用的Overlay信息命令
| 命令       | 描述                       |
| ---------- | -------------------------- |
| bm dump-target-overlay -h | 显示dump-target-overlay支持的命令信息 |
| bm dump-target-overlay -b <bundle-name> | 获取指定目标应用的所有OverlayBundleInfo信息 |
| bm dump-target-overlay -b <bundle-name> -m <module-name> | 根据指定的目标应用的包名和module名查询OverlayModuleInfo信息 |

* 示例
```Bash
* 示例
# 根据包名来获取目标应用com.ohos.app中的所有关联的OverlayBundleInfo信息
bm dump-target-overlay-b com.ohos.app

# 根据包名和module来获取目标应用com.ohos.app中目标module为entry的所有关联的OverlayModuleInfo信息
bm dump-target-overlay -b com.ohos.app -m entry

```

## 相关仓

[bundlemanager_bundle_framework](https://gitee.com/openharmony/bundlemanager_bundle_framework)
