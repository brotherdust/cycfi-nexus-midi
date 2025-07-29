# Nexus MIDI Controller - Implementation Sequence

## Prerequisites
- Ensure PlatformIO is installed and configured in VSCode
- Have the project open in VSCode
- All changes should be validated with `pio run` after each step

## Implementation Steps

### Step 1: Create Directory Structure
```bash
# From project root
mkdir -p include/config
mkdir -p include/storage
mkdir -p include/controllers
mkdir -p include/test
mkdir -p src/config
mkdir -p src/storage
mkdir -p src/controllers
mkdir -p src/test
```

### Step 2: Extract Hardware Configuration
1. Create `include/config/hardware_config.hpp`
2. Create `src/config/hardware_config.cpp`
3. Move pin definitions and hardware functions
4. Update includes in main file
5. Run `pio run` to validate

### Step 3: Extract Flash Storage
1. Create `include/storage/flash_manager.hpp`
2. Create `src/storage/flash_manager.cpp`
3. Move flash struct and implementations
4. Create flash_b and flash_c instances
5. Run `pio run` to validate

### Step 4: Extract Persistent Storage
1. Create `include/storage/persistent_storage.hpp`
2. Create `src/storage/persistent_storage.cpp`
3. Move save delay mechanism
4. Run `pio run` to validate

### Step 5: Create Base Controller Architecture
1. Create `include/controllers/base_controller.hpp`
2. Create `include/controllers/base_controller.tpp`
3. Extract generic controller template
4. Define IController interface
5. Run `pio run` to validate

### Step 6: Extract Individual Controllers (one at a time)

#### 6.1 Pitch Bend Controller
1. Create `include/controllers/pitch_bend_controller.hpp`
2. Create `src/controllers/pitch_bend_controller.cpp`
3. Move pitch_bend_controller struct
4. Run `pio run` to validate

#### 6.2 Program Change Controller
1. Create `include/controllers/program_change_controller.hpp`
2. Create `src/controllers/program_change_controller.cpp`
3. Move program_change_controller struct
4. Update flash storage references
5. Run `pio run` to validate

#### 6.3 Sustain Controller
1. Create `include/controllers/sustain_controller.hpp`
2. Create `src/controllers/sustain_controller.cpp`
3. Move sustain_controller struct
4. Run `pio run` to validate

#### 6.4 Bank Select Controller
1. Create `include/controllers/bank_select_controller.hpp`
2. Create `src/controllers/bank_select_controller.cpp`
3. Move bank_select_controller struct
4. Update flash storage references
5. Run `pio run` to validate

### Step 7: Create Controller Factory
1. Create `include/controllers/controller_factory.hpp`
2. Create `src/controllers/controller_factory.cpp`
3. Implement singleton pattern
4. Move all controller instances
5. Run `pio run` to validate

### Step 8: Extract Test Framework
1. Create `include/config/test_config.hpp`
2. Move all test defines
3. Create `include/test/test_runner.hpp`
4. Create `src/test/test_runner.cpp`
5. Move test implementations
6. Run `pio run` to validate

### Step 9: Update Main File
1. Remove all extracted code from `nexus_midi.ino`
2. Add new include statements
3. Update setup() to use new modules
4. Update loop() to use controller factory
5. Run `pio run` to validate

### Step 10: Final Validation
1. Run complete build: `pio run --target clean && pio run`
2. Check for any warnings or errors
3. Verify binary size hasn't increased significantly
4. Test upload to device if available

## Validation Checklist After Each Step

- [ ] Code compiles without errors
- [ ] No unresolved symbols
- [ ] All includes are found
- [ ] No duplicate definitions
- [ ] Global variables maintain correct linkage
- [ ] Memory usage is reasonable

## Common Issues and Solutions

### Issue: Undefined reference to midi_out
**Solution**: Ensure midi_out is declared extern in headers and defined in main file

### Issue: Multiple definition errors
**Solution**: Check for missing inline keywords on template functions or duplicate definitions

### Issue: Cannot find include file
**Solution**: Verify include paths in platformio.ini or use relative paths

### Issue: Flash write functions not found
**Solution**: Ensure MspFlash.h is included where needed

## Build Commands Reference

```bash
# Clean build
pio run --target clean

# Normal build
pio run

# Verbose build (for debugging)
pio run -v

# Build specific environment
pio run -e lpmsp430g2553

# Upload to device
pio run --target upload
```

## Git Commit Strategy

After each successful step, commit with descriptive message:
```bash
git add .
git commit -m "refactor: extract hardware configuration module"
git commit -m "refactor: extract flash storage module"
# etc.
```

This allows easy rollback if issues arise.