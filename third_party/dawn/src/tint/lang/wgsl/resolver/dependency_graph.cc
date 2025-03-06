// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/wgsl/resolver/dependency_graph.h"

#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "src/tint/lang/core/builtin_type.h"
#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/ast/alias.h"
#include "src/tint/lang/wgsl/ast/assignment_statement.h"
#include "src/tint/lang/wgsl/ast/blend_src_attribute.h"
#include "src/tint/lang/wgsl/ast/block_statement.h"
#include "src/tint/lang/wgsl/ast/break_if_statement.h"
#include "src/tint/lang/wgsl/ast/break_statement.h"
#include "src/tint/lang/wgsl/ast/call_statement.h"
#include "src/tint/lang/wgsl/ast/color_attribute.h"
#include "src/tint/lang/wgsl/ast/compound_assignment_statement.h"
#include "src/tint/lang/wgsl/ast/const.h"
#include "src/tint/lang/wgsl/ast/continue_statement.h"
#include "src/tint/lang/wgsl/ast/diagnostic_attribute.h"
#include "src/tint/lang/wgsl/ast/discard_statement.h"
#include "src/tint/lang/wgsl/ast/for_loop_statement.h"
#include "src/tint/lang/wgsl/ast/id_attribute.h"
#include "src/tint/lang/wgsl/ast/identifier.h"
#include "src/tint/lang/wgsl/ast/if_statement.h"
#include "src/tint/lang/wgsl/ast/increment_decrement_statement.h"
#include "src/tint/lang/wgsl/ast/input_attachment_index_attribute.h"
#include "src/tint/lang/wgsl/ast/internal_attribute.h"
#include "src/tint/lang/wgsl/ast/interpolate_attribute.h"
#include "src/tint/lang/wgsl/ast/invariant_attribute.h"
#include "src/tint/lang/wgsl/ast/let.h"
#include "src/tint/lang/wgsl/ast/location_attribute.h"
#include "src/tint/lang/wgsl/ast/loop_statement.h"
#include "src/tint/lang/wgsl/ast/must_use_attribute.h"
#include "src/tint/lang/wgsl/ast/override.h"
#include "src/tint/lang/wgsl/ast/return_statement.h"
#include "src/tint/lang/wgsl/ast/row_major_attribute.h"
#include "src/tint/lang/wgsl/ast/stage_attribute.h"
#include "src/tint/lang/wgsl/ast/stride_attribute.h"
#include "src/tint/lang/wgsl/ast/struct.h"
#include "src/tint/lang/wgsl/ast/struct_member_align_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_offset_attribute.h"
#include "src/tint/lang/wgsl/ast/struct_member_size_attribute.h"
#include "src/tint/lang/wgsl/ast/switch_statement.h"
#include "src/tint/lang/wgsl/ast/templated_identifier.h"
#include "src/tint/lang/wgsl/ast/traverse_expressions.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/ast/variable_decl_statement.h"
#include "src/tint/lang/wgsl/ast/while_statement.h"
#include "src/tint/lang/wgsl/ast/workgroup_attribute.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/scope_stack.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/macros/compiler.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/memory/block_allocator.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string.h"
#include "src/tint/utils/text/string_stream.h"

#define TINT_DUMP_DEPENDENCY_GRAPH 0

namespace tint::resolver {
namespace {

// Forward declaration
struct Global;

/// Dependency describes how one global depends on another global
struct DependencyInfo {
    /// The source of the symbol that forms the dependency
    Source source;
};

/// DependencyEdge describes the two Globals used to define a dependency
/// relationship.
struct DependencyEdge {
    /// The Global that depends on #to
    const Global* from;
    /// The Global that is depended on by #from
    const Global* to;

    /// @returns the hash code of the DependencyEdge
    tint::HashCode HashCode() const { return Hash(from, to); }

    /// Equality operator
    bool operator==(const DependencyEdge& rhs) const { return from == rhs.from && to == rhs.to; }
};

/// A map of DependencyEdge to DependencyInfo
using DependencyEdges = Hashmap<DependencyEdge, DependencyInfo, 64>;

/// Global describes a module-scope variable, type or function.
struct Global {
    explicit Global(const ast::Node* n) : node(n) {}

    /// The declaration ast::Node
    const ast::Node* node;
    /// A list of dependencies that this global depends on
    Vector<Global*, 8> deps;
};

/// A map of global name to Global
using GlobalMap = Hashmap<Symbol, Global*, 16>;

/// @returns a new error diagnostic with the given source.
diag::Diagnostic& AddError(diag::List& diagnostics, const Source& source) {
    return diagnostics.AddError(source);
}

/// @returns a new note diagnostic with the given source.
diag::Diagnostic& AddNote(diag::List& diagnostics, const Source& source) {
    return diagnostics.AddNote(source);
}

/// DependencyScanner is used to traverse a module to build the list of
/// global-to-global dependencies.
class DependencyScanner {
  public:
    /// Constructor
    /// @param globals_by_name map of global symbol to Global pointer
    /// @param diagnostics diagnostic messages, appended with any errors found
    /// @param graph the dependency graph to populate with resolved symbols
    /// @param edges the map of globals-to-global dependency edges, which will
    /// be populated by calls to Scan()
    DependencyScanner(const GlobalMap& globals_by_name,
                      diag::List& diagnostics,
                      DependencyGraph& graph,
                      DependencyEdges& edges)
        : globals_(globals_by_name),
          diagnostics_(diagnostics),
          graph_(graph),
          dependency_edges_(edges) {
        // Register all the globals at global-scope
        for (auto& it : globals_by_name) {
            scope_stack_.Set(it.key, it.value->node);
        }
    }

    /// Walks the global declarations, resolving symbols, and determining the
    /// dependencies of each global.
    void Scan(Global* global) {
        TINT_SCOPED_ASSIGNMENT(current_global_, global);
        Switch(
            global->node,
            [&](const ast::Struct* str) {
                Declare(str->name->symbol, str);
                for (auto* member : str->members) {
                    TraverseAttributes(member->attributes);
                    TraverseExpression(member->type);
                }
            },
            [&](const ast::Alias* alias) {
                Declare(alias->name->symbol, alias);
                TraverseExpression(alias->type);
            },
            [&](const ast::Function* func) {
                Declare(func->name->symbol, func);
                TraverseFunction(func);
            },
            [&](const ast::Variable* v) {
                Declare(v->name->symbol, v);
                TraverseVariable(v);
            },
            [&](const ast::DiagnosticDirective*) {
                // Diagnostic directives do not affect the dependency graph.
            },
            [&](const ast::Enable*) {
                // Enable directives do not affect the dependency graph.
            },
            [&](const ast::Requires*) {
                // Requires directives do not affect the dependency graph.
            },
            [&](const ast::ConstAssert* assertion) {
                TraverseExpression(assertion->condition);
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

  private:
    /// Traverses the variable, performing symbol resolution.
    void TraverseVariable(const ast::Variable* v) {
        if (auto* var = v->As<ast::Var>()) {
            TraverseExpression(var->declared_address_space);
            TraverseExpression(var->declared_access);
        }
        TraverseExpression(v->type);
        TraverseAttributes(v->attributes);
        TraverseExpression(v->initializer);
    }

    /// Traverses the function, performing symbol resolution and determining global dependencies.
    void TraverseFunction(const ast::Function* func) {
        TraverseAttributes(func->attributes);
        TraverseAttributes(func->return_type_attributes);
        // Perform symbol resolution on all the parameter types before registering
        // the parameters themselves. This allows the case of declaring a parameter
        // with the same identifier as its type.
        for (auto* param : func->params) {
            TraverseAttributes(param->attributes);
            TraverseExpression(param->type);
        }
        // Resolve the return type
        TraverseExpression(func->return_type);

        // Push the scope stack for the parameters and function body.
        scope_stack_.Push();
        TINT_DEFER(scope_stack_.Pop());

        for (auto* param : func->params) {
            if (auto shadows = scope_stack_.Get(param->name->symbol)) {
                graph_.shadows.Add(param, shadows);
            }
            Declare(param->name->symbol, param);
        }
        if (func->body) {
            TraverseStatements(func->body->statements);
        }
    }

    /// Traverses the statements, performing symbol resolution and determining
    /// global dependencies.
    void TraverseStatements(VectorRef<const ast::Statement*> stmts) {
        for (auto* s : stmts) {
            TraverseStatement(s);
        }
    }

    /// Traverses the statement, performing symbol resolution and determining
    /// global dependencies.
    void TraverseStatement(const ast::Statement* stmt) {
        if (!stmt) {
            return;
        }
        Switch(
            stmt,  //
            [&](const ast::AssignmentStatement* a) {
                TraverseExpression(a->lhs);
                TraverseExpression(a->rhs);
            },
            [&](const ast::BlockStatement* b) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseStatements(b->statements);
            },
            [&](const ast::BreakIfStatement* b) { TraverseExpression(b->condition); },
            [&](const ast::CallStatement* r) { TraverseExpression(r->expr); },
            [&](const ast::CompoundAssignmentStatement* a) {
                TraverseExpression(a->lhs);
                TraverseExpression(a->rhs);
            },
            [&](const ast::ForLoopStatement* l) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseStatement(l->initializer);
                TraverseExpression(l->condition);
                TraverseStatement(l->continuing);
                TraverseStatement(l->body);
            },
            [&](const ast::IncrementDecrementStatement* i) { TraverseExpression(i->lhs); },
            [&](const ast::LoopStatement* l) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseStatements(l->body->statements);
                TraverseStatement(l->continuing);
            },
            [&](const ast::IfStatement* i) {
                TraverseExpression(i->condition);
                TraverseStatement(i->body);
                if (i->else_statement) {
                    TraverseStatement(i->else_statement);
                }
            },
            [&](const ast::ReturnStatement* r) { TraverseExpression(r->value); },
            [&](const ast::SwitchStatement* s) {
                TraverseExpression(s->condition);
                for (auto* c : s->body) {
                    for (auto* sel : c->selectors) {
                        TraverseExpression(sel->expr);
                    }
                    TraverseStatement(c->body);
                }
            },
            [&](const ast::VariableDeclStatement* v) {
                if (auto* shadows = scope_stack_.Get(v->variable->name->symbol)) {
                    graph_.shadows.Add(v->variable, shadows);
                }
                TraverseVariable(v->variable);
                Declare(v->variable->name->symbol, v->variable);
            },
            [&](const ast::WhileStatement* w) {
                scope_stack_.Push();
                TINT_DEFER(scope_stack_.Pop());
                TraverseExpression(w->condition);
                TraverseStatement(w->body);
            },
            [&](const ast::ConstAssert* assertion) { TraverseExpression(assertion->condition); },
            [&](const ast::BreakStatement*) {},     //
            [&](const ast::ContinueStatement*) {},  //
            [&](const ast::DiscardStatement*) {},   //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Adds the symbol definition to the current scope, raising an error if two
    /// symbols collide within the same scope.
    void Declare(Symbol symbol, const ast::Node* node) {
        auto* old = scope_stack_.Set(symbol, node);
        if (old != nullptr && node != old) {
            auto name = symbol.Name();
            AddError(diagnostics_, node->source) << "redeclaration of '" << name << "'";
            AddNote(diagnostics_, old->source) << "'" << name << "' previously declared here";
        }
    }

    /// Traverses the expression @p root_expr, performing symbol resolution and determining global
    /// dependencies.
    void TraverseExpression(const ast::Expression* root_expr) {
        if (!root_expr) {
            return;
        }

        Vector<const ast::Expression*, 8> pending{root_expr};
        while (!pending.IsEmpty()) {
            auto* next = pending.Pop();
            bool ok = ast::TraverseExpressions(next, [&](const ast::IdentifierExpression* e) {
                AddDependency(e->identifier, e->identifier->symbol);
                return ast::TraverseAction::Descend;
            });
            if (!ok) {
                AddError(diagnostics_, next->source) << "TraverseExpressions failed";
                return;
            }
        }
    }

    /// Traverses the attribute list, performing symbol resolution and
    /// determining global dependencies.
    void TraverseAttributes(VectorRef<const ast::Attribute*> attrs) {
        for (auto* attr : attrs) {
            TraverseAttribute(attr);
        }
    }

    /// Traverses the attribute, performing symbol resolution and determining
    /// global dependencies.
    void TraverseAttribute(const ast::Attribute* attr) {
        Switch(
            attr,  //
            [&](const ast::BindingAttribute* binding) { TraverseExpression(binding->expr); },
            [&](const ast::ColorAttribute* color) { TraverseExpression(color->expr); },
            [&](const ast::GroupAttribute* group) { TraverseExpression(group->expr); },
            [&](const ast::IdAttribute* id) { TraverseExpression(id->expr); },
            [&](const ast::InputAttachmentIndexAttribute* idx) { TraverseExpression(idx->expr); },
            [&](const ast::BlendSrcAttribute* index) { TraverseExpression(index->expr); },
            [&](const ast::LocationAttribute* loc) { TraverseExpression(loc->expr); },
            [&](const ast::StructMemberAlignAttribute* align) { TraverseExpression(align->expr); },
            [&](const ast::StructMemberSizeAttribute* size) { TraverseExpression(size->expr); },
            [&](const ast::WorkgroupAttribute* wg) {
                TraverseExpression(wg->x);
                TraverseExpression(wg->y);
                TraverseExpression(wg->z);
            },
            [&](const ast::InternalAttribute* i) {
                for (auto* dep : i->dependencies) {
                    TraverseExpression(dep);
                }
            },
            [&](Default) {
                if (!attr->IsAnyOf<ast::BuiltinAttribute, ast::DiagnosticAttribute,
                                   ast::InterpolateAttribute, ast::InvariantAttribute,
                                   ast::MustUseAttribute, ast::RowMajorAttribute,
                                   ast::StageAttribute, ast::StrideAttribute,
                                   ast::StructMemberOffsetAttribute>()) {
                    TINT_ICE() << "unhandled attribute type: " << attr->TypeInfo().name;
                }
            });
    }

    /// The type of builtin that a symbol could represent.
    enum class BuiltinType {
        /// No builtin matched
        kNone = 0,
        /// Builtin function
        kFunction,
        /// Builtin
        kBuiltin,
        /// Address space
        kAddressSpace,
        /// Texel format
        kTexelFormat,
        /// Access
        kAccess,
    };

    /// BuiltinInfo stores information about the builtin that a symbol represents.
    struct BuiltinInfo {
        /// @returns the builtin value
        template <typename T>
        T Value() const {
            return std::get<T>(value);
        }

        BuiltinType type = BuiltinType::kNone;
        std::variant<std::monostate,
                     wgsl::BuiltinFn,
                     core::BuiltinType,
                     core::AddressSpace,
                     core::TexelFormat,
                     core::Access>
            value = {};
    };

    /// Get the builtin info for a given symbol.
    /// @param symbol the symbol
    /// @returns the builtin info
    DependencyScanner::BuiltinInfo GetBuiltinInfo(Symbol symbol) {
        return builtin_info_map.GetOrAdd(symbol, [&] {
            if (auto builtin_fn = wgsl::ParseBuiltinFn(symbol.NameView());
                builtin_fn != wgsl::BuiltinFn::kNone) {
                return BuiltinInfo{BuiltinType::kFunction, builtin_fn};
            }
            if (auto builtin_ty = core::ParseBuiltinType(symbol.NameView());
                builtin_ty != core::BuiltinType::kUndefined) {
                return BuiltinInfo{BuiltinType::kBuiltin, builtin_ty};
            }
            if (auto addr = core::ParseAddressSpace(symbol.NameView());
                addr != core::AddressSpace::kUndefined) {
                return BuiltinInfo{BuiltinType::kAddressSpace, addr};
            }
            if (auto fmt = core::ParseTexelFormat(symbol.NameView());
                fmt != core::TexelFormat::kUndefined) {
                return BuiltinInfo{BuiltinType::kTexelFormat, fmt};
            }
            if (auto access = core::ParseAccess(symbol.NameView());
                access != core::Access::kUndefined) {
                return BuiltinInfo{BuiltinType::kAccess, access};
            }
            return BuiltinInfo{};
        });
    }

    /// Adds the dependency from @p from to @p to, erroring if @p to cannot be resolved.
    void AddDependency(const ast::Identifier* from, Symbol to) {
        auto* resolved = scope_stack_.Get(to);
        if (!resolved) {
            auto builtin_info = GetBuiltinInfo(to);
            switch (builtin_info.type) {
                case BuiltinType::kNone:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier::UnresolvedIdentifier{to.Name()});
                    break;
                case BuiltinType::kFunction:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(builtin_info.Value<wgsl::BuiltinFn>()));
                    break;
                case BuiltinType::kBuiltin:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(builtin_info.Value<core::BuiltinType>()));
                    break;
                case BuiltinType::kAddressSpace:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(builtin_info.Value<core::AddressSpace>()));
                    break;
                case BuiltinType::kTexelFormat:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(builtin_info.Value<core::TexelFormat>()));
                    break;
                case BuiltinType::kAccess:
                    graph_.resolved_identifiers.Add(
                        from, ResolvedIdentifier(builtin_info.Value<core::Access>()));
                    break;
            }
            return;
        }

        if (auto global = globals_.Get(to); global && (*global)->node == resolved) {
            if (dependency_edges_.Add(DependencyEdge{current_global_, *global},
                                      DependencyInfo{from->source})) {
                current_global_->deps.Push(*global);
            }
        }

        graph_.resolved_identifiers.Add(from, ResolvedIdentifier(resolved));
    }

    using VariableMap = Hashmap<Symbol, const ast::Variable*, 32>;
    const GlobalMap& globals_;
    diag::List& diagnostics_;
    DependencyGraph& graph_;
    DependencyEdges& dependency_edges_;

    ScopeStack<Symbol, const ast::Node*> scope_stack_;
    Global* current_global_ = nullptr;

    Hashmap<Symbol, BuiltinInfo, 64> builtin_info_map;
};

/// The global dependency analysis system
struct DependencyAnalysis {
  public:
    /// Constructor
    DependencyAnalysis(diag::List& diagnostics, DependencyGraph& graph)
        : diagnostics_(diagnostics), graph_(graph) {}

    /// Performs global dependency analysis on the module, emitting any errors to
    /// #diagnostics.
    /// @returns true if analysis found no errors, otherwise false.
    bool Run(const ast::Module& module) {
        // Reserve container memory
        graph_.resolved_identifiers.Reserve(module.GlobalDeclarations().Length());
        sorted_.Reserve(module.GlobalDeclarations().Length());

        // Collect all the named globals from the AST module
        GatherGlobals(module);

        // Traverse the named globals to build the dependency graph
        DetermineDependencies();

        // Sort the globals into dependency order
        SortGlobals();

        // Dump the dependency graph if TINT_DUMP_DEPENDENCY_GRAPH is non-zero
        DumpDependencyGraph();

        graph_.ordered_globals = sorted_.Release();

        return !diagnostics_.ContainsErrors();
    }

  private:
    /// @param node the ast::Node of the global declaration
    /// @returns the symbol of the global declaration node
    /// @note will raise an ICE if the node is not a type, function or variable
    /// declaration
    Symbol SymbolOf(const ast::Node* node) const {
        return Switch(
            node,  //
            [&](const ast::TypeDecl* td) { return td->name->symbol; },
            [&](const ast::Function* func) { return func->name->symbol; },
            [&](const ast::Variable* var) { return var->name->symbol; },
            [&](const ast::DiagnosticDirective*) { return Symbol(); },
            [&](const ast::Enable*) { return Symbol(); },
            [&](const ast::Requires*) { return Symbol(); },
            [&](const ast::ConstAssert*) { return Symbol(); },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// @param node the ast::Node of the global declaration
    /// @returns the name of the global declaration node
    /// @note will raise an ICE if the node is not a type, function or variable
    /// declaration
    std::string NameOf(const ast::Node* node) const { return SymbolOf(node).Name(); }

    /// @param node the ast::Node of the global declaration
    /// @returns a string representation of the global declaration kind
    /// @note will raise an ICE if the node is not a type, function or variable
    /// declaration
    std::string KindOf(const ast::Node* node) {
        return Switch(
            node,                                                     //
            [&](const ast::Struct*) { return "struct"; },             //
            [&](const ast::Alias*) { return "alias"; },               //
            [&](const ast::Function*) { return "function"; },         //
            [&](const ast::Variable* v) { return v->Kind(); },        //
            [&](const ast::ConstAssert*) { return "const_assert"; },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Traverses `module`, collecting all the global declarations and populating
    /// the #globals and #declaration_order fields.
    void GatherGlobals(const ast::Module& module) {
        for (auto* node : module.GlobalDeclarations()) {
            auto* global = allocator_.Create(node);
            if (auto symbol = SymbolOf(node); symbol.IsValid()) {
                globals_.Add(symbol, global);
            }
            declaration_order_.Push(global);
        }
    }

    /// Walks the global declarations, determining the dependencies of each global
    /// and adding these to each global's Global::deps field.
    void DetermineDependencies() {
        DependencyScanner scanner(globals_, diagnostics_, graph_, dependency_edges_);
        for (auto* global : declaration_order_) {
            scanner.Scan(global);
        }
    }

    /// Performs a depth-first traversal of `root`'s dependencies, calling `enter`
    /// as the function decends into each dependency and `exit` when bubbling back
    /// up towards the root.
    /// @param enter is a function with the signature: `bool(Global*)`. The
    /// `enter` function returns true if TraverseDependencies() should traverse
    /// the dependency, otherwise it will be skipped.
    /// @param exit is a function with the signature: `void(Global*)`. The `exit`
    /// function is only called if the corresponding `enter` call returned true.
    template <typename ENTER, typename EXIT>
    void TraverseDependencies(const Global* root, ENTER&& enter, EXIT&& exit) {
        // Entry is a single entry in the traversal stack. Entry points to a
        // dep_idx'th dependency of Entry::global.
        struct Entry {
            const Global* global;  // The parent global
            size_t dep_idx;        // The dependency index in `global->deps`
        };

        if (!enter(root)) {
            return;
        }

        Vector<Entry, 16> stack{Entry{root, 0}};
        while (true) {
            auto& entry = stack.Back();
            // Have we exhausted the dependencies of entry.global?
            if (entry.dep_idx < entry.global->deps.Length()) {
                // No, there's more dependencies to traverse.
                auto& dep = entry.global->deps[entry.dep_idx];
                // Does the caller want to enter this dependency?
                if (enter(dep)) {               // Yes.
                    stack.Push(Entry{dep, 0});  // Enter the dependency.
                } else {
                    entry.dep_idx++;  // No. Skip this node.
                }
            } else {
                // Yes. Time to back up.
                // Exit this global, pop the stack, and if there's another parent node,
                // increment its dependency index, and loop again.
                exit(entry.global);
                stack.Pop();
                if (stack.IsEmpty()) {
                    return;  // All done.
                }
                stack.Back().dep_idx++;
            }
        }
    }

    /// SortGlobals sorts the globals into dependency order, erroring if cyclic
    /// dependencies are found. The sorted dependencies are assigned to #sorted.
    void SortGlobals() {
        if (diagnostics_.ContainsErrors()) {
            return;  // This code assumes there are no undeclared identifiers.
        }

        // Make sure all directives go before any other global declarations.
        for (auto* global : declaration_order_) {
            if (global->node->IsAnyOf<ast::DiagnosticDirective, ast::Enable, ast::Requires>()) {
                sorted_.Add(global->node);
            }
        }

        for (auto* global : declaration_order_) {
            if (global->node->IsAnyOf<ast::DiagnosticDirective, ast::Enable, ast::Requires>()) {
                // Skip directives here, as they are already added.
                continue;
            }
            UniqueVector<const Global*, 8> stack;
            TraverseDependencies(
                global,
                [&](const Global* g) {  // Enter
                    if (!stack.Add(g)) {
                        CyclicDependencyFound(g, stack.Release());
                        return false;
                    }
                    if (sorted_.Contains(g->node)) {
                        // Visited this global already.
                        // stack was pushed, but exit() will not be called when we return
                        // false, so pop here.
                        stack.Pop();
                        return false;
                    }
                    return true;
                },
                [&](const Global* g) {  // Exit. Only called if Enter returned true.
                    sorted_.Add(g->node);
                    stack.Pop();
                });

            sorted_.Add(global->node);

            if (DAWN_UNLIKELY(!stack.IsEmpty())) {
                // Each stack.push() must have a corresponding stack.pop_back().
                TINT_ICE() << "stack not empty after returning from TraverseDependencies()";
            }
        }
    }

    /// DepInfoFor() looks up the global dependency information for the dependency
    /// of global `from` depending on `to`.
    /// @note will raise an ICE if the edge is not found.
    DependencyInfo DepInfoFor(const Global* from, const Global* to) const {
        auto info = dependency_edges_.Get(DependencyEdge{from, to});
        if (DAWN_LIKELY(info)) {
            return *info;
        }
        TINT_ICE() << "failed to find dependency info for edge: '" << NameOf(from->node) << "' -> '"
                   << NameOf(to->node) << "'";
    }

    /// CyclicDependencyFound() emits an error diagnostic for a cyclic dependency.
    /// @param root is the global that starts the cyclic dependency, which must be
    /// found in `stack`.
    /// @param stack is the global dependency stack that contains a loop.
    void CyclicDependencyFound(const Global* root, VectorRef<const Global*> stack) {
        auto& err = AddError(diagnostics_, root->node->source);
        err << "cyclic dependency found: ";
        constexpr size_t kLoopNotStarted = ~0u;
        size_t loop_start = kLoopNotStarted;
        for (size_t i = 0; i < stack.Length(); i++) {
            auto* e = stack[i];
            if (loop_start == kLoopNotStarted && e == root) {
                loop_start = i;
            }
            if (loop_start != kLoopNotStarted) {
                err << "'" << NameOf(e->node) << "' -> ";
            }
        }
        err << "'" << NameOf(root->node) << "'";

        for (size_t i = loop_start; i < stack.Length(); i++) {
            auto* from = stack[i];
            auto* to = (i + 1 < stack.Length()) ? stack[i + 1] : stack[loop_start];
            auto info = DepInfoFor(from, to);
            AddNote(diagnostics_, info.source)
                << KindOf(from->node) + " '" << NameOf(from->node) << "' references "
                << KindOf(to->node) << " '" << NameOf(to->node) << "' here";
        }
    }

    void DumpDependencyGraph() {
#if TINT_DUMP_DEPENDENCY_GRAPH == 0
        if ((true)) {
            return;
        }
#endif  // TINT_DUMP_DEPENDENCY_GRAPH
        printf("=========================\n");
        printf("------ declaration ------ \n");
        for (auto* global : declaration_order_) {
            printf("%s\n", NameOf(global->node).c_str());
        }
        printf("------ dependencies ------ \n");
        for (auto* node : sorted_) {
            auto symbol = SymbolOf(node);
            auto* global = *globals_.Get(symbol);
            printf("%s depends on:\n", symbol.Name().c_str());
            for (auto* dep : global->deps) {
                printf("  %s\n", NameOf(dep->node).c_str());
            }
        }
        printf("=========================\n");
    }

    /// Program diagnostics
    diag::List& diagnostics_;

    /// The resulting dependency graph
    DependencyGraph& graph_;

    /// Allocator of Globals
    BlockAllocator<Global> allocator_;

    /// Global map, keyed by name. Populated by GatherGlobals().
    GlobalMap globals_;

    /// Map of DependencyEdge to DependencyInfo. Populated by DetermineDependencies().
    DependencyEdges dependency_edges_;

    /// Globals in declaration order. Populated by GatherGlobals().
    Vector<Global*, 64> declaration_order_;

    /// Globals in sorted dependency order. Populated by SortGlobals().
    UniqueVector<const ast::Node*, 64> sorted_;
};

}  // namespace

DependencyGraph::DependencyGraph() = default;
DependencyGraph::DependencyGraph(DependencyGraph&&) = default;
DependencyGraph::~DependencyGraph() = default;

bool DependencyGraph::Build(const ast::Module& module,
                            diag::List& diagnostics,
                            DependencyGraph& output) {
    DependencyAnalysis da{diagnostics, output};
    return da.Run(module);
}

std::string ResolvedIdentifier::String() const {
    if (auto* node = Node()) {
        return Switch(
            node,
            [&](const ast::TypeDecl* n) {  //
                return "type '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Var* n) {  //
                return "var '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Let* n) {  //
                return "let '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Const* n) {  //
                return "const '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Override* n) {  //
                return "override '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Function* n) {  //
                return "function '" + n->name->symbol.Name() + "'";
            },
            [&](const ast::Parameter* n) {  //
                return "parameter '" + n->name->symbol.Name() + "'";
            },  //
            TINT_ICE_ON_NO_MATCH);
    }
    if (auto builtin_fn = BuiltinFn(); builtin_fn != wgsl::BuiltinFn::kNone) {
        return "builtin function '" + tint::ToString(builtin_fn) + "'";
    }
    if (auto builtin_ty = BuiltinType(); builtin_ty != core::BuiltinType::kUndefined) {
        return "builtin type '" + tint::ToString(builtin_ty) + "'";
    }
    if (auto access = Access(); access != core::Access::kUndefined) {
        return "access '" + tint::ToString(access) + "'";
    }
    if (auto addr = AddressSpace(); addr != core::AddressSpace::kUndefined) {
        return "address space '" + tint::ToString(addr) + "'";
    }
    if (auto fmt = TexelFormat(); fmt != core::TexelFormat::kUndefined) {
        return "texel format '" + tint::ToString(fmt) + "'";
    }
    if (auto* unresolved = Unresolved()) {
        return "unresolved identifier '" + unresolved->name + "'";
    }

    TINT_UNREACHABLE() << "unhandled ResolvedIdentifier";
}

}  // namespace tint::resolver
