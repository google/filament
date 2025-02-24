# Passes

Each pass has a `Run` that starts the pass, from here there are 3 stages to every pass

## Step 1 - Analyze if we need to add check

Each pass does logic needed to know if the current instruction needs have check before it.

## Step 2 - Inject a function call

Functions are added via `Pass::InjectFunctionCheck`, but there are cases were we want to make sure we don't call the invalid instructions. For this we add an `if-else` control flow logic in SPIR-V (all handled by the `Pass::InjectConditionalFunctionCheck`) to inject the function. This will create the various blocks and resolve any ID updates


## Step 3 - Create the OpFunctionCall

Each pass will have its own unique signature to the function in the GLSL code being linked later, so the virtual `Pass::CreateFunctionCall` function is then called and the pass needs to create the `OpFunctionCall` instruction. This is where the pass can provide any arguments needed, likely data saved while doing the analyze phase.