//===-- Lower/CharacterExpr.h -- lowering of characters ---------*- C++ -*-===//
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

#ifndef FORTRAN_LOWER_CHARACTEREXPR_H
#define FORTRAN_LOWER_CHARACTEREXPR_H

#include "flang/Lower/FIRBuilder.h"
#include "flang/Lower/Support/BoxValue.h"

namespace Fortran::lower {

/// Helper to facilitate lowering of CHARACTER in FIR.
class CharacterExprHelper {
public:
  /// Constructor.
  explicit CharacterExprHelper(FirOpBuilder &builder, mlir::Location loc)
      : builder{builder}, loc{loc} {}
  CharacterExprHelper(const CharacterExprHelper &) = delete;

  /// Unless otherwise stated, all mlir::Value inputs of these pseudo-fir ops
  /// must be of type:
  /// - fir.boxchar<kind> (dynamic length character),
  /// - fir.ref<fir.array<len x fir.char<kind>>> (character with compile time
  ///      constant length),
  /// - fir.array<len x fir.char<kind>> (compile time constant character)

  /// Copy the \p count first characters of \p src into \p dest.
  /// \p count can have any integer type.
  void createCopy(const fir::CharBoxValue &dest, const fir::CharBoxValue &src,
                  mlir::Value count);

  /// Set characters of \p str at position [\p lower, \p upper) to blanks.
  /// \p lower and \upper bounds are zero based.
  /// If \p upper <= \p lower, no padding is done.
  /// \p upper and \p lower can have any integer type.
  void createPadding(const fir::CharBoxValue &str, mlir::Value lower,
                     mlir::Value upper);

  /// Create str(lb:ub), lower bounds must always be specified, upper
  /// bound is optional.
  fir::CharBoxValue createSubstring(const fir::CharBoxValue &str,
                                    llvm::ArrayRef<mlir::Value> bounds);

  /// Return blank character of given \p type !fir.char<kind>
  mlir::Value createBlankConstant(fir::CharacterType type);

  /// Lower \p lhs = \p rhs where \p lhs and \p rhs are scalar characters.
  /// It handles cases where \p lhs and \p rhs may overlap.
  void createAssign(const fir::ExtendedValue &lhs,
                    const fir::ExtendedValue &rhs);

  /// Lower an assignment where the buffer and LEN parameter are known and do
  /// not need to be unboxed.
  void createAssign(mlir::Value lptr, mlir::Value llen, mlir::Value rptr,
                    mlir::Value rlen);

  /// Create lhs // rhs in temp obtained with fir.alloca
  fir::CharBoxValue createConcatenate(const fir::CharBoxValue &lhs,
                                      const fir::CharBoxValue &rhs);

  /// LEN_TRIM intrinsic.
  mlir::Value createLenTrim(mlir::Value str);

  /// Embox \p addr and \p len and return fir.boxchar.
  /// Take care of type conversions before emboxing.
  /// \p len is converted to the integer type for character lengths if needed.
  mlir::Value createEmboxChar(mlir::Value addr, mlir::Value len);
  mlir::Value createEmbox(const fir::CharBoxValue &str);
  /// Embox a string array. The length is sizeof(str)*len(str).
  mlir::Value createEmbox(const fir::CharArrayBoxValue &str);

  /// Convert character array to a scalar by reducing the extents into the
  /// length. Will fail if call on non reference like base.
  fir::CharBoxValue toScalarCharacter(const fir::CharArrayBoxValue &);

  /// Unbox \p boxchar into (fir.ref<fir.char<kind>>, getLengthType()).
  std::pair<mlir::Value, mlir::Value> createUnboxChar(mlir::Value boxChar);

  /// Allocate a temp of fir::CharacterType type and length len.
  /// Returns related fir.ref<fir.array<? x fir.char<kind>>>.
  fir::CharBoxValue createCharacterTemp(mlir::Type type, mlir::Value len);

  /// Allocate a temp of compile time constant length.
  /// Returns related fir.ref<fir.array<len x fir.char<kind>>>.
  fir::CharBoxValue createCharacterTemp(mlir::Type type, int len);

  /// Return buffer/length pair of character str, if str is a constant,
  /// it is allocated into a temp, otherwise, its memory reference is
  /// returned as the buffer.
  /// The buffer type of str is of type:
  ///   - fir.ref<fir.array<len x fir.char<kind>>> if str has compile time
  ///      constant length.
  ///   - fir.ref<fir.char<kind>> if str has dynamic length.
  std::pair<mlir::Value, mlir::Value> materializeCharacter(mlir::Value str);

  /// Return the (buffer, length) pair of `str`. Returns the obvious pair if
  /// `str` is a scalar. However if `str` is an array of CHARACTER, this will
  /// perform an implicit concatenation of the entire array. This implements the
  /// implied semantics of using an array of CHARACTER in a scalar context.
  std::pair<mlir::Value, mlir::Value>
  materializeCharacterOrSequence(mlir::Value str);

  /// Return true if \p type is a character literal type (is
  /// `fir.array<len x fir.char<kind>>`).;
  static bool isCharacterLiteral(mlir::Type type);

  /// Return true if \p type is one of the following type
  /// - fir.boxchar<kind>
  /// - fir.ref<fir.array<len x fir.char<kind>>>
  /// - fir.array<len x fir.char<kind>>
  static bool isCharacter(mlir::Type type);

  /// Extract the kind of a character type
  static fir::KindTy getCharacterKind(mlir::Type type);

  /// Extract the kind of a character or array of character type.
  static fir::KindTy getCharacterOrSequenceKind(mlir::Type type);

  /// Determine the base character type
  static fir::CharacterType getCharacterType(mlir::Type type);
  static fir::CharacterType getCharacterType(const fir::CharBoxValue &box);
  static fir::CharacterType getCharacterType(mlir::Value str);

  /// Return the integer type that must be used to manipulate
  /// Character lengths. TODO: move this to FirOpBuilder?
  mlir::Type getLengthType() { return builder.getIndexType(); }

  /// Create an extended value from:
  /// - fir.boxchar<kind>
  /// - fir.ref<fir.array<len x fir.char<kind>>>
  /// - fir.array<len x fir.char<kind>>
  /// - fir.char<kind>
  /// - fir.ref<char<kind>>
  ///
  /// Does the heavy lifting of converting the value \p character (along with an
  /// optional \p len value) to an extended value. If \p len is null, a length
  /// value is extracted from \p character (or its type). This will produce an
  /// error if it's not possible. The returned value is a CharBoxValue if \p
  /// character is a scalar, otherwise it is a CharArrayBoxValue.
  fir::ExtendedValue toExtendedValue(mlir::Value character,
                                     mlir::Value len = {});

  /// Is `type` a sequence (array) of CHARACTER type? Return true for any of the
  /// following cases:
  ///   - !fir.array<len x dim x ... x !fir.char<kind>>
  ///   - !fir.array<dim x !fir.char<kind, len>>
  ///   - !fir.ref<T>  where T is either of the first two cases
  ///   - !fir.box<T>  where T is either of the first two cases
  ///
  /// In certain contexts, Fortran allows an array of CHARACTERs to be treated
  /// as if it were one longer CHARACTER scalar, each element append to the
  /// previous.
  static bool isArray(mlir::Type type);

  /// Temporary helper to help migrating towards properties of
  /// ExtendedValue containing characters.
  /// Mainly, this ensure that characters are always CharArrayBoxValue,
  /// CharBoxValue, or BoxValue and that the base address is not a boxchar.
  /// Return the argument if this is not a character.
  /// TODO: Create and propagate ExtendedValue according to properties listed
  /// above instead of fixing it when needed.
  fir::ExtendedValue cleanUpCharacterExtendedValue(const fir::ExtendedValue &);

private:
  fir::CharBoxValue materializeValue(mlir::Value str);
  fir::CharBoxValue toDataLengthPair(mlir::Value character);
  mlir::Type getReferenceType(const fir::CharBoxValue &c) const;
  mlir::Type getReferenceType(mlir::Value str) const;
  mlir::Type getSeqTy(const fir::CharBoxValue &c) const;
  mlir::Type getSeqTy(mlir::Value str) const;
  mlir::Value getCharBoxBuffer(const fir::CharBoxValue &box);
  mlir::Value createLoadCharAt(mlir::Value buff, mlir::Value index);
  void createStoreCharAt(mlir::Value str, mlir::Value index, mlir::Value c);
  void createLengthOneAssign(const fir::CharBoxValue &lhs,
                             const fir::CharBoxValue &rhs);
  void createAssign(const fir::CharBoxValue &lhs, const fir::CharBoxValue &rhs);
  mlir::Value createLenTrim(const fir::CharBoxValue &str);
  mlir::Value createBlankConstantCode(fir::CharacterType type);

private:
  FirOpBuilder &builder;
  mlir::Location loc;
};

} // namespace Fortran::lower

#endif // FORTRAN_LOWER_CHARACTEREXPR_H
