[![License](https://img.shields.io/github/license/great-andy/AppleSMC.svg)](LICENSE)

# Apple System Management Control (SMC) Tool

A lightweight Command-Line Interface (CLI) for macOS to interact with the Apple System Management Controller.  
This tool allows users to monitor sensor data, inspect fan configurations, and read/write SMC keys.

## Features
- **Key Discovery:** List all available SMC keys with optional pattern matching.
- **Fan Diagnostics:** View decoded fan speeds and status information.
- **Low-level Access:** Read and write (hex) directly to specific SMC keys.
- **Minimalist:** Fast execution with no heavy dependencies.

## Technical Implementation
Built with **C++20**, this tool features an advanced interpretation engine for SMC data:
- **Automatic Type Detection:** Automatically fetches and parses SMC attributes to determine the correct data type (e.g., `sp78`, `fpe2`, `ui32`).
- **Endianness Handling:** Intelligent MSB/LSB (Most/Least Significant Bit) conversion based on the key's internal attributes.

## Installation
```bash
git clone https://github.com/great-andy/AppleSMC.git
cd AppleSMC
make
sudo make install
```

## Usage
```bash
smc [options]
```

### Available Options
| Option | Description |
|:---|:---|
| `-l`, `--list=['*\|?']` | List all keys and values. Supports optional pattern matching. |
| `-f`, `--fans` | List decoded fan information (RPM, status, etc.). |
| `-k`, `--key <key>` | Set the specific key to read from or write to. |
| `-r`, `--read=[key]` | Read the value of a key. Optional: provide key directly. |
| `-w`, `--write <value>`| Write a hex value to the specified key. |
| `-h`, `--help` | Print help message and exit. |
| `-v`, `--version` | Print version information. |

### Examples
**List all temperature sensors:**
```bash
smc --list='T*'
```

**Check fan status:**
```bash
smc -f
```

**Read a specific key:**
```bash
# Example: Read incoming power from AC adapter (in Milliwatts)
smc -rACPW
```

**Write a hex value to a key:**
```bash
# Example: Manually switch the MagSafe led off
sudo smc -k ACLC -w 1
```
```bash
# Example: Reset the MagSafe led to system state
sudo smc -k ACLC -w 0
```

### Important: Root Access
SMC writing operations require `sudo`.

### Deep Metadata Inspection
The tool doesn't just read values; it queries the internal SMC key information to ensure data integrity:  
`KEY  [TYPE|ATTRIBUTES]  VALUE  (RAW_HEX)`

Example:
```bash
$ smc -rACPW
ACPW  [ui32|95]  19980  (0c 4e 00 00)
```

- **Type:** Displays the internal data type (e.g., `ui32`, `fpe2`).
- **Attributes:** Reveals the internal SMC attribute flags, providing insight into key permissions and behavior.
- **Value:** Converts the raw SMC bytes into a human-readable format based on the detected data type and attributes.
- **Raw Hex:** Always prints the raw hex dump alongside the parsed value for manual verification.

## ⚠️ Warning
Writing values to the SMC via `--write` can be dangerous. The SMC manages critical hardware functions like thermal regulation and power management.
Incorrect values may lead to hardware instability or damage. Use this tool at your own risk.

## License
This project is licensed under the [MIT License](LICENSE) (or your preferred license).

## Disclaimer
This tool interacts with the system hardware via IOKit. Use at your own risk. The authors are not responsible for any hardware issues.
