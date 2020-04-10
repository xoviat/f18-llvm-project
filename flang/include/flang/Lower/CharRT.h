//===-- Lower/CharRT.h -- lower CHARACTER operations ------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef FORTRAN_LOWER_CHARRT_H
#define FORTRAN_LOWER_CHARRT_H

#include "flang/Common/Fortran.h"
#include "mlir/IR/Value.h"

namespace Fortran {
namespace lower {
class AbstractConverter;

/// Generate call to a character comparison for two ssa-values of type
/// `boxchar`.
mlir::Value genBoxCharCompare(AbstractConverter &converter, mlir::Location loc,
                              common::RelationalOperator cmp,
                              mlir::Value lhsBuff, mlir::Value rhsBuff);

/// Generate call to a character comparison op for two unboxed variables. There
/// are 4 arguments, 2 for the lhs and 2 for the rhs. Each CHARACTER must pass a
/// reference to its buffer (`ref<char<K>>`) and its LEN type parameter (some
/// integral type).
mlir::Value genRawCharCompare(AbstractConverter &converter, mlir::Location loc,
                              common::RelationalOperator cmp,
                              mlir::Value lhsBuff, mlir::Value lhsLen,
                              mlir::Value rhsBuff, mlir::Value rhsLen);

} // namespace lower
} // namespace Fortran

#endif // FORTRAN_LOWER_CHARRT_H
