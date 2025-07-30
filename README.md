# Nexus MIDI Controller - Modular Refactoring

A modular refactoring of the original Cycfi Research Nexus MIDI controller codebase for the MSP430G2553 microcontroller, enhanced with switch-to-CC mapping functionality.

## Attribution

**This project is based on the original Nexus codebase created by [Cycfi Research](https://www.cycfi.com/).** This repository represents a modular refactoring of their original implementation, not a generic MIDI controller solution.

## Description

This repository contains a modular refactoring of the Cycfi Research Nexus MIDI controller firmware. The refactoring transforms the original monolithic codebase into a clean, maintainable architecture while preserving the core functionality and adding new capabilities. Built using PlatformIO and optimized for the MSP430G2553's memory constraints, this implementation maintains compatibility with the original hardware design.

### Key Enhancement

The primary enhancement in this refactoring is the addition of **switch-to-CC mapping functionality** that enables:
- Mapping switch positions to a range of MIDI Control Change (CC) numbers
- One-hot encoding configuration where only one CC number is active at a time
- Dynamic CC assignment based on physical switch position
- Seamless integration with the existing Nexus architecture

### Purpose

This refactoring serves to:
- Modernize the original Cycfi Nexus codebase with a modular architecture
- Add switch-to-CC mapping capabilities not present in the original
- Provide a maintainable foundation for further Nexus-based development
- Demonstrate embedded refactoring techniques while respecting memory constraints

## Origin and History

The original Nexus MIDI controller was created by Cycfi Research as part of their innovative music technology solutions. This refactoring project:
- Preserves the core Nexus functionality and design philosophy
- Restructures the code into modular components for better maintainability
- Adds the switch-to-CC mapping feature as a key enhancement
- Maintains strict compatibility with the MSP430G2553's limited resources (16KB Flash, 512B RAM)

## Key Features

### Original Nexus Features (Preserved)
- Multiple MIDI controller types (Program Change, Bank Select, Pitch Bend, etc.)
- Persistent storage for configurations
- Hardware abstraction layer
- Memory-optimized design for MSP430G2553

### Refactoring Improvements
- **Modular Architecture**: Clean separation of concerns with dedicated modules for controllers, storage, and configuration
- **Factory Pattern**: Flexible controller instantiation system
- **Namespace Organization**: Structured `nexus::` namespace hierarchy
- **Test Framework**: Built-in testing capabilities for validation

### New Enhancement: Switch-to-CC Mapping
- **One-Hot Encoding**: Maps switch positions to individual CC numbers where only one is active at a time
- **Configurable CC Range**: Define which CC numbers correspond to switch positions
- **Real-time Switching**: Instant CC message transmission on switch state changes
- **Integration**: Seamlessly integrated with the existing controller architecture

## Technology Stack

- **Microcontroller**: Texas Instruments MSP430G2553
- **Framework**: PlatformIO
- **Language**: C++ (C++98/03 standard for MSP430 compatibility)
- **Build System**: PlatformIO with MSP430 GCC toolchain
- **IDE**: Optimized for Visual Studio Code with PlatformIO extension

## Dependencies

- PlatformIO Core
- MSP430 platform support for PlatformIO
- energia framework (via PlatformIO)

## Installation and Setup

### Prerequisites

1. Install [Visual Studio Code](https://code.visualstudio.com/)
2. Install the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode) for VSCode
3. Ensure you have the MSP430 LaunchPad drivers installed

### Building the Project

1. Clone the repository:
   ```bash
   git clone [https://github.com/brotherdust/cycfi-nexus-midi.git](https://github.com/brotherdust/cycfi-nexus-midi.git)
   cd cycfi-nexus-midi
   ```

2. Open the project in VSCode:
   ```bash
   code .
   ```

3. Wait for PlatformIO to initialize and download dependencies

4. Build the project using the PlatformIO toolbar or:
   - Press `Ctrl+Alt+B` (or `Cmd+Alt+B` on macOS)
   - Or use the PlatformIO: Build command from the command palette

### Uploading to Hardware

1. Connect your MSP430G2553 LaunchPad via USB
2. Click the Upload button in the PlatformIO toolbar
3. Or press `Ctrl+Alt+U` (or `Cmd+Alt+U` on macOS)

## Usage Examples

### Basic Controller Setup

```cpp
#include "controllers/controller_factory.hpp"
#include "config/hardware_config.hpp"

// Create a program change controller
nexus::controllers::BaseController* pcController = 
    nexus::controllers::ControllerFactory::createController(
        nexus::controllers::PROGRAM_CHANGE
    );

// Initialize the controller
pcController->init();

// Process MIDI events in your main loop
void loop() {
    pcController->update();
}
```

### Configuring Hardware Pins

```cpp
// Hardware configuration is centralized in hardware_config.hpp
// Modify pin assignments and hardware parameters there
```

### Using Persistent Storage

```cpp
#include "storage/persistent_storage.hpp"

// Save current configuration
nexus::storage::PersistentStorage::saveConfiguration();

// Load saved configuration
nexus::storage::PersistentStorage::loadConfiguration();
```

## Project Structure

```
.
├── platformio.ini          # PlatformIO configuration
├── include/               # Header files
│   ├── config/           # Configuration headers
│   │   ├── feature_config.hpp
│   │   └── hardware_config.hpp
│   ├── controllers/      # Controller interfaces
│   │   ├── base_controller.hpp
│   │   ├── controller_factory.hpp
│   │   └── [specific controllers].hpp
│   ├── storage/          # Storage interfaces
│   │   ├── flash_manager.hpp
│   │   └── persistent_storage.hpp
│   ├── test/            # Test framework
│   ├── midi.hpp         # MIDI protocol definitions
│   └── util.hpp         # Utility functions
├── src/                 # Implementation files
│   ├── nexus_midi.ino   # Main program entry
│   ├── config/          # Configuration implementations
│   ├── controllers/     # Controller implementations
│   ├── storage/         # Storage implementations
│   └── test/           # Test implementations
└── docs/               # Documentation
    └── PC_CC_MAPPING_CONFIG.md
```

## Memory Considerations

This project is optimized for the MSP430G2553's limited resources:
- Flash: 16KB (monitor usage with `msp430-size`)
- RAM: 512B
- Uses C++98/03 features only (no C++11 or later)
- Avoids virtual functions where possible to minimize vtable overhead
- Prefers `constexpr` for compile-time constants

## Contributing

Contributions are welcome! Please follow these guidelines:

1. Maintain C++98/03 compatibility
2. Follow the existing namespace structure (nexus::)
3. Keep memory usage minimal
4. Add tests for new features
5. Use conventional commit messages:
   - `feat:` for new features
   - `fix:` for bug fixes
   - `refactor:` for code restructuring
   - `docs:` for documentation updates
   - `test:` for test additions/modifications

### Code Style

- Use the nexus namespace with appropriate sub-namespaces
- Initialize all class members in constructors
- Avoid dynamic memory allocation
- Prefer compile-time configuration over runtime

## License

This project is provided as-is for educational and personal use. Please check the repository for specific license information.

## Support and Contact

For questions, issues, or contributions:
- Open an issue in the project repository
- Check existing documentation in the `/docs` directory
- Review the test cases for usage examples

## Acknowledgments

- **Original Nexus Codebase**: Created by [Cycfi Research](https://www.cycfi.com/)
- **Platform**: Built with PlatformIO and the MSP430 platform
- **Framework**: Leverages the energia framework for Arduino-compatible development on MSP430 microcontrollers

This refactoring project respects and preserves the innovative design of the original Cycfi Nexus while extending its capabilities for modern use cases.