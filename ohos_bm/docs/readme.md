# ohos-bm

ohos-bm is a command-line tool for installing and uninstalling applications on OpenHarmony devices.

## Usage

### Install an application

```bash
ohos-bm install -p <file-path>
```

#### Install options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-p, --bundle-path <file-path>` | Install a hap/hsp/app by specified path. Multiple paths can be specified. |
| `-r, --replace` | Replace an existing bundle (default behavior) |
| `-s, --shared-bundle-dir-path <dir-path>` | Install inter-application hsp files |
| `-u, --user-id <user-id>` | Specify a user id (only supports current user or userId is 0) |
| `-w, --waitting-time <seconds>` | Set waiting time for installation (180s ~ 600s) |
| `-d, --downgrade` | Allow downgrade installation |
| `-g, --grant-permission` | Grant permissions during installation |

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

### Uninstall an application

```bash
ohos-bm uninstall -n <bundle-name>
```

#### Uninstall options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help information |
| `-n, --bundle-name <bundle-name>` | Specify the bundle name to uninstall |
| `-m, --module-name <module-name>` | Uninstall a specific module by module name |
| `-u, --user-id <user-id>` | Specify a user id (only supports current user or userId is 0) |
| `-k, --keep-data` | Keep user data after uninstall |
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

### Help

```bash
ohos-bm help
```

## Architecture

```
ohos_bm/
  include/             # Header files
    bundle_command.h          # Command handler (install/uninstall)
    bundle_command_common.h   # Common utilities (proxy, error messages)
    shell_command.h           # Base class for CLI command parsing
    status_receiver_impl.h    # Async status receiver
  src/                 # Source files
    main.cpp                  # Entry point
    bundle_command.cpp        # Install/uninstall implementation
    bundle_command_common.cpp # Proxy & error message map
    shell_command.cpp         # Command parsing & dispatch
    status_receiver_impl.cpp  # Async result handling
  test/                # Test code
  docs/                # Documentation
    readme.md
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
