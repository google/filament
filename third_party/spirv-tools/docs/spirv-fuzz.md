# Guide to writing a spirv-fuzz fuzzer pass

Writing a spirv-fuzz fuzzer pass usually requires two main contributions:

- A *transformation*, capturing a small semantics-preserving change that can be made to a SPIR-V module.  This requires adding a protobuf message representing the transformation, and a corresponding class that implements the `Transformation` interface.
- A new *fuzzer pass* class, implementing the `FuzzerPass` interface, that knows how to walk a SPIR-V module and apply the new transformation in a randomized fashion.

In some cases, more than one kind of transformation is required for a single fuzzer pass, and in some cases the transformations that a new fuzzer pass requires have already been introduced by existing passes.  But the most common case is to introduce a transformation and fuzzer pass together.

As an example, let's consider the `TransformationSetSelectionControl` transformation.  In SPIR-V, an `OpSelectionMerge` instruction (which intuitively indicates the start of an `if` or `switch` statement in a function) has a *selection control* mask, that can be one of `None`, `Flatten` or `DontFlatten`.  The details of these do not matter much for this little tutorial, but in brief, this parameter provides a hint to the shader compiler as to whether it would be profitable to attempt to flatten a piece of conditional code so that all of its statements are executed in a predicated fashion.

As the selection control mask is just a hint, changing the value of this mask should have no semantic impact on the module.  The `TransformationSelectionControl` transformation specifies a new value for a given selection control mask.

## Adding a new protobuf message

Take a look at the `Transformation` message in `spvtoolsfuzz.proto`.  This has a `oneof` field that can be any one of the different spirv-fuzz transformations.  Observe that one of the options is `TransformationSetSelectionControl`.  When adding a transformation you first need to add an option for your transformation to the end of the `oneof` declaration.

Now look at the `TransformationSetSelectionControl` message.  If adding your own transformation you need to add a new message for your transformation, and it should be placed alphabetically with respect to other transformations.

The fields of `TransformationSetSelectionControl` provide just enough information to (a) determine whether a given example of this transformation is actually applicable, and (b) apply the transformation in the case that it is applicable.  The details of the transformation message will vary a lot between transformations.  In this case, the message has a `block_id` field, specifying a block that must end with `OpSelectionMerge`, and a `selection_control` field, which is the new value for the selection control mask of the `OpSelectionMerge` instruction.

## Adding a new transformation class

If your transformation is called `TransformationSomeThing`, you need to add `transformation_some_thing.h` and `transformation_some_thing.cpp` to `source/fuzz` and the corresponding `CMakeLists.txt` file.  So for `TransformationSetSelectionControl` we have `transformation_selection_control.h` and `transformation_selection_control.cpp`, and we will use this as an example to illustrate the expected contents of these files.

The header file contains the specification of a class, `TransformationSetSelectionControl`, that implements the `Transformation` interface (from `transformation.h`).

A transformation class should always have a single field, which should be the associated protobuf message; in our case:

```
 private:
  protobufs::TransformationSetSelectionControl message_;
```

and two public constructors, one that takes a protobuf message; in our case:

```
  explicit TransformationSetSelectionControl(
      const protobufs::TransformationSetSelectionControl& message);
```

and one that takes a parameter for each protobuf message field; in our case:

```
  TransformationSetSelectionControl(uint32_t block_id);
```

The first constructor allows an instance of the class to be created from a corresponding protobuf message.  The second should provide the ingredients necessary to populate a protobuf message.

The class should also override the `IsApplicable`, `Apply` and `ToMessage` methods from `Transformation`.

See `transformation_set_selection_control.h` for an example.

The `IsApplicable` method should have a comment in the header file describing the conditions for applicability in simple terms.  These conditions should be implemented in the body of this method in the `.cpp` file.

In the case of `TransformationSetSelectionControl`, `IsApplicable` involves checking that `block_id` is indeed the id of a block that has an `OpSelectoinMerge` instruction, and that `selection_control` is a valid selection mask.

The `Apply` method should have a comment in the header file summarising the result of applying the transformation.  It should be implemented in the `.cpp` file, and you should assume that `IsApplicable` holds whenever `Apply` is invoked.

## Writing tests for the transformation class

Whenever you add a transformation class, `TransformationSomeThing`, you should add an associated test file, `transformation_some_thing_test.cpp`, under `test/fuzz`, adding it to the associated `CMakeLists.txt` file.

For example `test/fuzz/transformation_set_selection_control_test.cpp` contains tests for `TransformationSetSelectionControl`.  Your tests should aim to cover one example from each scenario where the transformation is inapplicable, and check that it is indeed deemed inapplicable, and then check that the transformation does the right thing when applied in a few different ways.

For example, the tests for `TransformationSetSelectionControl` check that a transformation of this kind is inapplicable if the `block_id` field of the transformation is not a block, or does not end in `OpSelectionMerge`, or if the `selection_control` mask has an illegal value.  It also checks that applying a sequence of valid transformations to a SPIR-V shader leads to a shader with appropriately modified selection controls.

## Adding a new fuzzer pass class

A *fuzzer pass* traverses a SPIR-V module looking for places to apply a certain kind of transformation, and randomly decides at which of these points to actually apply the transformation.  It might be necessary to apply other transformations in order to apply a given transformation (for example, if a transformation requires a certain type to be present in the module, said type can be added if not already present via another transformation).

A fuzzer pass implements the `FuzzerPass` interface, and overrides its `Apply` method.  If your fuzzer pass is named `FuzzerPassSomeThing` then it should be represented by `fuzzer_pass_some_thing.h` and `fuzzer_pass_some_thing.cpp`, under `source/fuzz`; these should be added to the associated `CMakeLists.txt` file.

Have a look at the source filed for `FuzzerPassAdjustSelectionControls`.  This pass considers every block that ends with `OpSelectionMerge`.  It decides randomly whether to adjust the selection control of this merge instruction via:

```
if (!GetFuzzerContext()->ChoosePercentage(
        GetFuzzerContext()->GetChanceOfAdjustingSelectionControl())) {
  continue;
}
```

The `GetChanceOfAddingSelectionControl()` method has been added to `FuzzerContext` specifically to support this pass, and returns a percentage between 0 and 100.  It returns the `chance_of_adjusting_selection_control_` of `FuzzerContext`, which is randomly initialized to lie with the interval defined by `kChanceOfAdjustingSelectionControl` in `fuzzer_context.cpp`.  For any pass you write, you will need to add an analogous `GetChanceOf...` method to `FuzzerContext`, backed by an appropriate field, and you will need to decide on lower and upper bounds for this field and specify these via a `kChanceOf...` constant.
