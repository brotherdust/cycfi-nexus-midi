# Kilocode Custom Rules for Nexus MIDI Controller Refactoring

## Git Commit Messages
- Follow strict git commit message best practices
- Subject line: max 50 characters, imperative mood, no period
- Use conventional commit format: type: brief description
- Types: refactor, fix, feat, docs, test, chore
- Example: "refactor: extract hardware configuration module"
- Blank line between subject and body
- Body: wrap at 72 characters, explain what and why
- No markdown, no bullet points, no special formatting

## Git Staging
- Only stage specific files that were worked on
- Never use `git add -A` or `git add .`
- List files explicitly: `git add file1 file2 file3`
- Review staged files before committing

## Git Commit Frequency
- Commit after completing each todo list item
- Stage and commit immediately after successful implementation
- Each commit should represent one logical unit of work
- Example workflow:
  1. Complete "Implement ring buffer template class"
  2. Stage: `git add include/debug/ring_buffer.hpp`
  3. Commit: `git commit -m "feat: add ring buffer for debug logging"`
  4. Move to next todo item
- This ensures incremental progress is saved and trackable

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

## C++ Language Restrictions
- DO NOT use C++11 features - compiler only supports C++98/03
- No in-class member initialization
- No auto keyword
- No nullptr (use NULL)
- No range-based for loops
- No lambda expressions
- Initialize all class members in constructors