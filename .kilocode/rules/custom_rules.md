# Kilocode Custom Rules for Nexus MIDI Controller Refactoring

## Git Commit Messages
- Use plain text only - no markdown formatting
- Keep messages concise and under 72 characters for the subject line
- Use conventional commit format: type: brief description
- Types: refactor, fix, feat, docs, test, chore
- Example: "refactor: extract hardware configuration module"
- If details needed, add blank line then bullet points (no special formatting)

## File Creation Policy
- Minimize unnecessary documentation files during active development
- Wait for user confirmation before creating summary/progress files
- Focus on code implementation over documentation generation

## Build Validation
- Always validate build after each extraction phase
- User will run builds via PlatformIO VSCode extension
- Wait for build results before proceeding to next phase
- Track memory usage changes after each phase

## Code Organization
- Use nexus namespace with sub-namespaces (config, storage, controllers, etc.)
- Maintain original global variable linkage patterns
- Prefer constexpr for template parameters
- Avoid virtual functions to minimize vtable overhead

## Refactoring Approach
- Extract one module at a time
- Validate compilation after each extraction
- Commit after successful build validation
- Keep memory overhead under 5% per phase

## PlatformIO Specifics
- Use VSCode PlatformIO extension for builds
- Don't attempt command-line PlatformIO commands
- Build outputs are in .pio/build/lpmsp430g2553/
- Memory analysis via msp430-size tool

## Communication Style
- Be direct and technical, avoid conversational filler
- Don't create files just to summarize - ask first
- Focus on implementation over documentation
- Respect token usage - be concise