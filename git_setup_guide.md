# Git Setup and Backup Guide

## 1. Initialize Git Repository (if not already done)

```bash
cd /Users/jarad/Documents/PlatformIO/Projects/250721-175828-lpmsp430g2553
git init
```

## 2. Create .gitignore File

Create a `.gitignore` file if it doesn't exist:

```gitignore
# PlatformIO
.pio/
.vscode/.browse.c_cpp.db*
.vscode/c_cpp_properties.json
.vscode/launch.json
.vscode/ipch

# Build outputs
*.elf
*.hex
*.bin
*.map

# OS files
.DS_Store
Thumbs.db

# Backup files
*.bak
*~
```

## 3. Initial Commit (if needed)

```bash
# Add all files
git add .

# Commit current working state
git commit -m "Initial commit: working MIDI controller before refactoring"
```

## 4. Create Refactoring Branch

```bash
# Create and switch to new branch
git checkout -b refactor/modular-architecture

# Verify you're on the new branch
git branch
```

## 5. Create Manual Backup

```bash
# Create backup directory with timestamp
mkdir -p ../backups
cp -r . ../backups/nexus-midi-backup-$(date +%Y%m%d-%H%M%S)

# Or create a zip archive
zip -r ../nexus-midi-backup-$(date +%Y%m%d-%H%M%S).zip . -x ".pio/*" -x ".git/*"
```

## 6. Verify Everything is Ready

```bash
# Check git status
git status

# Verify you're on the refactoring branch
git branch --show-current

# List recent commits
git log --oneline -5
```

## Git Commands During Refactoring

### After each successful phase:
```bash
# Stage changes
git add .

# Commit with descriptive message
git commit -m "refactor: extract hardware configuration module"
```

### If something goes wrong:
```bash
# Discard all changes and go back to last commit
git reset --hard HEAD

# Or go back to a specific commit
git reset --hard <commit-hash>
```

### To see what changed:
```bash
# See unstaged changes
git diff

# See staged changes
git diff --staged

# See all changes since branch creation
git diff main..refactor/modular-architecture
```

## Recommended Commit Messages

Use conventional commit format:

- `refactor: extract hardware configuration module`
- `refactor: create flash storage abstraction`
- `refactor: implement controller factory pattern`
- `refactor: extract MIDI controllers to separate modules`
- `test: add memory usage validation`
- `docs: update architecture documentation`

## When Refactoring is Complete

```bash
# Switch back to main branch
git checkout main

# Merge the refactoring
git merge refactor/modular-architecture

# Or create a pull request if using GitHub/GitLab
```

## Emergency Rollback

If you need to completely abandon the refactoring:

```bash
# Switch back to main
git checkout main

# Delete the refactoring branch
git branch -D refactor/modular-architecture

# Restore from backup if needed
cp -r ../backups/nexus-midi-backup-TIMESTAMP/* .