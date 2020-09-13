/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_STATEMENT
#define SKSL_STATEMENT

#include "src/sksl/ir/SkSLIRNode.h"
#include "src/sksl/ir/SkSLType.h"

namespace SkSL {

/**
 * Abstract supertype of all statements.
 */
struct Statement : public IRNode {
    enum Kind {
        kBlock = (int) Symbol::Kind::kLast + 1,
        kBreak,
        kContinue,
        kDiscard,
        kDo,
        kExpression,
        kFor,
        kIf,
        kInlineMarker,
        kNop,
        kReturn,
        kSwitch,
        kSwitchCase,
        kVarDeclaration,
        kVarDeclarations,
        kWhile,

        kFirst = kBlock,
        kLast = kWhile
    };

    Statement(int offset, Kind kind)
    : INHERITED(offset, (int) kind) {
        SkASSERT(kind >= Kind::kFirst && kind <= Kind::kLast);
    }

    Kind kind() const {
        return (Kind) fKind;
    }

    /**
     *  Use is<T> to check the type of a statement.
     *  e.g. replace `s.fKind == Statement::kReturn_Kind` with `s.is<ReturnStatement>()`.
     */
    template <typename T>
    bool is() const {
        return this->fKind == T::kStatementKind;
    }

    /**
     *  Use as<T> to downcast statements.
     *  e.g. replace `(ReturnStatement&) s` with `s.as<ReturnStatement>()`.
     */
    template <typename T>
    const T& as() const {
        SkASSERT(this->is<T>());
        return static_cast<const T&>(*this);
    }

    template <typename T>
    T& as() {
        SkASSERT(this->is<T>());
        return static_cast<T&>(*this);
    }

    virtual bool isEmpty() const {
        return false;
    }

    virtual std::unique_ptr<Statement> clone() const = 0;

    using INHERITED = IRNode;
};

}  // namespace SkSL

#endif
