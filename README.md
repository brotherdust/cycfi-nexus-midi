# Nexus GK MIDI Controller Firmware - Modular Refactoring

A modular refactoring of Cycfi Research's Nexus GK MIDI controller for MSP430G2553, enhanced with switch-to-CC mapping functionality.

## Attribution

Based on the original [Cycfi Research Nexus GK](https://www.cycfi.com/) codebase. Original source: [https://github.com/cycfi/nexus/tree/master/source/nexus_midi](https://github.com/cycfi/nexus/tree/master/source/nexus_midi)

## Overview

This refactoring transforms the original monolithic Nexus GK firmware into a modular architecture while adding switch-to-CC mapping capabilities. Built with PlatformIO for MSP430G2553 (16KB Flash, 512B RAM).

### Key Enhancement: Switch-to-CC Mapping
- One-hot encoding: only one CC number active per switch position
- Configurable CC range mapping
- Real-time CC transmission on switch changes

## Features

### Preserved from Original
- Multiple MIDI controllers (Program Change, Bank Select, Pitch Bend, Sustain, Volume, Modulation, FX)
- Persistent flash storage
- Hardware abstraction
- Memory-optimized design

### Refactoring Improvements
- **Modular Architecture**: Separated controllers, storage, and configuration modules
- **Factory Pattern**: Dynamic controller instantiation
- **Namespace Organization**: Structured `nexus::` hierarchy
- **Test Framework**: Built-in validation
- **PlatformIO Migration**: Replaced Energia IDE with modern toolchain for better dependency management and debugging

## Technical Stack

- **MCU**: MSP430G2553
- **Platform**: PlatformIO with energia framework
- **Language**: C++98/03 (MSP430 compatibility)
- **IDE**: VSCode with PlatformIO extension

## Quick Start

### Prerequisites
1. Install [VSCode](https://code.visualstudio.com/)
2. Install [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
3. Install MSP430 LaunchPad drivers

**NOTE**: The `mspdebug` tool included with PlatformIO is out of date and doesn't include the `tilib` driver. Ensure you are using at least version `0.25` AND has support for `tilib`. How to do that is outside of the scope of this project.

### Build & Upload
```bash
git clone https://github.com/brotherdust/cycfi-nexus-midi.git
cd cycfi-nexus-midi
code .
```

Build: `Ctrl+Alt+B` (Windows/Linux) or `Cmd+Alt+B` (macOS)  
Upload: Connect LaunchPad, then `Ctrl+Alt+U` or `Cmd+Alt+U`

## Memory Optimization

- C++98/03 only (no C++11 features)
- No virtual functions (minimize vtables)
- No dynamic allocation
- Monitor with `msp430-size`

## Contributing

Guidelines:
- Maintain C++98/03 compatibility
- Follow `nexus::` namespace structure
- Minimize memory usage
- Add tests for new features
- Use conventional commits (`feat:`, `fix:`, `refactor:`, `docs:`, `test:`)

## License

MIT License - See LICENSE.md

## Support

- Open issues in the repository
- Check `/docs` directory
- Review test cases for examples