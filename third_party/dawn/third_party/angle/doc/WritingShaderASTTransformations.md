# How to write ANGLE shader translator AST transformations

Usually, a shader AST transformation is structured as such:

**.h file**: one function taking a node to transform, often the root node. If the transformation needs to create new variables or functions, you need to pass a pointer to the TSymbolTable as well (all functions and variables need an id number that's based on a counter in TSymbolTable).

    class TIntermBlock;
    TransformAST(TIntermBlock *root)

**.cpp file**: implementation of the transformation, with all the implementation code in an anonymous namespace.

## Utilities to implement a transformation

The utilities for implementing AST transformations are in **src/compiler/translator/tree_util/**

**TIntermTraverser**: This traverses the tree recursively, visiting all nodes. Override the visit* functions you need.

**TLValueTrackingTraverser**: If you need to know if the node that is being visited is a target of a write (used as an l-value), use TLValueTrackingTraverser and its isLValueRequiredHere() function.

If you're only interested in function declarations or global variables, they're all in the global scope, so you can get away with just iterating over the root block instead of using a traverser.

TIntermTraverser has member functions to insert and replace nodes. To remove a node, replace it with an empty list. Usually visit functions queue replacements and then updateTree() is called after the traversal is complete - this way the replacement doesn't affect the current traversal. For some transformations of nested AST structures you may need to do multiple traversals.

**BuiltIn.h**: With the helpers here you can easily and cheaply create references to built-in variables that are stored as constexpr.

**IntermNode_utils.h**: These utilities can do things like create a zero node with an arbitrary type, create bool nodes, index nodes, or operations on temporary variables.

**RunAtTheEndOfShader.h**: Use this when you need to run code at the end of the shader so you don't need to worry about corner cases like return statements in main().

**ReplaceVariable.h**: Replace all references to a specific variable in the AST. Useful if a type of a variable needs to be changed, for example.

**FindMain.h**: Find the main() function in the AST.

**FindSymbolNode.h**: Find a particular symbol in the AST.

**IntermNodePatternMatcher.h**: This helper matches certain AST patterns that are needed in more than one different transformation, such as expressions returning an array.

**StaticType.h**: Create TType objects that are initialized at compile time.

Some member functions of AST nodes can also be useful:
* **deepCopy()** creates a copy of any typed node, including its children.
* **hasSideEffects()** determines whether an expression might have side effects. Usually we want to avoid removing nodes with side effects unless there's certainty they would never be executed.

## Checklist

When implementing traversers, be careful that:
* Each node will only have one parent (multiple parents could mess up further AST transformations)
* You take into account some less common AST structures, such as declarations inside a loop header.
* You run the transformation at the right stage of compilation. You don't want to reintroduce AST structures that have already been pruned away for example, and on the other hand you can depend on earlier transformations to clean up some inconvenient structures from the AST, like empty declarations or multiple variables declared on the same line.
* Make sure that any functions you add are marked with SymbolType::AngleInternal so they exist in a separate namespace from user-defined names and can't conflict with them.
