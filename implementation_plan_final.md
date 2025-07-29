# Nexus MIDI Controller - Final Implementation Plan

## Executive Summary

With **34.4% Flash** and **44.1% RAM** usage, we have excellent headroom for full modular refactoring. This plan transforms the monolithic 803-line file into a well-organized, maintainable architecture.

## Key Design Decisions

### 1. Architecture Overview
```
nexus_midi/
├── include/
│   ├── config/
│   │   ├── hardware_config.hpp      # Pin definitions, hardware constants
│   │   └── test_config.hpp          # Test mode defines
│   ├── storage/
│   │   ├── flash_manager.hpp        # Flash memory abstraction
│   │   └── persistent_storage.hpp   # Save delay mechanism
│   ├── controllers/
│   │   ├── base_controller.hpp      # Template base controller
│   │   ├── pitch_bend_controller.hpp
│   │   ├── program_change_controller.hpp
│   │   ├── sustain_controller.hpp
│   │   ├── bank_select_controller.hpp
│   │   └── controller_factory.hpp   # Singleton for controller instances
│   └── test/
│       └── test_runner.hpp          # Test mode implementations
├── src/
│   ├── [corresponding .cpp files]
│   └── nexus_midi.ino              # Main with setup() and loop()
```

### 2. Global Variable Strategy
- **midi_out**: Remains in main file, accessed via extern
- **Controllers**: Managed by singleton ControllerFactory
- **Pin constants**: Extern const in config namespace
- **Flash instances**: Extern in storage namespace

### 3. Memory-Conscious Design
- No virtual functions (avoid vtables)
- Static allocation only
- Inline small functions
- Use constexpr where possible

## Implementation Sequence

### Phase 1: Foundation (Low Risk)
1. **Create directory structure**
2. **Extract hardware configuration**
   - Move pin definitions
   - Move hardware constants
   - Move analog_read function
3. **Validate build** after each step

### Phase 2: Storage (Medium Risk)
4. **Extract flash manager**
   - Move flash struct
   - Create flash_b and flash_c instances
5. **Extract persistent storage**
   - Move save delay mechanism
6. **Validate** flash operations still work

### Phase 3: Controllers (Higher Risk)
7. **Create base controller template**
   - Extract generic controller logic
   - Handle midi_out access pattern
8. **Extract individual controllers** (one at a time)
   - Pitch bend
   - Program change (with flash integration)
   - Sustain
   - Bank select (with flash integration)
9. **Create controller factory**
   - Singleton pattern
   - Lazy initialization

### Phase 4: Finalization
10. **Extract test framework**
11. **Update main file**
12. **Final validation**

## Critical Implementation Details

### Handling midi_out Access
```cpp
// In nexus_midi.ino
cycfi::midi::midi_stream midi_out;

// In any controller needing access
extern cycfi::midi::midi_stream midi_out;
```

### Template Controller Pattern
```cpp
// base_controller.hpp
template <cycfi::midi::cc::controller ctrl>
class Controller {
    // Implementation in .tpp file
};

// Include template implementation
#include "controllers/base_controller.tpp"
```

### Controller Factory Pattern
```cpp
class ControllerFactory {
public:
    static ControllerFactory& instance() {
        static ControllerFactory instance;
        return instance;
    }
    // Controller accessors...
private:
    ControllerFactory(); // Initialize all controllers
};
```

## Validation Checkpoints

After each major step:
1. Run build: `pio run`
2. Check memory: `msp430-size -B firmware.elf`
3. Verify no increase > 5% Flash or 10% RAM
4. Test functionality if device available

## Risk Mitigation

1. **Git Strategy**: Commit after each successful extraction
2. **Rollback Plan**: Each phase is independently revertible
3. **Testing**: Maintain test modes throughout refactoring
4. **Memory Monitoring**: Track usage after each phase

## Success Criteria

- [ ] All builds succeed without warnings
- [ ] Memory usage stays under 40% Flash, 55% RAM
- [ ] All functionality preserved
- [ ] Code follows SOLID principles
- [ ] Clear module boundaries established

## Next Steps

1. Create a feature branch for refactoring
2. Set up memory monitoring script
3. Begin with Phase 1: Hardware Configuration
4. Proceed incrementally with validation

## Estimated Timeline

- Phase 1: 30 minutes
- Phase 2: 45 minutes
- Phase 3: 2 hours
- Phase 4: 30 minutes
- Testing & Validation: 1 hour

**Total: ~4-5 hours** for complete refactoring

## Questions Before Starting

1. ✅ Memory headroom confirmed (GREEN ZONE)
2. ✅ PlatformIO build system ready
3. ✅ Refactoring plan reviewed
4. ⏳ Git branch created?
5. ⏳ Backup of working code?

Ready to proceed with implementation!