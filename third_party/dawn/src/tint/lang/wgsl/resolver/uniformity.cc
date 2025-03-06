// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/wgsl/resolver/uniformity.h"

#include <limits>
#include <string>
#include <utility>
#include <vector>

#include "src/tint/lang/core/builtin_value.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/dependency_graph.h"
#include "src/tint/lang/wgsl/sem/block_statement.h"
#include "src/tint/lang/wgsl/sem/builtin_fn.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/if_statement.h"
#include "src/tint/lang/wgsl/sem/info.h"
#include "src/tint/lang/wgsl/sem/load.h"
#include "src/tint/lang/wgsl/sem/loop_statement.h"
#include "src/tint/lang/wgsl/sem/statement.h"
#include "src/tint/lang/wgsl/sem/switch_statement.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/lang/wgsl/sem/while_statement.h"
#include "src/tint/utils/containers/map.h"
#include "src/tint/utils/containers/scope_stack.h"
#include "src/tint/utils/containers/unique_vector.h"
#include "src/tint/utils/macros/defer.h"
#include "src/tint/utils/memory/block_allocator.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

// Set to `1` to dump the uniformity graph for each function in graphviz format.
#define TINT_DUMP_UNIFORMITY_GRAPH 0

#if TINT_DUMP_UNIFORMITY_GRAPH
#include <iostream>
#endif

namespace tint::resolver {

namespace {

/// Unwraps `u->expr`'s chain of indirect (*) and address-of (&) expressions, returning the first
/// expression that is neither of these.
/// E.g. If `u` is `*(&(*(&p)))`, returns `p`.
const ast::Expression* UnwrapIndirectAndAddressOfChain(const ast::UnaryOpExpression* u) {
    auto* e = u->expr;
    while (true) {
        auto* unary = e->As<ast::UnaryOpExpression>();
        if (unary &&
            (unary->op == core::UnaryOp::kIndirection || unary->op == core::UnaryOp::kAddressOf)) {
            e = unary->expr;
        } else {
            break;
        }
    }
    return e;
}

/// CallSiteTag describes the uniformity requirements on the call sites of a function.
struct CallSiteTag {
    enum {
        CallSiteRequiredToBeUniform,
        CallSiteNoRestriction,
    } tag;
    wgsl::DiagnosticSeverity severity = wgsl::DiagnosticSeverity::kUndefined;
};

/// FunctionTag describes a functions effects on uniformity.
enum FunctionTag {
    ReturnValueMayBeNonUniform,
    NoRestriction,
};

/// ParameterTag describes the uniformity requirements of values passed to a function parameter.
struct ParameterTag {
    enum {
        ParameterValueRequiredToBeUniform,
        ParameterContentsRequiredToBeUniform,
        ParameterNoRestriction,
    } tag;
    wgsl::DiagnosticSeverity severity = wgsl::DiagnosticSeverity::kUndefined;
};

/// Node represents a node in the graph of control flow and value nodes within the analysis of a
/// single function.
struct Node {
    /// Constructor
    /// @param a the corresponding AST node
    explicit Node(const ast::Node* a) : ast(a) {}

#if TINT_DUMP_UNIFORMITY_GRAPH
    /// The node tag.
    std::string tag;
#endif

    /// Type describes the type of the node, which is used to determine additional diagnostic
    /// information.
    enum Type {
        kRegular,
        kFunctionCallArgumentValue,
        kFunctionCallArgumentContents,
        kFunctionCallPointerArgumentResult,
        kFunctionCallReturnValue,
        kFunctionPointerParameterContents,
    };

    /// The type of the node.
    Type type = kRegular;

    /// `true` if this node represents a potential control flow change.
    bool affects_control_flow = false;

    /// The corresponding AST node, or nullptr.
    const ast::Node* ast = nullptr;

    /// The function call argument index, if applicable.
    uint32_t arg_index = 0xffffffffu;

    /// The set of edges from this node to other nodes in the graph.
    UniqueVector<Node*, 4> edges;

    /// The node that this node was visited from, or nullptr if not visited.
    Node* visited_from = nullptr;

    /// Add an edge to the `to` node.
    /// @param to the destination node
    void AddEdge(Node* to) {
        TINT_ASSERT(to != nullptr);
        edges.Add(to);
    }
};

/// ParameterInfo holds information about the uniformity requirements and effects for a particular
/// function parameter.
struct ParameterInfo {
    /// The semantic node in corresponds to this parameter.
    const sem::Parameter* sem;
    /// The parameter's direct uniformity requirements.
    ParameterTag tag_direct = {ParameterTag::ParameterNoRestriction};
    /// The parameter's uniformity requirements that affect the function return value.
    ParameterTag tag_retval = {ParameterTag::ParameterNoRestriction};
    /// Will be `true` if this function may cause the contents of this pointer parameter to become
    /// non-uniform.
    bool pointer_may_become_non_uniform = false;
    /// The parameters that are required to be uniform for the contents of this pointer parameter to
    /// be uniform at function exit.
    Vector<const sem::Parameter*, 8> ptr_output_source_param_values;
    /// The pointer parameters whose contents are required to be uniform for the contents of this
    /// pointer parameter to be uniform at function exit.
    Vector<const sem::Parameter*, 8> ptr_output_source_param_contents;
    /// The node in the graph that corresponds to this parameter's (immutable) value.
    Node* value;
    /// The node in the graph that corresponds to this pointer parameter's initial contents.
    Node* ptr_input_contents = nullptr;
    /// The node in the graph that corresponds to this pointer parameter's contents on return.
    Node* ptr_output_contents = nullptr;
};

/// FunctionInfo holds information about the uniformity requirements and effects for a particular
/// function, as well as the control flow graph.
struct FunctionInfo {
    /// Constructor
    /// @param func the AST function
    /// @param b the program builder
    FunctionInfo(const ast::Function* func, const ProgramBuilder& b) {
        name = func->name->symbol.Name();
        callsite_tag = {CallSiteTag::CallSiteNoRestriction};
        function_tag = NoRestriction;

        // Create special nodes.
        required_to_be_uniform_error = CreateNode({"RequiredToBeUniform_Error"});
        required_to_be_uniform_warning = CreateNode({"RequiredToBeUniform_Warning"});
        required_to_be_uniform_info = CreateNode({"RequiredToBeUniform_Info"});
        may_be_non_uniform = CreateNode({"MayBeNonUniform"});
        cf_start = CreateNode({"CF_start"});
        if (func->return_type) {
            value_return = CreateNode({"Value_return"});
        }

        // Create nodes for parameters.
        parameters.Resize(func->params.Length());
        for (size_t i = 0; i < func->params.Length(); i++) {
            auto* param = func->params[i];
            auto param_name = param->name->symbol.Name();
            auto* sem = b.Sem().Get(param);
            parameters[i].sem = sem;

            parameters[i].value = CreateNode({"param_", param_name});
            parameters[i].value->ast = param;
            if (sem->Type()->Is<core::type::Pointer>()) {
                // Create extra nodes for a pointer parameter's initial contents and its contents
                // when the function returns.
                parameters[i].ptr_input_contents =
                    CreateNode({"ptrparam_", param_name, "_input_contents"}, param);
                parameters[i].ptr_output_contents =
                    CreateNode({"ptrparam_", param_name, "_output_contents"});
                parameters[i].ptr_input_contents->type = Node::kFunctionPointerParameterContents;
                variables.Set(sem, parameters[i].ptr_input_contents);
                local_var_decls.Add(sem);
            } else {
                variables.Set(sem, parameters[i].value);
            }
        }
    }

    /// The name of the function.
    std::string name;

    /// The call site uniformity requirements.
    CallSiteTag callsite_tag;
    /// The function's uniformity effects.
    FunctionTag function_tag;
    /// The uniformity requirements of the function's parameters.
    Vector<ParameterInfo, 8> parameters;

    /// The control flow graph.
    BlockAllocator<Node> nodes;

    /// Special `RequiredToBeUniform` nodes.
    Node* required_to_be_uniform_error = nullptr;
    Node* required_to_be_uniform_warning = nullptr;
    Node* required_to_be_uniform_info = nullptr;
    /// Special `MayBeNonUniform` node.
    Node* may_be_non_uniform = nullptr;
    /// Special `CF_start` node.
    Node* cf_start = nullptr;
    /// Special `Value_return` node.
    Node* value_return = nullptr;

    /// Map from variables to their value nodes in the graph, scoped with respect to control flow.
    ScopeStack<const sem::Variable*, Node*> variables;

    /// The set of mutable variables declared in the function that are in scope at any given point
    /// in the analysis. This includes the contents of parameters to the function that are pointers.
    /// This is used by the analysis for if statements and loops to know which variables need extra
    /// nodes to capture their state when entering/exiting those constructs.
    Hashset<const sem::Variable*, 8> local_var_decls;

    /// The set of partial pointer variables - pointers that point to a subobject (into an array or
    /// struct).
    Hashset<const sem::Variable*, 4> partial_ptrs;

    /// LoopSwitchInfo tracks information about the value of variables for a control flow construct.
    struct LoopSwitchInfo {
        /// The type of this control flow construct.
        std::string type;
        /// The input values for local variables at the start of this construct.
        Hashmap<const sem::Variable*, Node*, 4> var_in_nodes;
        /// The exit values for local variables at the end of this construct.
        Hashmap<const sem::Variable*, Node*, 4> var_exit_nodes;
    };

    /// @returns the RequiredToBeUniform node that corresponds to `severity`
    Node* RequiredToBeUniform(wgsl::DiagnosticSeverity severity) {
        switch (severity) {
            case wgsl::DiagnosticSeverity::kError:
                return required_to_be_uniform_error;
            case wgsl::DiagnosticSeverity::kWarning:
                return required_to_be_uniform_warning;
            case wgsl::DiagnosticSeverity::kInfo:
                return required_to_be_uniform_info;
            default:
                TINT_UNREACHABLE() << "unhandled severity";
        }
    }

    /// @returns a LoopSwitchInfo for the given statement, allocating the LoopSwitchInfo if this is
    /// the first call with the given statement.
    LoopSwitchInfo& LoopSwitchInfoFor(const sem::Statement* stmt) {
        return *loop_switch_infos.GetOrAdd(stmt,
                                           [&] { return loop_switch_info_allocator.Create(); });
    }

    /// Disassociates the LoopSwitchInfo for the given statement.
    void RemoveLoopSwitchInfoFor(const sem::Statement* stmt) { loop_switch_infos.Remove(stmt); }

    /// Create a new node.
    /// @param tag_list a string list that will be used to identify the node for debugging purposes
    /// @param ast the optional AST node that this node corresponds to
    /// @returns the new node
    Node* CreateNode([[maybe_unused]] std::initializer_list<std::string_view> tag_list,
                     const ast::Node* ast = nullptr) {
        auto* node = nodes.Create(ast);

#if TINT_DUMP_UNIFORMITY_GRAPH
        // Make the tag unique and set it.
        // This only matters if we're dumping the graph.
        std::string tag = "";
        for (auto& t : tag_list) {
            tag += t;
        }
        std::string unique_tag = tag;
        int suffix = 0;
        while (tags_.Contains(unique_tag)) {
            unique_tag = tag + "_$" + std::to_string(++suffix);
        }
        tags_.Add(unique_tag);
        node->tag = name + "." + unique_tag;
#endif

        return node;
    }

    /// Reset the visited status of every node in the graph.
    void ResetVisited() {
        for (auto* node : nodes.Objects()) {
            node->visited_from = nullptr;
        }
    }

  private:
    /// A list of tags that have already been used within the current function.
    Hashset<std::string, 8> tags_;

    /// Map from control flow statements to the corresponding LoopSwitchInfo structure.
    Hashmap<const sem::Statement*, LoopSwitchInfo*, 8> loop_switch_infos;

    /// Allocator of LoopSwitchInfos
    BlockAllocator<LoopSwitchInfo> loop_switch_info_allocator;
};

/// UniformityGraph is used to analyze the uniformity requirements and effects of functions in a
/// module.
class UniformityGraph {
  public:
    /// Constructor.
    /// @param builder the program to analyze
    explicit UniformityGraph(ProgramBuilder& builder)
        : b(builder), sem_(b.Sem()), diagnostics_(builder.Diagnostics()) {}

    /// Destructor.
    ~UniformityGraph() {}

    /// Build and analyze the graph to determine whether the program satisfies the uniformity
    /// constraints of WGSL.
    /// @param dependency_graph the dependency-ordered module-scope declarations
    /// @returns true if all uniformity constraints are satisfied, otherise false
    bool Build(const DependencyGraph& dependency_graph) {
#if TINT_DUMP_UNIFORMITY_GRAPH
        std::cout << "digraph G {\n";
        std::cout << "rankdir=BT\n";
#endif

        // Process all functions in the module.
        bool success = true;
        for (auto* decl : dependency_graph.ordered_globals) {
            if (auto* func = decl->As<ast::Function>()) {
                if (!ProcessFunction(func)) {
                    success = false;
                    break;
                }
            }
        }

#if TINT_DUMP_UNIFORMITY_GRAPH
        std::cout << "\n}\n";
#endif

        return success;
    }

  private:
    const ProgramBuilder& b;
    const sem::Info& sem_;
    diag::List& diagnostics_;

    /// Map of analyzed function results.
    Hashmap<const ast::Function*, FunctionInfo, 8> functions_;

    /// The function currently being analyzed.
    FunctionInfo* current_function_;

    /// Create a new node.
    /// @param tag_list a string list that will be used to identify the node for debugging purposes
    /// @param ast the optional AST node that this node corresponds to
    /// @returns the new node
    inline Node* CreateNode(std::initializer_list<std::string_view> tag_list,
                            const ast::Node* ast = nullptr) {
        return current_function_->CreateNode(std::move(tag_list), ast);
    }

    /// Get the symbol name of an AST expression.
    /// @param expr the expression to get the symbol name of
    /// @returns the symbol name
    inline std::string NameFor(const ast::IdentifierExpression* expr) {
        return expr->identifier->symbol.Name();
    }

    /// @param var the variable to get the name of
    /// @returns the name of the variable @p var
    inline std::string NameFor(const ast::Variable* var) { return var->name->symbol.Name(); }

    /// @param var the variable to get the name of
    /// @returns the name of the variable @p var
    inline std::string NameFor(const sem::Variable* var) { return NameFor(var->Declaration()); }

    /// @param fn the function to get the name of
    /// @returns the name of the function @p fn
    inline std::string NameFor(const sem::Function* fn) {
        return fn->Declaration()->name->symbol.Name();
    }

    /// Process a function.
    /// @param func the function to process
    /// @returns true if there are no uniformity issues, false otherwise
    bool ProcessFunction(const ast::Function* func) {
        current_function_ = &functions_.Add(func, FunctionInfo(func, b)).value;

        // Process function body.
        if (func->body) {
            ProcessStatement(current_function_->cf_start, func->body);
        }

#if TINT_DUMP_UNIFORMITY_GRAPH
        // Dump the graph for this function as a subgraph.
        std::cout << "\nsubgraph cluster_" << current_function_->name << " {\n";
        std::cout << "  label=" << current_function_->name << ";";
        for (auto* node : current_function_->nodes.Objects()) {
            std::cout << "\n  \"" << node->tag << "\";";
            for (auto* edge : node->edges) {
                std::cout << "\n  \"" << node->tag << "\" -> \"" << edge->tag << "\";";
            }
        }
        std::cout << "\n}\n";
#endif

        /// Helper to generate a tag for the uniformity requirements of the parameter at `index`.
        auto get_param_tag = [&](UniqueVector<Node*, 4>& reachable, size_t index) {
            auto* param = sem_.Get(func->params[index]);
            auto& param_info = current_function_->parameters[index];
            if (param->Type()->Is<core::type::Pointer>()) {
                // For pointers, we distinguish between requiring uniformity of the contents versus
                // the pointer itself.
                if (reachable.Contains(param_info.ptr_input_contents)) {
                    return ParameterTag::ParameterContentsRequiredToBeUniform;
                } else if (reachable.Contains(param_info.value)) {
                    return ParameterTag::ParameterValueRequiredToBeUniform;
                }
            } else if (reachable.Contains(current_function_->variables.Get(param))) {
                // For non-pointers, the requirement is always on the value.
                return ParameterTag::ParameterValueRequiredToBeUniform;
            }
            return ParameterTag::ParameterNoRestriction;
        };

        // Look at which nodes are reachable from "RequiredToBeUniform".
        {
            UniqueVector<Node*, 4> reachable;
            auto traverse = [&](wgsl::DiagnosticSeverity severity) {
                Traverse(current_function_->RequiredToBeUniform(severity), &reachable);
                if (reachable.Contains(current_function_->may_be_non_uniform)) {
                    MakeError(*current_function_, current_function_->may_be_non_uniform, severity);
                    return false;
                }
                if (reachable.Contains(current_function_->cf_start)) {
                    if (current_function_->callsite_tag.tag == CallSiteTag::CallSiteNoRestriction) {
                        current_function_->callsite_tag = {CallSiteTag::CallSiteRequiredToBeUniform,
                                                           severity};
                    }
                }

                // Set the tags to capture the direct uniformity requirements of each parameter.
                for (size_t i = 0; i < func->params.Length(); i++) {
                    if (current_function_->parameters[i].tag_direct.tag ==
                        ParameterTag::ParameterNoRestriction) {
                        current_function_->parameters[i].tag_direct = {get_param_tag(reachable, i),
                                                                       severity};
                    }
                }
                return true;
            };
            if (!traverse(wgsl::DiagnosticSeverity::kError)) {
                return false;
            } else {
                if (traverse(wgsl::DiagnosticSeverity::kWarning)) {
                    traverse(wgsl::DiagnosticSeverity::kInfo);
                }
            }
        }

        // If "Value_return" exists, look at which nodes are reachable from it.
        if (current_function_->value_return) {
            current_function_->ResetVisited();

            UniqueVector<Node*, 4> reachable;
            Traverse(current_function_->value_return, &reachable);
            if (reachable.Contains(current_function_->may_be_non_uniform)) {
                current_function_->function_tag = ReturnValueMayBeNonUniform;
            }

            // Set the tags to capture the uniformity requirements of each parameter with respect to
            // the function return value.
            for (size_t i = 0; i < func->params.Length(); i++) {
                current_function_->parameters[i].tag_retval = {get_param_tag(reachable, i)};
            }
        }

        // Traverse the graph for each pointer parameter.
        for (size_t i = 0; i < func->params.Length(); i++) {
            auto& param_info = current_function_->parameters[i];
            if (param_info.ptr_output_contents == nullptr) {
                continue;
            }

            // Reset "visited" state for all nodes.
            current_function_->ResetVisited();

            UniqueVector<Node*, 4> reachable;
            Traverse(param_info.ptr_output_contents, &reachable);
            if (reachable.Contains(current_function_->may_be_non_uniform)) {
                param_info.pointer_may_become_non_uniform = true;
            }

            // Check every parameter to see if it feeds into this parameter's output value.
            // This includes checking this parameter (as it may feed into its own output value), so
            // we do not skip the `i==j` case.
            for (size_t j = 0; j < func->params.Length(); j++) {
                auto tag = get_param_tag(reachable, j);
                auto* source_param = sem_.Get(func->params[j]);
                if (tag == ParameterTag::ParameterContentsRequiredToBeUniform) {
                    param_info.ptr_output_source_param_contents.Push(source_param);
                } else if (tag == ParameterTag::ParameterValueRequiredToBeUniform) {
                    param_info.ptr_output_source_param_values.Push(source_param);
                }
            }
        }

        return true;
    }

    /// Process a statement, returning the new control flow node.
    /// @param cf the input control flow node
    /// @param stmt the statement to process d
    /// @returns the new control flow node
    Node* ProcessStatement(Node* cf, const ast::Statement* stmt) {
        return Switch(
            stmt,

            [&](const ast::AssignmentStatement* a) {
                if (a->lhs->Is<ast::PhonyExpression>()) {
                    auto [cf_r, _] = ProcessExpression(cf, a->rhs);
                    return cf_r;
                }
                auto [cf_l, v_l, ident] = ProcessLValueExpression(cf, a->lhs);
                auto [cf_r, v_r] = ProcessExpression(cf_l, a->rhs);
                v_l->AddEdge(v_r);

                // Update the variable node for the LHS variable.
                current_function_->variables.Set(ident, v_l);

                return cf_r;
            },

            [&](const ast::BlockStatement* block) {
                Hashmap<const sem::Variable*, Node*, 4> scoped_assignments;
                auto* sem = sem_.Get(block);
                {
                    // Push a new scope for variable assignments in the block.
                    current_function_->variables.Push();
                    TINT_DEFER(current_function_->variables.Pop());

                    for (auto* s : block->statements) {
                        cf = ProcessStatement(cf, s);
                        if (!sem_.Get(s)->Behaviors().Contains(sem::Behavior::kNext)) {
                            break;
                        }
                    }

                    auto* parent = sem->Parent();
                    auto* loop = parent ? parent->As<sem::LoopStatement>() : nullptr;
                    if (loop) {
                        // We've reached the end of a loop body. If there is a continuing block,
                        // process it before ending the block so that any variables declared in the
                        // loop body are visible to the continuing block.
                        if (auto* continuing =
                                loop->Declaration()->As<ast::LoopStatement>()->continuing) {
                            auto& loop_body_behavior = sem->Behaviors();
                            if (loop_body_behavior.Contains(sem::Behavior::kNext) ||
                                loop_body_behavior.Contains(sem::Behavior::kContinue)) {
                                cf = ProcessStatement(cf, continuing);
                            }
                        }
                    }

                    if (sem_.Get<sem::FunctionBlockStatement>(block)) {
                        // We've reached the end of the function body.
                        // Add edges from pointer parameter outputs to their current value.
                        for (auto& param : current_function_->parameters) {
                            if (param.ptr_output_contents) {
                                param.ptr_output_contents->AddEdge(
                                    current_function_->variables.Get(param.sem));
                            }
                        }
                    }

                    scoped_assignments = std::move(current_function_->variables.Top());
                }

                // Propagate all variables assignments to the containing scope if the behavior is
                // 'Next'.
                auto& behaviors = sem->Behaviors();
                if (behaviors.Contains(sem::Behavior::kNext)) {
                    for (auto& var : scoped_assignments) {
                        current_function_->variables.Set(var.key, var.value);
                    }
                }

                // Remove any variables declared in this scope from the set of in-scope variables.
                for (auto decl : sem->Decls()) {
                    current_function_->local_var_decls.Remove(decl.value.variable);
                }

                return cf;
            },

            [&](const ast::BreakStatement* brk) {
                // Find the loop or switch statement that we are in.
                auto* parent = sem_.Get(brk)
                                   ->FindFirstParent<sem::SwitchStatement, sem::LoopStatement,
                                                     sem::ForLoopStatement, sem::WhileStatement>();

                auto& info = current_function_->LoopSwitchInfoFor(parent);

                // Propagate variable values to the loop/switch exit nodes.
                for (auto& var : current_function_->local_var_decls) {
                    // Skip variables that were declared inside this loop/switch.
                    if (auto* lv = var->As<sem::LocalVariable>();
                        lv &&
                        lv->Statement()->FindFirstParent([&](auto* s) { return s == parent; })) {
                        continue;
                    }

                    // Add an edge from the variable exit node to its value at this point.
                    auto* exit_node = info.var_exit_nodes.GetOrAdd(var, [&] {
                        auto name = NameFor(var);
                        return CreateNode({name, "_value_", info.type, "_exit"});
                    });
                    exit_node->AddEdge(current_function_->variables.Get(var));
                }

                return cf;
            },

            [&](const ast::BreakIfStatement* brk) {
                // This works very similar to the IfStatement uniformity below, execpt instead of
                // processing the body, we directly inline the BreakStatement uniformity from
                // above.
                auto* sem = sem_.Get(brk);

                auto [_, v_cond] = ProcessExpression(cf, brk->condition);

                // Add a diagnostic node to capture the control flow change.
                auto* v = CreateNode({"break_if_stmt"}, brk);
                v->affects_control_flow = true;
                v->AddEdge(v_cond);

                {
                    auto* parent = sem->FindFirstParent<sem::LoopStatement>();
                    auto& info = current_function_->LoopSwitchInfoFor(parent);

                    // Propagate variable values to the loop exit nodes.
                    for (auto& var : current_function_->local_var_decls) {
                        // Skip variables that were declared inside this loop.
                        if (auto* lv = var->As<sem::LocalVariable>();
                            lv && lv->Statement()->FindFirstParent(
                                      [&](auto* s) { return s == parent; })) {
                            continue;
                        }

                        // Add an edge from the variable exit node to its value at this point.
                        auto* exit_node = info.var_exit_nodes.GetOrAdd(var, [&] {
                            auto name = NameFor(var);
                            return CreateNode({name, "_value_", info.type, "_exit"});
                        });

                        exit_node->AddEdge(current_function_->variables.Get(var));
                    }
                }

                if (sem->Behaviors() != sem::Behaviors{sem::Behavior::kNext}) {
                    auto* cf_end = CreateNode({"break_if_CFend"});
                    cf_end->AddEdge(v);
                    return cf_end;
                }
                return cf;
            },

            [&](const ast::CallStatement* c) {
                auto [cf1, _] = ProcessCall(cf, c->expr);
                return cf1;
            },

            [&](const ast::CompoundAssignmentStatement* c) {
                // The compound assignment statement `a += b` is equivalent to:
                //   let p = &a;
                //   *p = *p + b;

                // Evaluate the LHS.
                auto [cf1, l1, ident] = ProcessLValueExpression(cf, c->lhs);

                // Get the current value loaded from the LHS reference before evaluating the RHS.
                auto* lhs_load = current_function_->variables.Get(ident);

                // Evaluate the RHS.
                auto [cf2, v2] = ProcessExpression(cf1, c->rhs);

                // Create a node for the resulting value.
                auto* result = CreateNode({"binary_expr_result"});
                result->AddEdge(v2);
                if (lhs_load) {
                    result->AddEdge(lhs_load);
                }

                // Update the variable node for the LHS variable.
                l1->AddEdge(result);
                current_function_->variables.Set(ident, l1);

                return cf2;
            },

            [&](const ast::ContinueStatement* c) {
                // Find the loop statement that we are in.
                auto* parent = sem_.Get(c)
                                   ->FindFirstParent<sem::LoopStatement, sem::ForLoopStatement,
                                                     sem::WhileStatement>();
                auto& info = current_function_->LoopSwitchInfoFor(parent);

                // Propagate assignments to the loop input nodes.
                for (auto v : info.var_in_nodes) {
                    auto* in_node = v.value;
                    auto* out_node = current_function_->variables.Get(v.key);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }
                return cf;
            },

            [&](const ast::DiscardStatement*) { return cf; },

            [&](const ast::ForLoopStatement* f) {
                auto* sem_loop = sem_.Get(f);
                auto* cfx = CreateNode({"loop_start"});

                // Insert the initializer before the loop.
                auto* cf_init = cf;
                if (f->initializer) {
                    cf_init = ProcessStatement(cf, f->initializer);
                }
                auto* cf_start = cf_init;

                auto& info = current_function_->LoopSwitchInfoFor(sem_loop);
                info.type = "forloop";

                // Create input nodes for any variables declared before this loop.
                for (auto& v : current_function_->local_var_decls) {
                    auto* in_node = CreateNode({NameFor(v), "_value_forloop_in"});
                    in_node->AddEdge(current_function_->variables.Get(v));
                    info.var_in_nodes.Replace(v, in_node);
                    current_function_->variables.Set(v, in_node);
                }

                // Insert the condition at the start of the loop body.
                if (f->condition) {
                    auto [cf_cond, v] = ProcessExpression(cfx, f->condition);
                    auto* cf_condition_end = CreateNode({"for_condition_CFend"}, f);
                    cf_condition_end->affects_control_flow = true;
                    cf_condition_end->AddEdge(v);
                    cf_start = cf_condition_end;

                    // Propagate assignments to the loop exit nodes.
                    for (auto& var : current_function_->local_var_decls) {
                        auto* exit_node = info.var_exit_nodes.GetOrAdd(var, [&] {
                            auto name = NameFor(var);
                            return CreateNode({name, "_value_", info.type, "_exit"});
                        });
                        exit_node->AddEdge(current_function_->variables.Get(var));
                    }
                }
                auto* cf1 = ProcessStatement(cf_start, f->body);

                // Insert the continuing statement at the end of the loop body.
                if (f->continuing) {
                    auto* cf2 = ProcessStatement(cf1, f->continuing);
                    cfx->AddEdge(cf2);
                } else {
                    cfx->AddEdge(cf1);
                }
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto& v : info.var_in_nodes) {
                    auto* in_node = v.value;
                    auto* out_node = current_function_->variables.Get(v.key);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto& v : info.var_exit_nodes) {
                    current_function_->variables.Set(v.key, v.value);
                }

                if (f->initializer) {
                    // Remove variables declared in the for-loop initializer from the current scope.
                    if (auto* decl = f->initializer->As<ast::VariableDeclStatement>()) {
                        current_function_->local_var_decls.Remove(sem_.Get(decl->variable));
                    }
                }

                current_function_->RemoveLoopSwitchInfoFor(sem_loop);

                if (sem_loop->Behaviors() == sem::Behaviors{sem::Behavior::kNext}) {
                    return cf;
                } else {
                    return cfx;
                }
            },

            [&](const ast::WhileStatement* w) {
                auto* sem_loop = sem_.Get(w);
                auto* cfx = CreateNode({"loop_start"});

                auto* cf_start = cf;

                auto& info = current_function_->LoopSwitchInfoFor(sem_loop);
                info.type = "whileloop";

                // Create input nodes for any variables declared before this loop.
                for (auto& v : current_function_->local_var_decls) {
                    auto* in_node = CreateNode({NameFor(v), "_value_forloop_in"});
                    in_node->AddEdge(current_function_->variables.Get(v));
                    info.var_in_nodes.Replace(v, in_node);
                    current_function_->variables.Set(v, in_node);
                }

                // Insert the condition at the start of the loop body.
                {
                    auto [cf_cond, v] = ProcessExpression(cfx, w->condition);
                    auto* cf_condition_end = CreateNode({"while_condition_CFend"}, w);
                    cf_condition_end->affects_control_flow = true;
                    cf_condition_end->AddEdge(v);
                    cf_start = cf_condition_end;
                }

                // Propagate assignments to the loop exit nodes.
                for (auto& var : current_function_->local_var_decls) {
                    auto* exit_node = info.var_exit_nodes.GetOrAdd(var, [&] {
                        auto name = NameFor(var);
                        return CreateNode({name, "_value_", info.type, "_exit"});
                    });
                    exit_node->AddEdge(current_function_->variables.Get(var));
                }
                auto* cf1 = ProcessStatement(cf_start, w->body);
                cfx->AddEdge(cf1);
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto v : info.var_in_nodes) {
                    auto* in_node = v.value;
                    auto* out_node = current_function_->variables.Get(v.key);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto v : info.var_exit_nodes) {
                    current_function_->variables.Set(v.key, v.value);
                }

                current_function_->RemoveLoopSwitchInfoFor(sem_loop);

                if (sem_loop->Behaviors() == sem::Behaviors{sem::Behavior::kNext}) {
                    return cf;
                } else {
                    return cfx;
                }
            },

            [&](const ast::IfStatement* i) {
                auto* sem_if = sem_.Get(i);
                auto [_, v_cond] = ProcessExpression(cf, i->condition);

                // Add a diagnostic node to capture the control flow change.
                auto* v = CreateNode({"if_stmt"}, i);
                v->affects_control_flow = true;
                v->AddEdge(v_cond);

                Hashmap<const sem::Variable*, Node*, 4> true_vars;
                Hashmap<const sem::Variable*, Node*, 4> false_vars;

                // Helper to process a statement with a new scope for variable assignments.
                // Populates `assigned_vars` with new nodes for any variables that are assigned in
                // this statement.
                auto process_in_scope =
                    [&](Node* cf_in, const ast::Statement* s,
                        Hashmap<const sem::Variable*, Node*, 4>& assigned_vars) {
                        // Push a new scope for variable assignments.
                        current_function_->variables.Push();

                        // Process the statement.
                        auto* cf_out = ProcessStatement(cf_in, s);

                        assigned_vars = current_function_->variables.Top();

                        // Pop the scope and return.
                        current_function_->variables.Pop();
                        return cf_out;
                    };

                auto* cf1 = process_in_scope(v, i->body, true_vars);

                bool true_has_next = sem_.Get(i->body)->Behaviors().Contains(sem::Behavior::kNext);
                bool false_has_next = true;

                Node* cf2 = nullptr;
                if (i->else_statement) {
                    cf2 = process_in_scope(v, i->else_statement, false_vars);

                    false_has_next =
                        sem_.Get(i->else_statement)->Behaviors().Contains(sem::Behavior::kNext);
                }

                // Update values for any variables assigned in the if or else blocks.
                for (auto& var : current_function_->local_var_decls) {
                    // Skip variables not assigned in either block.
                    if (!true_vars.Contains(var) && !false_vars.Contains(var)) {
                        continue;
                    }

                    // Create an exit node for the variable.
                    auto* out_node = CreateNode({NameFor(var), "_value_if_exit"});

                    // Add edges to the assigned value or the initial value.
                    // Only add edges if the behavior for that block contains 'Next'.
                    if (true_has_next) {
                        if (true_vars.Contains(var)) {
                            out_node->AddEdge(*true_vars.Get(var));
                        } else {
                            out_node->AddEdge(current_function_->variables.Get(var));
                        }
                    }
                    if (false_has_next) {
                        if (false_vars.Contains(var)) {
                            out_node->AddEdge(*false_vars.Get(var));
                        } else {
                            out_node->AddEdge(current_function_->variables.Get(var));
                        }
                    }

                    current_function_->variables.Set(var, out_node);
                }

                if (sem_if->Behaviors() != sem::Behaviors{sem::Behavior::kNext}) {
                    auto* cf_end = CreateNode({"if_CFend"});
                    cf_end->AddEdge(cf1);
                    if (cf2) {
                        cf_end->AddEdge(cf2);
                    }
                    return cf_end;
                }
                return cf;
            },

            [&](const ast::IncrementDecrementStatement* i) {
                // The increment/decrement statement `i++` is equivalent to `i = i + 1`.

                // Evaluate the LHS.
                auto [cf1, l1, ident] = ProcessLValueExpression(cf, i->lhs);

                // Get the current value loaded from the LHS reference.
                auto* lhs_load = current_function_->variables.Get(ident);

                // Create a node for the resulting value.
                auto* result = CreateNode({"incdec_result"});
                result->AddEdge(cf1);
                if (lhs_load) {
                    result->AddEdge(lhs_load);
                }

                // Update the variable node for the LHS variable.
                l1->AddEdge(result);
                current_function_->variables.Set(ident, l1);

                return cf1;
            },

            [&](const ast::LoopStatement* l) {
                auto* sem_loop = sem_.Get(l);
                auto* cfx = CreateNode({"loop_start"});

                auto& info = current_function_->LoopSwitchInfoFor(sem_loop);
                info.type = "loop";

                // Create input nodes for any variables declared before this loop.
                for (auto& v : current_function_->local_var_decls) {
                    auto name = NameFor(v);
                    auto* in_node = CreateNode({name, "_value_loop_in"}, v->Declaration());
                    in_node->AddEdge(current_function_->variables.Get(v));
                    info.var_in_nodes.Replace(v, in_node);
                    current_function_->variables.Set(v, in_node);
                }

                // Note: The continuing block is processed as a special case at the end of
                // processing the loop body BlockStatement. This is so that variable declarations
                // inside the loop body are visible to the continuing statement.
                auto* cf1 = ProcessStatement(cfx, l->body);
                cfx->AddEdge(cf1);
                cfx->AddEdge(cf);

                // Add edges from variable loop input nodes to their values at the end of the loop.
                for (auto v : info.var_in_nodes) {
                    auto* in_node = v.value;
                    auto* out_node = current_function_->variables.Get(v.key);
                    if (out_node != in_node) {
                        in_node->AddEdge(out_node);
                    }
                }

                // Set each variable's exit node as its value in the outer scope.
                for (auto v : info.var_exit_nodes) {
                    current_function_->variables.Set(v.key, v.value);
                }

                current_function_->RemoveLoopSwitchInfoFor(sem_loop);

                if (sem_loop->Behaviors() == sem::Behaviors{sem::Behavior::kNext}) {
                    return cf;
                } else {
                    return cfx;
                }
            },

            [&](const ast::ReturnStatement* r) {
                Node* cf_ret;
                if (r->value) {
                    auto [cf1, v] = ProcessExpression(cf, r->value);
                    current_function_->value_return->AddEdge(v);
                    cf_ret = cf1;
                } else {
                    TINT_ASSERT(cf != nullptr);
                    cf_ret = cf;
                }

                // Add edges from each pointer parameter output to its current value.
                for (auto& param : current_function_->parameters) {
                    if (param.ptr_output_contents) {
                        param.ptr_output_contents->AddEdge(
                            current_function_->variables.Get(param.sem));
                    }
                }

                return cf_ret;
            },

            [&](const ast::SwitchStatement* s) {
                auto* sem_switch = sem_.Get(s);
                auto [cfx, v_cond] = ProcessExpression(cf, s->condition);

                // Add a diagnostic node to capture the control flow change.
                auto* v = CreateNode({"switch_stmt"}, s);
                v->affects_control_flow = true;
                v->AddEdge(v_cond);

                Node* cf_end = nullptr;
                if (sem_switch->Behaviors() != sem::Behaviors{sem::Behavior::kNext}) {
                    cf_end = CreateNode({"switch_CFend"});
                }

                auto& info = current_function_->LoopSwitchInfoFor(sem_switch);
                info.type = "switch";

                auto* cf_n = v;
                for (auto* c : s->body) {
                    auto* sem_case = sem_.Get(c);

                    current_function_->variables.Push();
                    cf_n = ProcessStatement(v, c->body);

                    if (cf_end) {
                        cf_end->AddEdge(cf_n);
                    }

                    if (sem_case->Behaviors().Contains(sem::Behavior::kNext)) {
                        // Propagate variable values to the switch exit nodes.
                        for (auto& var : current_function_->local_var_decls) {
                            // Skip variables that were declared inside the switch.
                            if (auto* lv = var->As<sem::LocalVariable>();
                                lv && lv->Statement()->FindFirstParent(
                                          [&](auto* st) { return st == sem_switch; })) {
                                continue;
                            }

                            // Add an edge from the variable exit node to its new value.
                            auto* exit_node = info.var_exit_nodes.GetOrAdd(var, [&] {
                                auto name = NameFor(var);
                                return CreateNode({name, "_value_", info.type, "_exit"});
                            });
                            exit_node->AddEdge(current_function_->variables.Get(var));
                        }
                    }
                    current_function_->variables.Pop();
                }

                // Update nodes for any variables assigned in the switch statement.
                for (auto var : info.var_exit_nodes) {
                    current_function_->variables.Set(var.key, var.value);
                }

                return cf_end ? cf_end : cf;
            },

            [&](const ast::VariableDeclStatement* decl) {
                Node* node;
                auto* sem_var = sem_.Get(decl->variable);
                if (decl->variable->initializer) {
                    auto [cf1, v] = ProcessExpression(cf, decl->variable->initializer);
                    cf = cf1;
                    node = v;

                    // Store if lhs is a partial pointer
                    if (sem_var->Type()->Is<core::type::Pointer>()) {
                        auto* init = sem_.Get(decl->variable->initializer);
                        if (auto* unary_init = init->Declaration()->As<ast::UnaryOpExpression>()) {
                            auto* e = UnwrapIndirectAndAddressOfChain(unary_init);
                            if (e->Is<ast::AccessorExpression>()) {
                                current_function_->partial_ptrs.Add(sem_var);
                            }
                        }
                    }
                } else {
                    node = cf;
                }
                current_function_->variables.Set(sem_var, node);

                if (decl->variable->Is<ast::Var>()) {
                    current_function_->local_var_decls.Add(
                        sem_.Get<sem::LocalVariable>(decl->variable));
                }

                return cf;
            },

            [&](const ast::ConstAssert*) {
                return cf;  // No impact on uniformity
            },

            TINT_ICE_ON_NO_MATCH);
    }

    /// Process an identifier expression.
    /// @param cf the input control flow node
    /// @param ident the identifier expression to process
    /// @param load_rule true if the load rule is being invoked on this identifier
    /// @returns a pair of (control flow node, value node)
    std::pair<Node*, Node*> ProcessIdentExpression(Node* cf,
                                                   const ast::IdentifierExpression* ident,
                                                   bool load_rule = false) {
        // Helper to check if the entry point attribute of `obj` indicates non-uniformity.
        auto has_nonuniform_entry_point_attribute = [&](auto* obj, auto* entry_point) {
            // Only the num_workgroups and workgroup_id builtins, and subgroup_size builtin used in
            // compute stage are uniform.
            if (auto* builtin_attr = ast::GetAttribute<ast::BuiltinAttribute>(obj->attributes)) {
                auto builtin = builtin_attr->builtin;
                if (builtin == core::BuiltinValue::kNumWorkgroups ||
                    builtin == core::BuiltinValue::kWorkgroupId) {
                    return false;
                }
                if (builtin == core::BuiltinValue::kSubgroupSize) {
                    if (entry_point->PipelineStage() == ast::PipelineStage::kCompute) {
                        // Subgroup size is uniform in compute.
                        return false;
                    } else {
                        // Currently the only other allowed usage for subgroup_size is in fragment.
                        TINT_ASSERT(entry_point->PipelineStage() == ast::PipelineStage::kFragment);
                        // Subgroup size is considered to be varying for fragment.
                        return true;
                    }
                }
            }
            return true;
        };

        auto* node = CreateNode({NameFor(ident), "_ident_expr"}, ident);
        auto* sem_ident = sem_.GetVal(ident);
        TINT_ASSERT(sem_ident);
        auto* var_user = sem_ident->Unwrap()->As<sem::VariableUser>();
        auto* sem = var_user->Variable();
        return Switch(
            sem,

            [&](const sem::Parameter* param) {
                auto* user_func = param->Owner()->As<sem::Function>();
                if (user_func && user_func->Declaration()->IsEntryPoint()) {
                    if (auto* str = param->Type()->As<sem::Struct>()) {
                        // We consider the whole struct to be non-uniform if any one of its members
                        // is non-uniform.
                        bool uniform = true;
                        for (auto* member : str->Members()) {
                            if (has_nonuniform_entry_point_attribute(member->Declaration(),
                                                                     user_func->Declaration())) {
                                uniform = false;
                            }
                        }
                        node->AddEdge(uniform ? cf : current_function_->may_be_non_uniform);
                        return std::make_pair(cf, node);
                    } else {
                        if (has_nonuniform_entry_point_attribute(param->Declaration(),
                                                                 user_func->Declaration())) {
                            node->AddEdge(current_function_->may_be_non_uniform);
                        } else {
                            node->AddEdge(cf);
                        }
                        return std::make_pair(cf, node);
                    }
                } else {
                    node->AddEdge(cf);

                    auto* current_value = current_function_->variables.Get(param);
                    if (auto* ptr = param->Type()->As<core::type::Pointer>()) {
                        if (load_rule) {
                            if (ptr->AddressSpace() == core::AddressSpace::kFunction ||
                                ptr->Access() == core::Access::kRead) {
                                // We are loading from a pointer to a function-scope variable or an
                                // immutable module-scope variable, so add an edge to its contents.
                                node->AddEdge(current_value);
                            } else {
                                // We are loading from a pointer to a mutable module-scope variable,
                                // which always has non-uniform contents.
                                node->AddEdge(current_function_->may_be_non_uniform);
                            }
                        } else {
                            // This is a pointer parameter that we are not loading from, so add an
                            // edge to the pointer value itself.
                            node->AddEdge(current_function_->parameters[param->Index()].value);
                        }
                    } else {
                        // The parameter is a value, so add an edge to it.
                        node->AddEdge(current_value);
                    }

                    return std::make_pair(cf, node);
                }
            },

            [&](const sem::GlobalVariable* global) {
                // Loads from global read-write variables may be non-uniform.
                if (global->Declaration()->Is<ast::Var>() &&
                    global->Access() != core::Access::kRead && load_rule) {
                    node->AddEdge(current_function_->may_be_non_uniform);
                } else {
                    node->AddEdge(cf);
                }
                return std::make_pair(cf, node);
            },

            [&](const sem::LocalVariable* local) {
                node->AddEdge(cf);

                auto* local_value = current_function_->variables.Get(local);
                if (local->Type()->Is<core::type::Pointer>()) {
                    if (load_rule) {
                        // We are loading from the pointer, so add an edge to its contents.
                        auto* root = var_user->RootIdentifier();
                        if (root->Is<sem::GlobalVariable>()) {
                            if (root->Access() != core::Access::kRead) {
                                // The contents of a mutable global variable is always non-uniform.
                                node->AddEdge(current_function_->may_be_non_uniform);
                            }
                        } else {
                            node->AddEdge(current_function_->variables.Get(root));
                        }

                        // The uniformity of the contents also depends on the uniformity of the
                        // pointer itself. For a pointer captured in a let declaration, this will
                        // come from the value node of that declaration.
                        node->AddEdge(local_value);
                    } else {
                        // The variable is a pointer that we are not loading from, so add an edge to
                        // the pointer value itself.
                        node->AddEdge(local_value);
                    }
                } else if (local->Type()->Is<core::type::Reference>()) {
                    if (load_rule) {
                        // We are loading from the reference, so add an edge to its contents.
                        node->AddEdge(local_value);
                    } else {
                        // References to local variables (i.e. var declarations) are always uniform,
                        // so no other edges needed.
                    }
                } else {
                    // The identifier is a value declaration, so add an edge to it.
                    node->AddEdge(local_value);
                }

                return std::make_pair(cf, node);
            },

            TINT_ICE_ON_NO_MATCH);
    }

    /// Process an expression.
    /// @param cf the input control flow node
    /// @param expr the expression to process
    /// @param load_rule true if the load rule is being invoked on this expression
    /// @returns a pair of (control flow node, value node)
    std::pair<Node*, Node*> ProcessExpression(Node* cf,
                                              const ast::Expression* expr,
                                              bool load_rule = false) {
        if (sem_.Get<sem::Load>(expr)) {
            // Set the load-rule flag to indicate that identifier expressions in this sub-tree
            // should add edges to the contents of the variables that they refer to.
            load_rule = true;
        }

        return Switch(
            expr,

            [&](const ast::BinaryExpression* e) {
                if (e->IsLogical()) {
                    // Short-circuiting binary operators are a special case.
                    auto [cf1, v1] = ProcessExpression(cf, e->lhs);

                    // Add a diagnostic node to capture the control flow change.
                    auto* v1_cf = CreateNode({"short_circuit_op"}, e);
                    v1_cf->affects_control_flow = true;
                    v1_cf->AddEdge(v1);

                    auto [cf2, v2] = ProcessExpression(v1_cf, e->rhs);
                    return std::pair<Node*, Node*>(cf, v2);
                } else {
                    auto [cf1, v1] = ProcessExpression(cf, e->lhs);
                    auto [cf2, v2] = ProcessExpression(cf1, e->rhs);
                    auto* result = CreateNode({"binary_expr_result"}, e);
                    result->AddEdge(v1);
                    result->AddEdge(v2);
                    return std::pair<Node*, Node*>(cf2, result);
                }
            },

            [&](const ast::CallExpression* c) { return ProcessCall(cf, c); },

            [&](const ast::IdentifierExpression* i) {
                return ProcessIdentExpression(cf, i, load_rule);
            },

            [&](const ast::IndexAccessorExpression* i) {
                auto [cf1, v1] = ProcessExpression(cf, i->object, load_rule);
                auto [cf2, v2] = ProcessExpression(cf1, i->index);
                auto* result = CreateNode({"index_accessor_result"});
                result->AddEdge(v1);
                result->AddEdge(v2);
                return std::pair<Node*, Node*>(cf2, result);
            },

            [&](const ast::LiteralExpression*) { return std::make_pair(cf, cf); },

            [&](const ast::MemberAccessorExpression* m) {
                return ProcessExpression(cf, m->object, load_rule);
            },

            [&](const ast::UnaryOpExpression* u) {
                return ProcessExpression(cf, u->expr, load_rule);
            },

            TINT_ICE_ON_NO_MATCH);
    }

    /// @param u unary expression with op == kIndirection
    /// @returns true if `u` is an indirection unary expression that ultimately dereferences a
    /// partial pointer, false otherwise.
    bool IsDerefOfPartialPointer(const ast::UnaryOpExpression* u) {
        TINT_ASSERT(u->op == core::UnaryOp::kIndirection);

        // To determine if we're dereferencing a partial pointer, unwrap *&
        // chains; if the final expression is an identifier, see if it's a
        // partial pointer. If it's not an identifier, then it must be an
        // index/member accessor expression, and thus a partial pointer.
        auto* e = UnwrapIndirectAndAddressOfChain(u);
        if (auto* var_user = sem_.Get<sem::VariableUser>(e)) {
            if (current_function_->partial_ptrs.Contains(var_user->Variable())) {
                return true;
            }
        } else {
            TINT_ASSERT(e->Is<ast::AccessorExpression>());
            return true;
        }
        return false;
    }

    /// LValue holds the Nodes returned by ProcessLValueExpression()
    struct LValue {
        /// The control-flow node for an LValue expression
        Node* cf = nullptr;

        /// The new value node for an LValue expression
        Node* new_val = nullptr;

        /// The root identifier for an LValue expression.
        const sem::Variable* root_identifier = nullptr;
    };

    /// Process an LValue expression.
    /// @param cf the input control flow node
    /// @param expr the expression to process
    /// @param is_dereferencing `true` if we are dereferencing a pointer
    /// @param is_partial_reference `true` if we are referencing a subset of a variable
    /// @returns a tuple of (control flow node, variable node, root identifier)
    LValue ProcessLValueExpression(Node* cf,
                                   const ast::Expression* expr,
                                   bool is_dereferencing = false,
                                   bool is_partial_reference = false) {
        return Switch(
            expr,

            [&](const ast::IdentifierExpression* i) {
                auto* sem = sem_.GetVal(i)->UnwrapLoad()->As<sem::VariableUser>();
                auto result = Switch(
                    sem->Variable(),
                    [&](const sem::GlobalVariable*) {
                        // Pointers cannot be stored in module-scope variables, so we should never
                        // be dereferencing here.
                        TINT_ASSERT(!is_dereferencing);

                        return LValue{cf, current_function_->may_be_non_uniform, nullptr};
                    },
                    [&](const sem::LocalVariable* local) {
                        Node* value = nullptr;
                        const sem::Variable* root_ident = local;
                        if (is_dereferencing) {
                            // If we are dereferencing then we must have a pointer, and the only
                            // declaration that can hold a pointer is a `let`.
                            TINT_ASSERT(local->Declaration()->Is<ast::Let>() &&
                                        local->Type()->Is<core::type::Pointer>());

                            // Determine the root identifier for the contents of the pointer.
                            root_ident = local->Initializer()->RootIdentifier();

                            // Create a new value node for the contents of the pointer.
                            value = CreateNode({NameFor(root_ident), "_contents"});

                            // The uniformity of the value depends on the pointer itself.
                            value->AddEdge(current_function_->variables.Get(local));
                        } else {
                            // Create a new value node for this variable.
                            value = CreateNode({NameFor(i), "_lvalue"});
                        }

                        return LValue{cf, value, root_ident};
                    },
                    [&](const sem::Parameter* param) {
                        // Parameters can only be LValues when we are dereferencing a pointer.
                        TINT_ASSERT(is_dereferencing && param->Type()->Is<core::type::Pointer>());

                        // Create a new value node for the contents of the pointer.
                        auto* value = CreateNode({NameFor(i), "_contents"});

                        // The uniformity of the value depends on the pointer itself.
                        value->AddEdge(current_function_->parameters[param->Index()].value);

                        return LValue{cf, value, param};
                    },
                    [&](Default) -> LValue {
                        TINT_ICE() << "unknown lvalue identifier expression type: "
                                   << std::string(sem->Variable()->TypeInfo().name);
                    });

                // If the identifier is part of an expression that is a partial reference to a
                // variable (e.g. index or member access), we link back to the variable's previous
                // value. If the previous value was non-uniform, a partial assignment will not make
                // it uniform.
                auto* old_value = current_function_->variables.Get(result.root_identifier);
                if (is_partial_reference && old_value) {
                    result.new_val->AddEdge(old_value);
                }

                return result;
            },

            [&](const ast::IndexAccessorExpression* i) {
                // If the source object is a pointer, there is an implicit dereference due to the
                // pointer_composite_access language feature.
                is_dereferencing =
                    is_dereferencing || sem_.GetVal(i->object)->Type()->Is<core::type::Pointer>();

                auto [cf1, l1, root_ident] =
                    ProcessLValueExpression(cf, i->object, is_dereferencing,
                                            /*is_partial_reference*/ true);
                auto [cf2, v2] = ProcessExpression(cf1, i->index);
                l1->AddEdge(v2);
                return LValue{cf2, l1, root_ident};
            },

            [&](const ast::MemberAccessorExpression* m) {
                // If the source object is a pointer, there is an implicit dereference due to the
                // pointer_composite_access language feature.
                is_dereferencing =
                    is_dereferencing || sem_.GetVal(m->object)->Type()->Is<core::type::Pointer>();

                return ProcessLValueExpression(cf, m->object, is_dereferencing,
                                               /*is_partial_reference*/ true);
            },

            [&](const ast::UnaryOpExpression* u) {
                if (u->op == core::UnaryOp::kIndirection) {
                    return ProcessLValueExpression(
                        cf, u->expr,
                        /* is_dereferencing */ true,
                        /* is_partial_reference */ is_partial_reference ||
                            IsDerefOfPartialPointer(u));
                }
                return ProcessLValueExpression(cf, u->expr,
                                               /* is_dereferencing */ false,
                                               /* is_partial_reference */ is_partial_reference);
            },

            TINT_ICE_ON_NO_MATCH);
    }

    /// Process a function call expression.
    /// @param cf the input control flow node
    /// @param call the function call to process
    /// @returns a pair of (control flow node, value node)
    std::pair<Node*, Node*> ProcessCall(Node* cf, const ast::CallExpression* call) {
        std::string name = NameFor(call->target);

        // Process call arguments
        Node* cf_last_arg = cf;
        Vector<Node*, 8> args;
        Vector<Node*, 8> ptrarg_contents;
        ptrarg_contents.Resize(call->args.Length());
        for (size_t i = 0; i < call->args.Length(); i++) {
            auto [cf_i, arg_i] = ProcessExpression(cf_last_arg, call->args[i]);

            // Capture the index of this argument in a new node.
            // Note: This is an additional node that isn't described in the specification, for the
            // purpose of providing diagnostic information.
            Node* arg_node = CreateNode({name, "_arg_", std::to_string(i)}, call);
            arg_node->type = Node::kFunctionCallArgumentValue;
            arg_node->arg_index = static_cast<uint32_t>(i);
            arg_node->AddEdge(arg_i);

            // For pointer arguments, create an additional node to represent the contents of that
            // pointer prior to the function call.
            auto* sem_arg = sem_.GetVal(call->args[i]);
            if (sem_arg->Type()->Is<core::type::Pointer>()) {
                auto* arg_contents =
                    CreateNode({name, "_ptrarg_", std::to_string(i), "_contents"}, call);
                arg_contents->type = Node::kFunctionCallArgumentContents;
                arg_contents->arg_index = static_cast<uint32_t>(i);

                auto* root = sem_arg->RootIdentifier();
                if (root->Is<sem::GlobalVariable>()) {
                    if (root->Access() != core::Access::kRead) {
                        // The contents of a mutable global variable is always non-uniform.
                        arg_contents->AddEdge(current_function_->may_be_non_uniform);
                    }
                } else {
                    arg_contents->AddEdge(current_function_->variables.Get(root));
                }
                arg_contents->AddEdge(arg_node);
                ptrarg_contents[i] = arg_contents;
            }

            cf_last_arg = cf_i;
            args.Push(arg_node);
        }

        // Note: This is an additional node that isn't described in the specification, for the
        // purpose of providing diagnostic information.
        Node* call_node = CreateNode({name, "_call"}, call);
        call_node->AddEdge(cf_last_arg);

        Node* result = CreateNode({name, "_return_value"}, call);
        result->type = Node::kFunctionCallReturnValue;
        Node* cf_after = CreateNode({"CF_after_", name}, call);

        auto default_severity = kUniformityFailuresAsError ? wgsl::DiagnosticSeverity::kError
                                                           : wgsl::DiagnosticSeverity::kWarning;

        // Get tags for the callee.
        CallSiteTag callsite_tag = {CallSiteTag::CallSiteNoRestriction};
        FunctionTag function_tag = NoRestriction;
        auto* sem = SemCall(call);
        const FunctionInfo* func_info = nullptr;
        Switch(
            sem->Target(),
            [&](const sem::BuiltinFn* builtin) {
                // Most builtins have no restrictions. The exceptions are barriers, derivatives,
                // some texture sampling builtins, and atomics.
                if (builtin->IsBarrier()) {
                    callsite_tag = {CallSiteTag::CallSiteRequiredToBeUniform, default_severity};
                } else if (builtin->Fn() == wgsl::BuiltinFn::kWorkgroupUniformLoad) {
                    callsite_tag = {CallSiteTag::CallSiteRequiredToBeUniform, default_severity};
                } else if (builtin->IsDerivative() ||
                           builtin->Fn() == wgsl::BuiltinFn::kTextureSample ||
                           builtin->Fn() == wgsl::BuiltinFn::kTextureSampleBias ||
                           builtin->Fn() == wgsl::BuiltinFn::kTextureSampleCompare) {
                    // Get the severity of derivative uniformity violations in this context.
                    auto severity = sem_.DiagnosticSeverity(
                        call, wgsl::CoreDiagnosticRule::kDerivativeUniformity);
                    if (severity != wgsl::DiagnosticSeverity::kOff) {
                        callsite_tag = {CallSiteTag::CallSiteRequiredToBeUniform, severity};
                    }
                    function_tag = ReturnValueMayBeNonUniform;
                } else if (builtin->IsAtomic()) {
                    callsite_tag = {CallSiteTag::CallSiteNoRestriction};
                    function_tag = ReturnValueMayBeNonUniform;
                } else if (builtin->Fn() == wgsl::BuiltinFn::kTextureLoad) {
                    // Loading from a read-write storage texture may produce a non-uniform value.
                    auto* storage =
                        builtin->Parameters()[0]->Type()->As<core::type::StorageTexture>();
                    if (storage && storage->Access() == core::Access::kReadWrite) {
                        callsite_tag = {CallSiteTag::CallSiteNoRestriction};
                        function_tag = ReturnValueMayBeNonUniform;
                    }
                } else if (builtin->IsSubgroup()) {
                    // Get the severity of subgroup uniformity violations in this context.
                    auto severity = sem_.DiagnosticSeverity(
                        call, wgsl::CoreDiagnosticRule::kSubgroupUniformity);
                    if (severity != wgsl::DiagnosticSeverity::kOff) {
                        callsite_tag = {CallSiteTag::CallSiteRequiredToBeUniform, severity};
                    }
                    function_tag = ReturnValueMayBeNonUniform;
                }
            },
            [&](const sem::Function* func) {
                // We must have already analyzed the user-defined function since we process
                // functions in dependency order.
                auto info = functions_.Get(func->Declaration());
                TINT_ASSERT(info);
                callsite_tag = info->callsite_tag;
                function_tag = info->function_tag;
                func_info = info.value;
            },
            [&](const sem::ValueConstructor*) {
                callsite_tag = {CallSiteTag::CallSiteNoRestriction};
                function_tag = NoRestriction;
            },
            [&](const sem::ValueConversion*) {
                callsite_tag = {CallSiteTag::CallSiteNoRestriction};
                function_tag = NoRestriction;
            },  //
            TINT_ICE_ON_NO_MATCH);

        cf_after->AddEdge(call_node);

        if (function_tag == ReturnValueMayBeNonUniform) {
            result->AddEdge(current_function_->may_be_non_uniform);
        }

        result->AddEdge(cf_after);

        // For each argument, add edges based on parameter tags.
        for (size_t i = 0; i < args.Length(); i++) {
            if (func_info) {
                auto& param_info = func_info->parameters[i];

                // Capture the direct uniformity requirements.
                switch (param_info.tag_direct.tag) {
                    case ParameterTag::ParameterValueRequiredToBeUniform:
                        current_function_->RequiredToBeUniform(param_info.tag_direct.severity)
                            ->AddEdge(args[i]);
                        break;
                    case ParameterTag::ParameterContentsRequiredToBeUniform: {
                        current_function_->RequiredToBeUniform(param_info.tag_direct.severity)
                            ->AddEdge(ptrarg_contents[i]);
                        break;
                    }
                    case ParameterTag::ParameterNoRestriction:
                        break;
                }
                // Capture the effects of this parameter on the return value.
                switch (param_info.tag_retval.tag) {
                    case ParameterTag::ParameterValueRequiredToBeUniform:
                        result->AddEdge(args[i]);
                        break;
                    case ParameterTag::ParameterContentsRequiredToBeUniform: {
                        result->AddEdge(ptrarg_contents[i]);
                        break;
                    }
                    case ParameterTag::ParameterNoRestriction:
                        break;
                }

                // Capture the effects of other call parameters on the contents of this parameter
                // after the call returns.
                auto* sem_arg = sem_.GetVal(call->args[i]);
                if (sem_arg->Type()->Is<core::type::Pointer>()) {
                    auto* ptr_result =
                        CreateNode({name, "_ptrarg_", std::to_string(i), "_result"}, call);
                    ptr_result->type = Node::kFunctionCallPointerArgumentResult;
                    ptr_result->arg_index = static_cast<uint32_t>(i);
                    if (param_info.pointer_may_become_non_uniform) {
                        ptr_result->AddEdge(current_function_->may_be_non_uniform);
                    } else {
                        // Add edge to the call to catch when it's called in non-uniform control
                        // flow.
                        ptr_result->AddEdge(call_node);

                        // Add edges from the resulting pointer value to any other arguments that
                        // feed it. We distinguish between requirements on the source arguments
                        // value versus its contents for pointer arguments.
                        for (auto* source : param_info.ptr_output_source_param_values) {
                            ptr_result->AddEdge(args[source->Index()]);
                        }
                        for (auto* source : param_info.ptr_output_source_param_contents) {
                            ptr_result->AddEdge(ptrarg_contents[source->Index()]);
                        }
                    }

                    // Update the current stored value for this pointer argument.
                    auto* root_ident = sem_arg->RootIdentifier();
                    TINT_ASSERT(root_ident);
                    current_function_->variables.Set(root_ident, ptr_result);
                }
            } else {
                auto* builtin = sem->Target()->As<sem::BuiltinFn>();
                if (builtin && builtin->Fn() == wgsl::BuiltinFn::kWorkgroupUniformLoad) {
                    // The workgroupUniformLoad builtin requires its parameter to be uniform.
                    current_function_->RequiredToBeUniform(default_severity)->AddEdge(args[i]);
                } else if (builtin &&
                           (builtin->Fn() == wgsl::BuiltinFn::kSubgroupShuffleDown ||
                            builtin->Fn() == wgsl::BuiltinFn::kSubgroupShuffleUp ||
                            builtin->Fn() == wgsl::BuiltinFn::kSubgroupShuffleXor) &&
                           i == 1) {
                    // The subgroupShuffle{Down,Up,Xor} builtins require their `delta` parameters to
                    // be uniform.
                    // Get the severity of subgroup uniformity violations in this context.
                    auto severity = sem_.DiagnosticSeverity(
                        call->args[i], wgsl::CoreDiagnosticRule::kSubgroupUniformity);
                    if (severity != wgsl::DiagnosticSeverity::kOff) {
                        current_function_->RequiredToBeUniform(severity)->AddEdge(args[i]);
                    }
                } else {
                    // All other builtin function parameters are RequiredToBeUniformForReturnValue,
                    // as are parameters for value constructors and value conversions.
                    result->AddEdge(args[i]);
                }
            }
        }

        // Add the callsite requirement last.
        // We traverse edges in reverse order, so this makes the callsite requirement take highest
        // priority when reporting violations.
        if (callsite_tag.tag == CallSiteTag::CallSiteRequiredToBeUniform) {
            current_function_->RequiredToBeUniform(callsite_tag.severity)->AddEdge(call_node);
        }

        return {cf_after, result};
    }

    /// Traverse a graph starting at `source`, inserting all visited nodes into `reachable` and
    /// recording which node they were reached from.
    /// @param source the starting node
    /// @param reachable the set of reachable nodes to populate, if required
    void Traverse(Node* source, UniqueVector<Node*, 4>* reachable = nullptr) {
        Vector<Node*, 8> to_visit{source};

        while (!to_visit.IsEmpty()) {
            auto* node = to_visit.Back();
            to_visit.Pop();

            if (reachable) {
                reachable->Add(node);
            }
            for (auto* to : node->edges) {
                if (to->visited_from == nullptr) {
                    to->visited_from = node;
                    to_visit.Push(to);
                }
            }
        }
    }

    /// Trace back along a path from `start` until finding a node that matches a predicate.
    /// @param start the starting node
    /// @param pred the predicate function
    /// @returns the first node found that matches the predicate, or nullptr
    template <typename F>
    Node* TraceBackAlongPathUntil(Node* start, F&& pred) {
        auto* current = start;
        while (current) {
            if (pred(current)) {
                break;
            }
            current = current->visited_from;
        }
        return current;
    }

    /// Recursively descend through the function called by `call` and the functions that it calls in
    /// order to find a call to a builtin function that requires uniformity with the given severity.
    const ast::CallExpression* FindBuiltinThatRequiresUniformity(
        const ast::CallExpression* call,
        wgsl::DiagnosticSeverity severity) {
        auto* target = SemCall(call)->Target();
        if (target->Is<sem::BuiltinFn>()) {
            // This is a call to a builtin, so we must be done.
            return call;
        } else if (auto* user = target->As<sem::Function>()) {
            // This is a call to a user-defined function, so inspect the functions called by that
            // function and look for one whose node has an edge from the RequiredToBeUniform node.
            auto target_info = functions_.Get(user->Declaration());
            for (auto* call_node : target_info->RequiredToBeUniform(severity)->edges) {
                if (call_node->type == Node::kRegular) {
                    auto* child_call = call_node->ast->As<ast::CallExpression>();
                    return FindBuiltinThatRequiresUniformity(child_call, severity);
                }
            }
            TINT_UNREACHABLE() << "unable to find child call with uniformity requirement";
        } else {
            TINT_UNREACHABLE() << "unexpected call expression type";
        }
    }

    /// Add diagnostic notes to show where control flow became non-uniform on the way to a node.
    /// @param function the function being analyzed
    /// @param required_to_be_uniform the node to traverse from
    /// @param may_be_non_uniform the node to traverse to
    void ShowControlFlowDivergence(FunctionInfo& function,
                                   Node* required_to_be_uniform,
                                   Node* may_be_non_uniform) {
        // Traverse the graph to generate a path from the node to the source of non-uniformity.
        function.ResetVisited();
        Traverse(required_to_be_uniform);

        // Get the source of the non-uniform value.
        auto* non_uniform_source = may_be_non_uniform;
        if (non_uniform_source == function.may_be_non_uniform) {
            non_uniform_source = non_uniform_source->visited_from;
        }
        TINT_ASSERT(non_uniform_source);

        // Show where the non-uniform value results in non-uniform control flow.
        auto* control_flow = TraceBackAlongPathUntil(
            non_uniform_source, [](Node* node) { return node->affects_control_flow; });
        if (control_flow) {
            diagnostics_.AddNote(control_flow->ast->source)
                << "control flow depends on possibly non-uniform value";
            // TODO(jrprice): There are cases where the function with uniformity requirements is not
            // actually inside this control flow construct, for example:
            // - A conditional interrupt (e.g. break), with a barrier elsewhere in the loop
            // - A conditional assignment to a variable, which is later used to guard a barrier
            // In these cases, the diagnostics are not entirely accurate as they may not highlight
            // the actual cause of divergence.
        }

        ShowSourceOfNonUniformity(non_uniform_source);
    }

    /// Add a diagnostic note to show the origin of a non-uniform value.
    /// @param non_uniform_source the node that represents a non-uniform value
    void ShowSourceOfNonUniformity(Node* non_uniform_source) {
        TINT_ASSERT(non_uniform_source);

        auto var_type = [&](const sem::Variable* var) {
            switch (var->AddressSpace()) {
                case core::AddressSpace::kStorage:
                    return "read_write storage buffer ";
                case core::AddressSpace::kWorkgroup:
                    return "workgroup storage variable ";
                case core::AddressSpace::kPrivate:
                    return "module-scope private variable ";
                default:
                    return "";
            }
        };
        auto param_type = [&](const ast::Parameter* param) {
            if (ast::HasAttribute<ast::BuiltinAttribute>(param->attributes)) {
                return "builtin ";
            } else if (ast::HasAttribute<ast::LocationAttribute>(param->attributes)) {
                return "user-defined input ";
            } else {
                return "parameter ";
            }
        };

        // Show the source of the non-uniform value.
        Switch(
            non_uniform_source->ast,
            [&](const ast::IdentifierExpression* ident) {
                auto* var = sem_.GetVal(ident)->UnwrapLoad()->As<sem::VariableUser>()->Variable();
                if (auto* param = var->As<sem::Parameter>()) {
                    auto* func = param->Owner()->As<sem::Function>();
                    diagnostics_.AddNote(ident->source)
                        << param_type(param->Declaration()) << "'" << NameFor(ident) << "' of '"
                        << NameFor(func) << "' may be non-uniform";
                } else {
                    diagnostics_.AddNote(ident->source)
                        << "reading from " << var_type(var) << "'" << NameFor(ident)
                        << "' may result in a non-uniform value";
                }
            },
            [&](const ast::Parameter* p) {
                auto* param = sem_.Get(p);
                auto* func = param->Owner()->As<sem::Function>();
                if (non_uniform_source->type == Node::kFunctionPointerParameterContents) {
                    diagnostics_.AddNote(p->source)
                        << "parameter '" << NameFor(p) << "' of '" << NameFor(func)
                        << "' may point to a non-uniform value";
                } else {
                    diagnostics_.AddNote(p->source)
                        << param_type(p) << "'" << NameFor(p) << "' of '" << NameFor(func)
                        << "' may be non-uniform";
                }
            },
            [&](const ast::Variable* v) {
                auto* var = sem_.Get(v);
                diagnostics_.AddNote(v->source)
                    << "reading from " << var_type(var) << "'" << NameFor(v)
                    << "' may result in a non-uniform value";
            },
            [&](const ast::CallExpression* c) {
                auto target_name = NameFor(c->target);
                switch (non_uniform_source->type) {
                    case Node::kFunctionCallReturnValue: {
                        diagnostics_.AddNote(c->source)
                            << "return value of '" + target_name + "' may be non-uniform";
                        break;
                    }
                    case Node::kFunctionCallArgumentContents: {
                        auto* arg = c->args[non_uniform_source->arg_index];
                        auto* var = sem_.GetVal(arg)->RootIdentifier();
                        diagnostics_.AddNote(var->Declaration()->source)
                            << "reading from " << var_type(var) << "'" << NameFor(var)
                            << "' may result in a non-uniform value";
                        break;
                    }
                    case Node::kFunctionCallArgumentValue: {
                        auto* arg = c->args[non_uniform_source->arg_index];
                        // TODO(jrprice): Which output? (return value vs another pointer argument).
                        diagnostics_.AddNote(arg->source)
                            << "passing non-uniform pointer to '" << target_name
                            << "' may produce a non-uniform output";
                        break;
                    }
                    case Node::kFunctionCallPointerArgumentResult: {
                        diagnostics_.AddNote(c->args[non_uniform_source->arg_index]->source)
                            << "contents of pointer may become non-uniform after calling '"
                            << target_name << "'";
                        break;
                    }
                    default: {
                        TINT_ICE() << "unhandled source of non-uniformity";
                    }
                }
            },
            [&](const ast::Expression* e) {
                diagnostics_.AddNote(e->source) << "result of expression may be non-uniform";
            },  //
            TINT_ICE_ON_NO_MATCH);
    }

    /// Generate a diagnostic message for a uniformity issue.
    /// @param function the function that the diagnostic is being produced for
    /// @param source_node the node that has caused a uniformity issue in `function`
    /// @param severity the severity of the diagnostic
    void MakeError(FunctionInfo& function, Node* source_node, wgsl::DiagnosticSeverity severity) {
        // Helper to produce a diagnostic message, as a note or with the global failure severity.
        auto report = [&](Source source, std::string msg, bool note) {
            diag::Diagnostic error{};
            error.severity = note ? diag::Severity::Note : wgsl::ToSeverity(severity);
            error.source = source;
            error.message = msg;
            diagnostics_.Add(std::move(error));
        };

        // Traverse the graph to generate a path from RequiredToBeUniform to the source node.
        function.ResetVisited();
        Traverse(function.RequiredToBeUniform(severity));
        TINT_ASSERT(source_node->visited_from);

        // Find a node that is required to be uniform that has a path to the source node.
        auto* cause = TraceBackAlongPathUntil(source_node, [&](Node* node) {
            return node->visited_from == function.RequiredToBeUniform(severity);
        });

        // The node will always have a corresponding call expression.
        auto* call = cause->ast->As<ast::CallExpression>();
        TINT_ASSERT(call);
        auto* target = SemCall(call)->Target();
        auto func_name = NameFor(call->target);

        if (cause->type == Node::kFunctionCallArgumentValue ||
            cause->type == Node::kFunctionCallArgumentContents) {
            bool is_value = (cause->type == Node::kFunctionCallArgumentValue);

            auto* user_func = target->As<sem::Function>();
            if (user_func) {
                // Recurse into the called function to show the reason for the requirement.
                auto next_function = functions_.Get(user_func->Declaration());
                auto& param_info = next_function->parameters[cause->arg_index];
                MakeError(*next_function,
                          is_value ? param_info.value : param_info.ptr_input_contents, severity);

                // Show the place where the non-uniform argument was passed.
                // If this is a builtin, this will be the trigger location for the failure.
                StringStream ss;
                ss << "possibly non-uniform value passed" << (is_value ? "" : " via pointer")
                   << " here";
                report(call->args[cause->arg_index]->source, ss.str(), /* note */ true);
            } else {
                // The uniformity requirement must come from a builtin function.
                auto* builtin = target->As<sem::BuiltinFn>();
                TINT_ASSERT(builtin);
                StringStream ss;
                ss << "'" << builtin->Fn() << "' requires argument " << cause->arg_index << " to "
                   << (is_value ? "be uniform" : "have uniform contents");
                report(call->args[cause->arg_index]->source, ss.str(), /* note */ false);
            }

            // Show the origin of non-uniformity for the value or data that is being passed.
            ShowSourceOfNonUniformity(source_node->visited_from);
        } else {
            auto* builtin_call = FindBuiltinThatRequiresUniformity(call, severity);
            {
                // Show a builtin was reachable from this call (which may be the call itself).
                // This will be the trigger location for the failure.
                StringStream ss;
                ss << "'" << NameFor(builtin_call->target)
                   << "' must only be called from uniform control flow";
                report(builtin_call->source, ss.str(), /* note */ false);
            }

            if (builtin_call != call) {
                // The call was to a user function, so show that call too.
                StringStream ss;
                ss << "called ";
                if (target->As<sem::Function>() != SemCall(builtin_call)->Stmt()->Function()) {
                    ss << "indirectly ";
                }
                ss << "by '" << func_name << "' from '" << function.name << "'";
                report(call->source, ss.str(), /* note */ true);
            }

            // Show the point at which control-flow depends on a non-uniform value.
            ShowControlFlowDivergence(function, cause, source_node);
        }
    }

    // Helper for obtaining the sem::Call node for the ast::CallExpression
    const sem::Call* SemCall(const ast::CallExpression* expr) const {
        return sem_.Get(expr)->UnwrapMaterialize()->As<sem::Call>();
    }
};

}  // namespace

bool AnalyzeUniformity(ProgramBuilder& builder, const DependencyGraph& dependency_graph) {
    UniformityGraph graph(builder);
    return graph.Build(dependency_graph);
}

}  // namespace tint::resolver
