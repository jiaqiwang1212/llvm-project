<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# Bufferization Pipelines

## Purpose
Pre-built pass pipeline declarations for the complete bufferization workflow (One-Shot Bufferize + deallocation + copy insertion).

## Key Files
| File | Description |
|------|-------------|
| `Passes.h` | Pipeline registration and option struct declarations |

## For AI Agents

### Working In This Directory
- Use `buildOneShotBufferizationPipeline` to construct the standard pipeline
- Pipeline options are configured via `OneShotBufferizationOptions`

## Dependencies
- Depends on: Bufferization Transforms, MemRef dialect

<!-- MANUAL: -->
