# OTP Module Manager

## Overview

This project implements an **HMAC (Hash-based Message Authentication Code)** algorithm using the **Linux Kernel Crypto API**.

It demonstrates how to:

- Implement a **kernel module**
- Register a **device** for user-space communication
- Use **procfs** for user-space interaction
- Use the **Crypto API** for cryptographic operations

This project is designed for **kernel module development** and should be compiled within a Linux kernel environment.

## Features

- Uses **HMAC-SHA1** to compute message authentication codes
- ...

## Installation & Usage

### Prerequisites

Ensure you have:

- A **Linux system** with kernel headers installed
- `make` and `gcc` for building kernel modules
- Root privileges for loading/unloading kernel modules

### Makefile script

The Makefile contains the following targets:

- `all` - Builds the kernel module into an `.ko` file
- `load` - Builds and loads the kernel module
- `unload` - Unloads the kernel module
- `clean` - Cleans the build directory
- `info` - Displays information about the kernel module
- `log` - Displays the kernel log
- `logx` - Displays the kernel log continuously
- `help` - Displays the Makefile help

### Procedure

1. Clone the repository:

2. Run the `make load` command to build and load the kernel module:

```bash
$ make load
```

3. Check the kernel log for module information:

```bash
$ make log
```

4. Use the `procfs` interface to interact with the module:

```bash
$ echo "Hello, World!" > /proc/otp
$ cat /proc/otp
```

## File Structure

```
.
├── crypto.c        # Main implementation of HMAC using Crypto API
├── Makefile        # Build script for compiling the module
├── module.c        # Kernel module initialization and cleanup
├── module.d/       # Directory for kernel module source files
│   └──  *          # Object files generated during compilation
├── ...             # ...
├── ...             # ...
└── README.md       # User Documentation
```

## Authors

**Fazanwolf** - [GitHub](https://github.com/Fazanwolf)

**Salocin** - [GitHub](https://github.com/...)

**...** - [GitHub](https://github.com/...)

## License

This project is licensed under the **GPLv3** License - see the [LICENSE](LICENSE) file for details.

## References

- [Linux Kernel Crypto API Documentation](https://www.kernel.org/doc/html/latest/crypto/index.html)

