# unirtos-xlat-demos

[中文](README.zh.md) | English

This repository is recommended to be used via the unirtos-cli demo workflow to ensure consistent project creation, environment setup, and build processes.

## Feature Description

This demo demonstrates the basic development workflow for CLAT (Customer-side Translator) based on 464XLAT on UniRTOS. It is a good starting point for IPv4/IPv6 network translation and PING verification development.

- Demonstrates getting and printing the current CLAT switch state and full configuration parameters
- Demonstrates enabling the CLAT feature (setting `clat_switch` to 1)
- Demonstrates initiating a PING test to an IPv4 server (`8.8.8.8`) over a CLAT-enabled data connection (PDP)
- Demonstrates waiting for PING completion and printing result statistics (RTT, packet loss, etc.)
- Demonstrates reading the full CLAT configuration (DNS64 mode, destination prefix, FQDN, TOS, DF flag, etc.)
- Includes an asynchronous message-queue-driven PING callback handling framework

## Quick Start

### 1. Install the UniRTOS Toolchain

- [Development Preparation](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/开发准备/开发准备.html)
- [Install Cross-Compilation Toolchain](https://www.quectel.com.cn/unirtos/docs?docs_page=快速上手/环境搭建/环境搭建.html)
- [Install Python3](https://www.python.org/downloads/)
- [Install git](https://git-scm.com)
- Install unirtos-cli: `pip install unirtos-cli`

After installation, verify the following commands are available:

```bash
python --version       # Python3
git --version
unirtos --version      # 1.0.5 or later
unirtos-cli version    # 1.0.11 or later
```

### 2. Create the Demo Project with unirtos-cli

List available demos and versions:

```bash
unirtos-cli ls-demos
```

Create the demo project:

```bash
unirtos-cli new -r unirtos-xlat-demos
```

To specify a version:

```bash
unirtos-cli new -r unirtos-xlat-demos -v 1.0.0
```

### 3. Enter the Project and Build

```bash
cd unirtos-xlat-demos-1.0.0
unirtos-cli env-setup
unirtos-cli build
```

## Common Commands

```bash
# Open SDK menu configuration
unirtos-cli menuconfig

# Clean build artifacts
unirtos-cli clean
```

## Technical Community

Forum: https://forumschinese.quectel.com/c/66-category/66

## Contributing

Contributions are welcome. Please follow these guidelines:
- Run a basic validation before submitting: env-setup, build, clean.
- Use clear commit messages describing the purpose, scope of changes, and validation results.
- Update README and related documentation when adding features or changing behavior.
- Submit bug fixes and feature improvements via Issues or Pull Requests.
