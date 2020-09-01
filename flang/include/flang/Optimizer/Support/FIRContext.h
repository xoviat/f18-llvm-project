//===-- Optimizer/Support/FIRContext.h --------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// Coding style: https://mlir.llvm.org/getting_started/DeveloperGuide/
//
//===----------------------------------------------------------------------===//
/// \file
///
/// Setters and getters for associating context with an instance of a ModuleOp.
/// The context is typically set by the tool and needed in later stages to
/// determine how to correctly generate code.
//===----------------------------------------------------------------------===//

#ifndef OPTIMIZER_SUPPORT_FIRCONTEXT_H
#define OPTIMIZER_SUPPORT_FIRCONTEXT_H

#include "llvm/ADT/Triple.h"

namespace mlir {
class ModuleOp;
}

namespace fir {
class KindMapping;
struct NameUniquer;

/// Set the target triple for the module. `triple` must not be deallocated while
/// module `mod` is still live.
void setTargetTriple(mlir::ModuleOp mod, llvm::Triple &triple);

/// Get a pointer to the Triple instance from the Module. If none was set,
/// returns a nullptr.
llvm::Triple *getTargetTriple(mlir::ModuleOp mod);

/// Set the name uniquer for the module. `uniquer` must not be deallocated while
/// module `mod` is still live.
void setNameUniquer(mlir::ModuleOp mod, NameUniquer &uniquer);

/// Get a pointer to the NameUniquer instance from the Module. If none was set,
/// returns a nullptr.
NameUniquer *getNameUniquer(mlir::ModuleOp mod);

/// Set the kind mapping for the module. `kindMap` must not be deallocated while
/// module `mod` is still live.
void setKindMapping(mlir::ModuleOp mod, KindMapping &kindMap);

/// Get a pointer to the KindMapping instance from the Module. If none was set,
/// returns a nullptr.
KindMapping *getKindMapping(mlir::ModuleOp mod);

/// Helper for determining the target from the host, etc. Tools may use this
/// function to provide a consistent interpretation of the `--target=<string>`
/// command-line option.
/// An empty string ("") or "default" will specify that the default triple
/// should be used. "native" will specify that the host machine be used to
/// construct the triple.
std::string determineTargetTriple(llvm::StringRef triple);

} // namespace fir

#endif // OPTIMIZER_SUPPORT_FIRCONTEXT_H
