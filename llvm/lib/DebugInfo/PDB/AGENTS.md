<!-- Parent: ../AGENTS.md -->
<!-- Generated: 2026-05-07 | Updated: 2026-05-07 -->

# PDB

## Purpose
Microsoft PDB (Program Database) file format reader and writer. Provides access to Windows debug symbols, type information, and symbol records. Core infrastructure for Windows debugging support.

## Key Files
| File | Description |
|------|-------------|
| `PDB.cpp` | Main PDB file parsing entry point |
| `PDBContext.cpp` | PDB context and symbol session management |
| `PDBExtras.cpp` | Utilities and helper functions for PDB |
| `PDBInterfaceAnchors.cpp` | Interface anchor implementations |
| `IPDBSourceFile.cpp` | Source file interface implementation |
| `GenericError.cpp` | Generic error types and handling |
| `PDBSymbol.cpp` | Base PDB symbol representation |
| `PDBSymDumper.cpp` | Symbol dumping and pretty-printing |
| `UDTLayout.cpp` | User-defined type layout analysis |
| `PDBSymbolAnnotation.cpp` | Annotation symbol type |
| `PDBSymbolBlock.cpp` | Block symbol type |
| `PDBSymbolCompiland.cpp` | Compiland (source file) symbol |
| `PDBSymbolCompilandDetails.cpp` | Compiland details symbol |
| `PDBSymbolCompilandEnv.cpp` | Compiland environment symbol |
| `PDBSymbolCustom.cpp` | Custom/unknown symbol type |
| `PDBSymbolData.cpp` | Data variable symbol |
| `PDBSymbolExe.cpp` | Executable symbol |
| `PDBSymbolFunc.cpp` | Function symbol |
| `PDBSymbolFuncDebugStart.cpp` | Function debug start symbol |
| `PDBSymbolFuncDebugEnd.cpp` | Function debug end symbol |
| `PDBSymbolLabel.cpp` | Label symbol |
| `PDBSymbolPublicSymbol.cpp` | Public symbol |
| `PDBSymbolThunk.cpp` | Thunk symbol |
| `PDBSymbolTypeArray.cpp` | Array type symbol |
| `PDBSymbolTypeBaseClass.cpp` | Base class type symbol |
| `PDBSymbolTypeBuiltin.cpp` | Built-in type symbol |
| `PDBSymbolTypeCustom.cpp` | Custom type symbol |
| `PDBSymbolTypeDimension.cpp` | Array dimension type symbol |
| `PDBSymbolTypeEnum.cpp` | Enumeration type symbol |
| `PDBSymbolTypeFriend.cpp` | Friend type symbol |
| `PDBSymbolTypeFunctionArg.cpp` | Function argument type symbol |
| `PDBSymbolTypeFunctionSig.cpp` | Function signature type symbol |
| `PDBSymbolTypeManaged.cpp` | Managed code type symbol |
| `PDBSymbolTypePointer.cpp` | Pointer type symbol |
| `PDBSymbolTypeTypedef.cpp` | Typedef type symbol |
| `PDBSymbolTypeUDT.cpp` | User-defined type (struct/class/union) symbol |
| `PDBSymbolTypeVTable.cpp` | Virtual table type symbol |
| `PDBSymbolTypeVTableShape.cpp` | Virtual table shape type symbol |
| `PDBSymbolUnknown.cpp` | Unknown symbol type |
| `PDBSymbolUsingNamespace.cpp` | Using namespace declaration symbol |

## For AI Agents

### Working In This Directory
- PDB is complex; understand the symbol hierarchy before modifying individual symbol types.
- PDBContext is the main entry point; changes here affect all clients.
- Symbol types (PDBSymbolFunc, PDBSymbolData, etc.) follow a pattern; add new types consistently.
- Coordinate with `../CodeView/` for type and symbol record mapping.
- Coordinate with `../MSF/` for file container handling.
- UDTLayout is performance-critical for type analysis; optimize carefully.

### Dependencies

#### Internal
- Depends on `../CodeView/` (type and symbol record parsing)
- Depends on `../MSF/` (file container format)
- Depends on `llvm/lib/Support/` (error handling, utilities)
- Used by Windows debugging tools and debuggers

<!-- MANUAL: -->
