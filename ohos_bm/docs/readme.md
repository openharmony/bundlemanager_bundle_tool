# ohos-bm

ohos-bm is a command-line tool for managing applications on OpenHarmony devices.

## Commands Overview

| Command | Description |
|---------|-------------|
| `help` | View information about ohos-bm subcommands |
| `install` | Install a hap, hsp or app package |
| `uninstall` | Uninstall an application package |
| `dump` | View application package information |
| `dump-dependencies` | View dependency information of specified application and module |
| `dump-shared` | View inter-application shared library information |
| `clean` | Clean application cache or data files |
| `set-disposed-rule` | Set disposed rule for clone app to control component behavior |
| `delete-disposed-rule` | Delete disposed rule for clone app |

## Usage

### help

View available commands.

```bash
ohos-bm help
```

### install

Install a hap, hsp or app package.

```bash
ohos-bm install -p <file-path>
```

#### Install options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-p, --bundlePath <file-path>` | Install a hap/hsp/app by specified path. Multiple paths can be specified. |
| `-r, --replace` | Replace an existing bundle (default behavior) |
| `-s, --sharedBundleDirPath <dir-path>` | Install inter-application hsp files |
| `-u, --userId <user-id>` | Specify a user id (only supports current user or userId is 0) |
| `-w, --waittingTime <seconds>` | Set waiting time for installation (180s ~ 600s) |
| `-d, --downgrade` | Allow downgrade installation |
| `-g, --grantPermission` | Grant permissions during installation |

#### Install examples

```bash
# Install a single hap
ohos-bm install -p /data/local/tmp/test.hap

# Install multiple haps of one bundle
ohos-bm install -p /data/local/tmp/entry.hap /data/local/tmp/feature.hap

# Install with replace
ohos-bm install -r -p /data/local/tmp/test.hap

# Install with shared hsp
ohos-bm install -p /data/local/tmp/test.hap -s /data/local/tmp/shared.hsp

# Install with downgrade allowed
ohos-bm install -p /data/local/tmp/test.hap -d

# Install with permissions granted
ohos-bm install -p /data/local/tmp/test.hap -g
```

### uninstall

Uninstall an application package.

```bash
ohos-bm uninstall -n <bundle-name>
```

#### Uninstall options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-n, --bundleName <bundle-name>` | Specify the bundle name to uninstall |
| `-m, --moduleName <module-name>` | Uninstall a specific module by module name |
| `-u, --userId <user-id>` | Specify a user id (only supports current user or userId is 0) |
| `-k, --keepData` | Keep user data after uninstall |
| `-s, --shared` | Uninstall inter-application shared library |
| `-v, --version <versionCode>` | Specify version code for shared library uninstall |

#### Uninstall examples

```bash
# Uninstall by bundle name
ohos-bm uninstall -n com.example.test

# Uninstall a specific module
ohos-bm uninstall -n com.example.test -m entry

# Uninstall keeping user data
ohos-bm uninstall -n com.example.test -k

# Uninstall shared library
ohos-bm uninstall -n com.example.sharedlib -s

# Uninstall shared library by version
ohos-bm uninstall -n com.example.sharedlib -s -v 1000000
```

### dump

View application package information.

```bash
ohos-bm dump -n <bundle-name>
```

#### Dump options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-a, --all` | List all bundles in system |
| `-g, --debugBundle` | List debug bundles in system |
| `-n, --bundleName <bundle-name>` | List the bundle info by a bundle name |
| `-s, --shortcutInfo` | List the shortcut info |
| `-d, --deviceId <device-id>` | Specify a device id |
| `-u, --userId <user-id>` | Specify a user id (only supports current user or userId is 0) |
| `-l, --label` | List the label info |

#### Dump examples

```bash
# List all bundles
ohos-bm dump -a

# Dump specific bundle info
ohos-bm dump -n com.example.test

# Dump shortcut info
ohos-bm dump -n com.example.test -s

# Dump debug bundles
ohos-bm dump -g

# Dump label info
ohos-bm dump -n com.example.test -l
```

### dump-dependencies

View dependency information of specified application and module.

```bash
ohos-bm dump-dependencies -n <bundle-name> -m <module-name>
```

#### Dump-dependencies options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-n, --bundleName <bundle-name>` | Specify bundle name |
| `-m, --moduleName <module-name>` | Specify module name |

#### Dump-dependencies examples

```bash
# Dump dependencies for specific module
ohos-bm dump-dependencies -n com.example.test -m entry
```

### dump-shared

View inter-application shared library information.

```bash
ohos-bm dump-shared -n <bundle-name>
```

#### Dump-shared options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-a, --all` | List all inter-application shared library names in system |
| `-n, --bundleName <bundle-name>` | Dump inter-application shared library information by bundle name |

#### Dump-shared examples

```bash
# List all shared libraries
ohos-bm dump-shared -a

# Dump specific shared library info
ohos-bm dump-shared -n com.example.sharedlib
```

### clean

Clean application cache or data files.

```bash
ohos-bm clean -n <bundle-name> -c
```

#### Clean options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-n, --bundleName <bundle-name>` | Specify bundle name |
| `-c, --cache` | Clean bundle cache files by bundle name |
| `-d, --data` | Clean bundle data files by bundle name |
| `-u, --userId <user-id>` | Specify a user id (only supports current user or userId is 0) |
| `-i, --appIndex <app-index>` | Specify a app index |

#### Clean examples

```bash
# Clean cache files
ohos-bm clean -n com.example.test -c

# Clean data files
ohos-bm clean -n com.example.test -d

# Clean cache for specific user
ohos-bm clean -n com.example.test -c -u 100
```

### set-disposed-rule

Set disposed rule for clone app to control component behavior.

```bash
ohos-bm set-disposed-rule --appId <app-id> --priority <priority> ...
```

#### Set-disposed-rule options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `--appId <app-id>` | Application appId or appIdentifier (required) |
| `--appIndex <app-index>` | Clone app index, a positive integer |
| `--priority <priority>` | Disposed rule priority, non-negative integer, higher value means higher priority (required) |
| `--componentType <type>` | Component type to control (required): 1=UI_ABILITY, 2=UI_EXTENSION |
| `--disposedType <type>` | Disposed type (required): 1=BLOCK_APPLICATION, 2=BLOCK_ABILITY, 3=NON_BLOCK |
| `--controlType <type>` | Control type (required): 1=ALLOWED_LIST, 2=DISALLOWED_LIST |
| `--elements <element-uri>` | Element to control, format: /bundleName/moduleName/abilityName. Can be repeated for multiple elements. |
| `--wantBundleName <name>` | Want redirection target bundleName (required) |
| `--wantModuleName <name>` | Want redirection target moduleName |
| `--wantAbilityName <name>` | Want redirection target abilityName (required) |
| `--wantParamsStrings <key> <value>` | Want string type additional parameters. Can be repeated. |
| `--wantParamsInts <key> <value>` | Want int type additional parameters. Can be repeated. |
| `--wantParamsBools <key> <true/false>` | Want bool type additional parameters. Can be repeated. |

#### Set-disposed-rule examples

```bash
# Block an application
ohos-bm set-disposed-rule --appId com.example.test \
    --priority 100 --componentType 1 --disposedType 1 \
    --controlType 1 --wantBundleName com.example.redirect \
    --wantAbilityName MainAbility

# Block specific abilities with allowed list
ohos-bm set-disposed-rule --appId com.example.test \
    --priority 100 --componentType 2 --disposedType 2 \
    --controlType 1 \
    --elements /com.example.test/entry/MainAbility \
    --elements /com.example.test/feature/SubAbility \
    --wantBundleName com.example.redirect \
    --wantModuleName entry \
    --wantAbilityName MainAbility
```

### delete-disposed-rule

Delete disposed rule for clone app.

```bash
ohos-bm delete-disposed-rule --appId <app-id>
```

#### Delete-disposed-rule options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `--appId <app-id>` | Application appId or appIdentifier (required) |
| `--appIndex <app-index>` | Clone app index, a positive integer |

#### Delete-disposed-rule examples

```bash
# Delete disposed rule by appId
ohos-bm delete-disposed-rule --appId com.example.test

# Delete disposed rule for specific clone app
ohos-bm delete-disposed-rule --appId com.example.test --appIndex 1001
```

## Architecture

```
ohos_bm/
  include/             # Header files
    bundle_command.h          # Command handler (install/uninstall)
    bundle_command_common.h   # Common utilities (proxy, error messages)
    error_code_utils.h        # Error code to string mapping utility
    shell_command.h           # Base class for CLI command parsing
    status_receiver_impl.h    # Async status receiver
  src/                 # Source files
    main.cpp                  # Entry point
    bundle_command.cpp        # Install/uninstall implementation
    bundle_command_common.cpp # Proxy & error message map
    error_code_utils.cpp      # Error code mapping implementation
    shell_command.cpp         # Command parsing & dispatch
    status_receiver_impl.cpp  # Async result handling
  test/                # Test code
  docs/                # Documentation
    readme.md
  config.json          # CLI configuration file
  BUILD.gn             # Build configuration
```

## Dependencies

- `bundle_framework:appexecfwk_base` - Base bundle framework types
- `bundle_framework:appexecfwk_core` - Core bundle framework interfaces
- `bundle_framework:bundle_tool_libs` - Bundle tool libraries
- `ipc:ipc_core` - IPC/Binder core
- `samgr:samgr_proxy` - System ability manager proxy
- `c_utils:utils` - C++ utility library
- `hilog:libhilog` - Logging
- `init:libbegetutil` - System parameter utilities
- `json:nlohmann_json_static` - JSON library

## Output Format

All commands return JSON output with the following structure:

```json
{
    "type": "result",
    "status": "success|failed",
    "data": {},
    "errCode": "SUCCESS|ERR_XXX",
    "errMsg": "Error message",
    "suggestion": "Suggestion for resolving the error"
}
```

## Permissions

| Command | Required Permission |
|---------|---------------------|
| `install` | `ohos.permission.cli.INSTALL_BUNDLE` |
| `uninstall` | `ohos.permission.cli.UNINSTALL_BUNDLE` |
| `dump` | `ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED` |
| `dump-dependencies` | `ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED` |
| `dump-shared` | `ohos.permission.cli.GET_BUNDLE_INFO_PRIVILEGED` |
| `clean` | `ohos.permission.cli.REMOVE_CACHE_FILES` |
| `set-disposed-rule` | `ohos.permission.cli.MANAGE_DISPOSED_APP_STATUS` |
| `delete-disposed-rule` | `ohos.permission.cli.MANAGE_DISPOSED_APP_STATUS` |