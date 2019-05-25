# benchdnn

**benchdnn** is a standalone correctness and performance benchmark for
[Intel(R) Math Kernel Library for Deep Neural Networks (Intel(R) MKL-DNN)](/intel/mkl-dnn).
The purpose of the benchmark is extended and robust correctness verification of
the primitives provided by Intel MKL-DNN. Currently, **benchdnn** supports convolutions
, inner products, reorder, batch normalization, deconvolution, recurrent neural network, and shuffle of different data types.


## License
**benchdnn** is licensed under
[Apache License Version 2.0](http://www.apache.org/licenses/LICENSE-2.0).


## Usage (main driver)

**benchdnn** itself is a driver for different implementation-specific
harnesses. So far it uses a harness for Intel MKL-DNN [convolution](/tests/benchdnn/README.md#usage-convolution-harness), [inner product](/tests/benchdnn/README.md#usage-ip-harness),
[reorder](/tests/benchdnn/README.md#usage-reorder-harness), [batch normalization](/tests/benchdnn/README.md#usage-batch-normalization-harness), [deconvolution](/tests/benchdnn/README.md#usage-deconvolution-harness), [shuffle](/tests/benchdnn/README.md#usage-shuffle-harness), and [recurrent neural network](/tests/benchdnn/README.md#usage-rnn-harness) as well as a
harness for testing [itself](/tests/benchdnn/README.md#usage-self-harness).

Usage:
```
    $ ./benchdnn: [--HARNESS] [--mode=MODE] [--max-ms-per-prb=MAX-MS-PER-PRB] [-vN|--verbose=N] HARNESS-OPTS
```
where:

 - `HARNESS` is either `conv` [default], `ip`, `shuffle`, `reorder`, `bnorm`, `rnn`, or `self`

 - `MODE` -- string that contains flags for benchmark mode. Use `C` or `c` for correctness (used by default), and `P` or `p` for performance

 - `MAX-MS-PER-PRB`  is passed to assign the maximum time spent per problem in milliseconds, by default `3e3`
 - `-vN|--verbose=N` -- verbose level, default `0`

 - `HARNESS-OPTS`  are passed to the chosen harness

Returns `0` on success (all tests passed) or non-zero in case of any error.

## Notations / Glossary / Abbreviations

|Abbreviation   | Description
|:---           |:---
| src           | Source image (input image for forward convolution)
| wei           | Weights (aka filter)
| bia           | Bias
| dst           | Destination image (output image for forward convolution)
| acc           | Accumulation (typically in terms of data type)
| ic, oc        | Input/Output channels (aka feature maps)
| ih, iw        | Input height and width
| oh, ow        | Output height and width
| kh, kw        | Kernel (filter, weights) height and width
| sh, sw        | Convolution stride over height and width
| ph, pw        | Convolution top and left padding
| mb            | Minibatch (amount of images processed at once)
| g             | Groups (a way to reduce the amount of computations, see Alexnet topology)
| FWD_{D,B}     | forward w/o and w/ bias
| BWD_{D,W,WB}  | backward wrt data, weights, and weights and bias
| DIRECT, WINO  | convolution algorithm: direct or Winograd based
| AUTO          | convolution algorithm is chosen by MKL-DNN for best performance


## Usage (convolution harness)

```
    [harness-knobs] [conv-desc] ...
```

where *harness-knobs* are:

 - `--cfg={f32, u8s8u8s32, ...}` configuration (see below [convolution configuration](/tests/benchdnn/README.md#convolution-configurations-also-known-as-precision-specification)), default `f32`
 - `--dir={FWD_D (forward data), FWD_B (forward data + bias),FWD_I (forward data inference), BWD_D (backward data), BWD_W (backward weights), BWD_WB (backward weights + bias)}` direction, default `FWD_B`
 - `--alg={DIRECT, WINO, AUTO}` convolution algorithm, default DIRECT
 - `--attr="attr_str"` convolution attributes (see in the section below), default `""` (no attributes set)
 - `--mb=N` override minibatch that is specified in convolution description, default `0` (use mb specified in conv desc)
 - `--match=regex` check only convolutions that match with regex, default is `".*"`. Notice: Windows may only interpret string arguments surrounded by double quotation marks.
 - `--skip-impl="str1[:str2]..."` skip implementation (see mkldnn_query_impl_info_str), default `""`
 - `--allow-unimpl=true|false` do not treat unimplemented configuration as an error, default `false`
 - `--perf-template=template-str` set template for performance report (see section *Performance measurements*)
 - `--reset` reset all the parameters set before to default one
 - `-vN|--verbose=N` verbose level, default `0`
 - `--batch=file` use options from the given file (see in subdirectory)
 - `--mode=` string that contains flags for benchmark mode. Use `C` or `c` for correctness (used by default), and `P` or `p` for performance 

and *conv-desc* is the convolution description. The canonical form is:
```
    gXmbXicXihXiwXocXohXowXkhXkwXshXswXphXpwXdhXdwXnS
```
Here X is a number and S is a string (n stands for name). Some of the parameters
may be omitted if a default exists (for example, if g is not specified
**benchdnn** uses 1) or if it can be computed automatically (for example, the output shape
can be derived from the input one and the kernel). Also, if either width or height
is not specified, it is assumed that height == width. The special symbol `_` is
ignored, so it may be used as a delimiter. See `str2desc()` in conv/conv_aux.cpp
for more details and implicit rules.

The attribute string *attr_str* is defined as follows (line breaks are for readability):
```
    [oscale={none,common,per_oc}[:scale];]
    [post_ops='[{relu,sum[:sum_scale]};]...';]
```

Next, `oscale` stands for output_scales. The first parameter is the policy that
is defined below. The second optional parameter is a scale that specifies
either the one common output scale (for the `none` and `common` polices) or a
starting point for the `per_oc` policy, which uses many scales. The default scale
is 1.0. Known policies are:

  - `none` (default) means no output scales set (i.e. scale = 1.)
  - `common` corresponds to `mask=0` with common scale factor
  - `per_oc` corresponds to `mask=1<<1` (i.e. output channels) with different scale factors

Next, `post_ops` stands for post operation sequence. Currently supported post
operations are:

  - `relu` with no parameters (i.e. corresponding scale is 1., alg = eltwise_relu, alpha = beta = 0.)
  - `sum` with optional parameter scale (default 1.)

### Convolution configurations (also known as precision specification)

`--cfg` option specifies what convolution would be used in terms of data type.
Also it defines all the magic with data filling inside. For the integer type,
saturation is implicitly implied.

Finally configuration defines the threshold for computation errors (ideally we
want to keep it at 0, and it seems to work for now).

The table below shows cases supported by Intel MKL-DNN and corresponding
configurations for **benchdnn**:

|src type | wei type | dst type | acc type | cfg          | notes
|:---     |:---      |:---      |:---      |:---          |:---
| f32     | f32      | f32      | f32      | f32          | inference optimized for sse4.2+, training avx2+
| u8      | s8       | f32      | s32      | u8s8f32s32   | optimized for processors with support of avx512vl, forward pass only (aka FWD_D, FWD_B)
| u8      | s8       | s32      | s32      | u8s8s32s32   | same notes as for u8s8f32s32
| u8      | s8       | s8       | s32      | u8s8s8s32    | same notes as for u8s8f32s32
| u8      | s8       | u8       | s32      | u8s8u8s32    | same notes as for u8s8f32s32
| s8      | s8       | f32      | s32      | s8s8f32s32   | same notes as for u8s8f32s32
| s8      | s8       | s32      | s32      | s8s8s32s32   | same notes as for u8s8f32s32
| s8      | s8       | s8       | s32      | s8s8s8s32    | same notes as for u8s8f32s32
| s8      | s8       | u8       | s32      | s8s8u8s32    | same notes as for u8s8f32s32


### Performance measurements (convolution harness)

**benchdnn** supports a custom performance report. A template is passed via the
command line and consists of terminal and nonterminal symbols. Nonterminal
symbols are printed as-is. A description of terminal symbols is given below.
There is also a notion of modifiers (marked with @) that change the meaning of
terminal symbols; for example, the sign '-' means minimum of (in terms of time).
See the table of modifiers below.

> **Caution:** Threads must be pinned in order to get consistent frequency.

| Abbreviation  | Description
|:------------  |:-----------
| %d            | problem descriptor
| %D            | expanded problem descriptor (conv parameters in csv format)
| %n            | problem name
| %z            | direction
| %@F           | effective cpu frequency computed as clocks[@] / time[@]
| %O            | number of ops required (padding is not taken into account)
| %@t           | time in ms
| %@c           | time in clocks
| %@p           | ops per second

| Modifier  | Description
|:--------  |:-----------
|           | default
| -         | min (time) -- default
| 0         | avg (time)
| +         | max (time)
|           |
| K         | Kilo (1e3)
| M         | Mega (1e6)
| G         | Giga (1e9)

The definition of expanded problem descriptor is:
`g,mb,ic,ih,iw,oc,oh,ow,kh,kw,sh,sw,ph,pw`.

The default template can be found in conv/bench_conv.cpp and is defined as
`perf,%n,%d,%GO,%GF,%-t,%-Gp,%0t,%0Gp`. That will produce the following output
in CSV format:
```
string: perf
convolution name
full conv-desc
number of giga ops calculated
effective cpu frequency in GHz (amb clocks[min] / time[min])
minimum time spent in ms
best gigaops (since it corresponds to mimimum time)
average time spent in ms
average gigaops (since it corresponds to average time)
```
Here is an example of the performance output:
```
 perf,"yolov2:conv1",mb16ic3ih610oc32oh608kh3n"yolov2:conv1",10.2205,0,43.9827,232.375,58.0146,176.171
```
full convolution descriptor is `mb16ic3ih610oc32oh608kh3n"yolov2:conv1"` in the above example.

### Examples (convolution harness)

Run the set of f32 forward convolutions from inputs/conv_all file w/ bias and default minibatch:
```
    $ ./benchdnn --conv \
        --cfg=f32 --dir=FWD_B --batch=inputs/conv_all
```

Run the same but with post_ops ReLU:
```
    $ ./benchdnn --conv \
        --cfg=f32 --dir=FWD_B --attr="post_ops='relu'" --batch=inputs/conv_all
```

Run the same as previous but also measure performance:
```
    $ ./benchdnn --conv  --mode=CORRnPERF \
        --cfg=f32 --dir=FWD_B --attr="post_ops='relu'" --batch=inputs/conv_all
```

> **Note**: Instead of `CORRnPERF`, one can use `CP`, `PC`, `cp`, or `pc`

Run a set of f32 backward convolutions wrt weights with kh=3 and
verbose level set to 2:
```
    $ ./benchdnn --conv -v2 \
        --cfg=f32 --dir=BWD_W --match='.*kh3[^0-9].*' --batch=inputs/conv_all
```

Run a set of u8s8u8s32 backward convolutions wrt data but skip all
the convolutions that will use reference or gemm-based implementation:
```
    $ ./benchdnn --conv \
        --cfg=u8s8u8s32 --dir=BWD_B --skip-impl='ref:gemm' --batch=inputs/conv_all
```

Run explicitly specified 1st forward convolution (including bias) from Alexnet
with the minibatch set to 4, verbose level set to 1 for two given
configurations (`u8s8u8s32` and `f32`):
```
    $ ./benchdnn --conv -v1 \
        --mb=4 --dir=FWD_B \
        --cfg=u8s8u8s32 ic3ih227iw227_oc96oh55ow55_kh11kw11_sh4sw4ph0pw0_n"alexnet:conv1" \
        --cfg=f32 ic3ih227iw227_oc96oh55ow55_kh11kw11_sh4sw4ph0pw0_n"alexnet:conv1"
```

Run batch file for different algorithms (assuming the file specifies only
convolutions and does not include harness options that would override any
passed on the command line). Also ignore mkldnn_unimplemented errors in case of
Winograd:
```
    $ ./benchdnn --conv \
        --alg=DIRECT --batch=convs.in \
        --allow-unimpl=true \
        --alg=WINO   --batch=convs.in \
        --alg=AUTO   --batch=convs.in 
```

Run a set of u8s8u8s32 forward convolutions without bias, skipping
reference implementations and not triggering unimplemented as an error, with
one common output scale set to 0.5 with rounding mode set to down
(via attributes):
```
    $ ./benchdnn --conv \
        --cfg=u8s8u8s32 --dir=FWD_D --skip-impl="ref" --allow-unimpl=true \
        --attr="oscale=common:.5" --batch=inputs/conv_all
```



## Usage (batch normalization harness)

```
    ./benchdnn --bnorm [harness-knobs] bnorm-desc ...
```

where *harness-knobs* are:

 - `--mb=N` override minibatch that is specified in batch normalization description, default `0` (use mb specified in bnorm-desc)
 - `--dir={FWD_D (forward data /training), FWD_I (forward data /inference), BWD_D (backward data), BWD_DW (backward data + weights)}` direction, default `FWD_D`
 - `--dt={f32, s32, ...}` base data type, default `f32`
 - `--tag={nchw, nChw16c, ...}` data layout, default `nchw`
 - `--flags=[|G|S|R]` batch normalization flags, default `none` (G -- global stats, S -- use scale shift, R -- fuse with ReLU)
 - `--attr="attr_str"` attributes (see in the convolution section above), default `""` (no attributes set)
 - `--match=regex` check only bnorm that match with regex, default is `".*"`. Notice: Windows may only interpret string arguments surrounded by double quotation marks.
 - `--skip-impl="str1[:str2]..."` skip implementation (see mkldnn_query_impl_info_str), default `""`
 - `--perf-template=template-str` set template for performance report (very similar to the convolution one)
 - `--reset` reset all the parameters set before to default one
 - `-vN|--verbose=N` verbose level, default `0`
 - `--batch=file` use options from the given file (see in subdirectory)

and *bnorm-desc* is a batch normalization description. The canonical form is:
```
    mbXicXidXihXiwXepsYnS
```
Here X is an integer number, Y is a real number, and S is a string (n stands for
name). The special symbol `_` is ignored, so it may be used as delimiter. There are
some implicit rules:
 - if mb is omitted set mb to 2

 - if iw is omitted set iw to ih (and vice versa)

 - if eps is omitted set eps to 1./16

### Performance measurements (batch normalization harness)

**benchdnn** supports a custom performance report. A template is passed via the
command line and consists of terminal and nonterminal symbols. Nonterminal
symbols are printed as-is. A description of terminal symbols is given below.
There is also a notion of modifiers (marked with @) that change the meaning of
terminal symbols; for example, the sign '-' means minimum of (in terms of time). See the
table of modifiers below.

> **Caution:** Threads must be pinned in order to get consistent frequency.

| abbreviation  | description
|:------------  |:-----------
| %d            | problem descriptor
| %D            | expanded problem descriptor (parameters in csv format)
| %n            | problem name
| %z            | direction
| %f            | flags
| %q            | data type (precision)
| %f            | data format (layout)
| %@t           | time in ms

The definition of expanded problem descriptor is: `mb,ic,id,ih,iw,eps`.

The default template can be found in bnorm/bench_bnorm.cpp and is defined as
`perf,%n,%z,%f,%q,%f,%D,%-t,%0t`. That will produce the following output
in CSV format:
```
string: perf
bnorm name
direction
batch normalization flags
base data type
batch normalization flags
expanded bnorm problem descriptor
minimum time spent in ms
average time spent in ms
```
Here is an example of performance output:
```
perf,"resnet_50:bn_conv1",FWD_D,,f32,,50,64,1,112,112,0.0625,10.7729,77.1917
```
expanded bnorm problem descriptor is `50,64,1,112,112,0.0625` in the above example.

### Examples (batch normalization harness)

Run the set of bnorms from inputs/bnorm/bnorm_resnet_50 file with default minibatch:
```
    $ ./benchdnn --bnorm \
         --batch=inputs/bnorm/bnorm_resnet_50
```

Run the same as previous but also measure performance:
```
    $ ./benchdnn --bnorm --mode=CORRnPERF \
         --batch=inputs/bnorm/bnorm_resnet_50
```


## Usage (rnn harness)

```
    ./benchdnn --rnn [harness-knobs] [rnn-desc] ...
```

where *harness-knobs* are:

 - `--prop={FWD_D (forward data), BWD_DW (backward data + weights)}` direction, default `FWD_D``
 - `--alg={VANILLA_RNN, VANILLA_LSTM, VANILLA_GRU, LBR_GRU}` algorithm, default `VANILLA_RNN``
 - `--direction={left2right, right2left, concat, sum}`  direction, default `left2right``
 - `--activation={RELU, LOGISTIC, TANH}` activation, default `RELU``
 - `--reset` reset all the parameters set before to default one 
 - `--batch=file` use options from the given file (see in subdirectory)

and *rnn-desc* is rnn description. The canonical form is:
```
 lXtXmbXsicXslcXdicXdlc
```
Here X is a number and S is a string. Some implicit rules:
 - default values: l = 1, t = 1, mb = 2, S="wip"

 - if slc/dlc/dic is undefined => slc/dlc/dic = sic

See `str2desc()` in rnn/rnn_aux.cpp
for more details and implicit rules :^)

### Performance measurements (rnn harness)


Runing rnn with performance measurememt mode will produce the following output
in CSV format:
```
string: perf
algorithm
activation function
direction
expanded rnn problem descriptor
name
time spent in ms
minimum time spent in ms
maximum time spent in ms
average time spent in ms
```
Here is an example of performance output:
```
perf,VANILLA_RNN,RELU,left2right,l1t1mb128sic512slc512dic512dlc512n""GNMT_enc-training"",time(ms):min=68.0007,max=176.006,avg=91.2686
```
expanded rnn problem descriptor is `l1t1mb128sic512slc512dic512dlc512n` in the above example.

### Examples (rnn harness)

Run the set of rnn training from inputs/rnn/rnn_training file with default minibatch:
```
    $ ./benchdnn --rnn \
         --batch=inputs/rnn/rnn_training
```

Run the same as previous but also measure performance:
```
    $ ./benchdnn --rnn --mode=CORRnPERF \
         --batch=inputs/rnn/rnn_training
```


## Usage (deconvolution harness)

```
    ./benchdnn --deconv [harness-knobs] [deconv-desc] ...
```

where *harness-knobs* are:

 - `--cfg={f32, u8s8u8s32, ...}` configuration (ref conv session above  [convolution configuration](/tests/benchdnn/README.md#convolution-configurations-also-known-as-precision-specification)), default `f32`
 - `--match=regex` check only deconvolutions that match with regex, default is `".*"`. Notice: Windows may only interpret string arguments surrounded by double quotation marks.
 - `--mb=N` override minibatch that is specified in deconvolution description, default `0` (use mb specified in deconv desc)
 - `--dir={FWD_D (forward data), FWD_B (forward data + bias),FWD_I (forward data inference), BWD_D (backward data), BWD_W (backward weights), BWD_WB (backward weights + bias)}` direction, default `FWD_B`
 - `--alg={DIRECT, WINO, AUTO}` deconvolution algorithm, default DIRECT
 - `--attr="attr_str"` deconvolution attributes (see in the convolution section above), default `""` (no attributes set)
 - `--skip-impl="str1[:str2]..."` skip implementation (see mkldnn_query_impl_info_str), default `""`
 - `--allow-unimpl=true|false` do not treat unimplemented configuration as an error, default `false`
 - `--perf-template=template-str` set template for performance report (see section *Performance measurements*)
 - `--mode=` string that contains flags for benchmark mode. Use `C` or `c` for correctness (used by default), and `P` or `p` for performance  
 - `--reset` reset all the parameters set before to default one 
 - `-vN|--verbose=N` verbose level, default `0`
 - `--batch=file` use options from the given file (see in subdirectory)

and *deconv-desc* is deconvolution description. The canonical form is:
```
    gXmbXicXihXiwXocXohXowXkhXkwXshXswXphXpwXdhXdwXnS   
```
Here X is a number and S is string (n stands for name). Some of the parameters
might be omitted if a default exists (e.g. if g is not specified
**benchdnn** uses 1) or if the can be computed automatically (e.g. output shape
can be derived from the input one and kernel). Also if either width or height
is not specified than it is assumed height == width. Special symbol `_` is
ignored, hence maybe used as delimiter. See `str2desc()` in conv/conv_aux.cpp
for more details and implicit rules :^)


### Performance measurements (deconvolution harness)

**benchdnn** supports a custom performance report. please refer above Performance measurements convolution harness session for detail, [convolution harness](/tests/benchdnn/README.md#performance-measurements-convolution-harness).

The default template can be found in conv/bench_deconv.cpp and is defined as
`perf,%n,%d,%GO,%GF,%-t,%-Gp,%0t,%0Gp`. That will produce the following output
in CSV format:
```
string: perf
deconvolution name
full deconv-desc
number of giga ops calculated
effective cpu frequency in GHz (amb clocks[min] / time[min])
minimum time spent in ms
best gigaops (since it corresponds to mimimum time)
average time spent in ms
average gigaops (since it corresponds to average time)
```
Here is an example of performance output:
```
 perf,"alexnet:deconv1",mb256ic96ih55oc3oh227kh11sh4n"alexnet:deconv1",2.9733,0,249.474,11.9183,307.702,9.66291
```
full deconvolution descriptor is `mb256ic96ih55oc3oh227kh11sh4n"alexnet:deconv1"` in the above example.

### Examples (deconvolution harness)

Run the set of f32 forward deconvolutions from inputs/deconv_all file w/ bias and default minibatch:
```
    $ ./benchdnn --deconv \
        --cfg=f32 --dir=FWD_B --batch=inputs/deconv_all
```

Run the same as previous but also measure performance:
```
    $ ./benchdnn --deconv  --mode=CORRnPERF \
        --cfg=f32 --dir=FWD_B  --batch=inputs/deconv_all
```

## Usage (ip harness)

```
    ./benchdnn --ip [harness-knobs] [ip-desc] ...
```

where *harness-knobs* are:

 - `--cfg={f32, u8s8u8s32, ...}` configuration (ref conv session above  [convolution configuration](/tests/benchdnn/README.md#convolution-configurations-also-known-as-precision-specification)), default `f32``
 - `--mb=N` override minibatch that is specified in ip description, default `0` (use mb specified in ip desc)
 - `--dir={FWD_D (forward data), FWD_B (forward data + bias),FWD_I (forward data inference), BWD_D (backward data), BWD_W (backward weights), BWD_WB (backward weights + bias)}` direction, default `FWD_B`
 - `--attr="attr_str"` ip attributes (see in the convolution section above), default `""` (no attributes set)
 - `--allow-unimpl=true|false` do not treat unimplemented configuration as an error, default `false`
 - `--perf-template=template-str` set template for performance report (see section *Performance measurements*)
 - `--mode=` string that contains flags for benchmark mode. Use `C` or `c` for correctness (used by default), and `P` or `p` for performance  
 - `--reset`  reset all the parameters set before to default one 
 - `-vN|--verbose=N` verbose level, default `0`
 - `--batch=file` use options from the given file (see in subdirectory)

and *ip-desc* is ip description. The canonical form is:
```
    mbXicXidXihXiwXSocXnS   
```
Here X is a number and S is a string (n stands for name). 
The special symbol `_` is ignored, so it may be used as a delimiter. 
Some implicit rules:
 - default values:  mb = 2, id = 1, S="wip"

 - if H is undefined => H = W

 - if W is undefined => W = H

See `str2desc()` in ip/ip_aux.cpp
for more details and implicit rules :^)

### Performance measurements (ip harness)

**benchdnn** supports a custom performance report. A template is passed via the
command line and consists of terminal and nonterminal symbols. Nonterminal
symbols are printed as-is. A description of terminal symbols is given below.
There is also a notion of modifiers (marked with @) that change the meaning of
terminal symbols; for example, the sign '-' means minimum of (in terms of time). See the
table of modifiers below.

> **Caution:** Threads must be pinned in order to get consistent frequency.

| abbreviation  | description
|:------------  |:-----------
| %d            | problem descriptor
| %D            | expanded problem descriptor (parameters in csv format)
| %n            | problem name
| %z            | direction
| %f            | flags
| %q            | data type (precision)
| %f            | data format (layout)
| %@t           | time in ms

The definition of expanded problem descriptor is: `mb,oc,ic,id,ih,iw`.

The default template can be found in bnorm/bench_ip.cpp and is defined as
`perf,%D,%n,%z,%q,%-t,%-Gp,%0t,%0Gp`. That will produce the following output
in CSV format:
```
string: perf
expanded ip problem descriptor
name
direction
data type
minimum time spent in ms
best gigaops (since it corresponds to mimimum time)
average time spent in ms
average gigaops (since it corresponds to average time)
```

Here is an example of performance output:
```
perf,112,1000,2048,1,1,1,"resnet:ip1",FWD_B,f32,3.99976,114.695,19.0323,24.1039
```
expanded ip problem descriptor is `112,1000,2048,1,1,1` in the above example.

### Examples (ip harness)

Run the set of ip from inputs/ip/ip_all file with default minibatch:
```
    $ ./benchdnn --ip \
         --batch=inputs/ip/ip_all
```

Run the same as previous but also measure performance:
```
    $ ./benchdnn --ip --mode=CORRnPERF \
         --batch=inputs/ip/ip_all
```

## Usage (shuffle harness)

```
    ./benchdnn --shuffle [harness-knobs]  [dim]...
```

where *harness-knobs* are:

 - `--match==regex` check only shuffle that match with regex, default is `".*"`. Notice: Windows may only interpret string arguments surrounded by double quotation marks.
 - `--dir={FWD_D (forward data), FWD_B (forward data + bias),FWD_I (forward data inference), BWD_D (backward data), BWD_W (backward weights), BWD_WB (backward weights + bias)}` direction, default `FWD_B`
 - `--dt={f32, s32, ...}` base data type, default `f32`
 - `--tag={nchw, nChw16c, ...}` data layout, default `nchw`
 - `--axis=` default `1`
 - `--group=` default `1`
 - `--mode=` string that contains flags for benchmark mode. Use `C` or `c` for correctness (used by default), and `P` or `p` for performance 
 - `-vN|--verbose=N` verbose level, default `0`
 - `--batch=file` use options from the given file (see in subdirectory)

and *dim* is ip description. The canonical form is:
```
    dxdxdxdxd   
```
Here d is a number.

See `str2dims()` in shuffle/shuffle_aux.cpp for more details.

### Performance measurements (shuffle harness)

**benchdnn** supports a custom performance report. A template is passed via the
command line and consists of terminal and nonterminal symbols. Nonterminal
symbols are printed as-is. A description of terminal symbols is given below.
There is also a notion of modifiers (marked with @) that change the meaning of
terminal symbols; for example, the sign '-' means minimum of (in terms of time). See the
table of modifiers below.

> **Caution:** Threads must be pinned in order to get consistent frequency.

| Abbreviation  | Description
|:------------  |:-----------
| %d            | problem descriptor
| %D            | expanded problem descriptor (parameters in csv format)
| %z            | direction
| %q            | data type (precision)
| %f            | data format (layout)
| %a            | axis
| %g            | group size
| %@t           | time in ms

The definition of expanded problem descriptor is: `dxdxdxdxd`.

The default template can be found in shuffle/bench_shuffle.cpp and is defined as
`perf,%z,%q,%f,%D,%a,%g,%-t,%0t`. That will produce the following output
in CSV format:
```
string: perf
direction
data type
data format
expanded shuffle problem descriptor
axis
group size
minimum time spent in ms
average time spent in ms
```
Here is an example of performance output.
```
perf,FWD_D,u8,nCdhw16c,1x272x2x56x56,4,4,11.6177,16.509
```
expanded shuffle problem descriptor is `1x272x2x56x56` in the above example.

### Examples (shuffle harness)

Run the set of shuffle from inputs/shuffle/test_shuffle_axis file with default minibatch:
```
    $ ./benchdnn --shuffle \
         --batch=inputs/shuffle/test_shuffle_axis
```

Run the same as previous but also measure performance:
```
    $ ./benchdnn --shuffle --mode=CORRnPERF \
         --batch=inputs/shuffle/test_shuffle_axis
```

## Usage (reorder harness)

```
    ./benchdnn --reorder [harness-knobs]  ...
```

where *harness-knobs* are:

 - `--idt={f32, s32, ...}` base input data type, default `f32`
 - `--odt={f32, s32, ...}` base output data type, default `f32`
 - `--dt={f32, s32, ...}` base data type, default `f32`
 - `--itag={nchw, nChw16c, ...}` input data layout, default `nchw`
 - `--otag={nchw, nChw16c, ...}` output data layout, default `nchw`
 - `--tag={nchw, nChw16c, ...}` data layout, default `nchw`
 - `--def-scales={,,}` input defined scales. separate number by ',' ex : 0.125, 0.25, 0.5, 1, 2, 4, 8
 - `--attr="attr_str"` ip attributes (see in the section below), default `""` (no attributes set)
 - `--both-dir-dt=true|false` , default `false`
 - `--both-dir-tag=true|false` , default `false`
 - `--allow-unimpl=true|false` do not treat unimplemented configuration as an error, default `false`
 - `--run` run reorder bench
 - `--perf-template=template-str` set template for performance report (see section *Performance measurements*)
 - `--reset` reset all the parameters set before to default one 
 - `--mode=` string that contains flags for benchmark mode. Use `C` or `c` for correctness (used by default), and `P` or `p` for performance 
 - `-vN|--verbose=N` verbose level, default `0`
 - `--batch=file` use options from the given file (see in subdirectory)

### Performance measurements (reorder harness)

**benchdnn** supports a custom performance report. A template is passed via the
command line and consists of terminal and nonterminal symbols. Nonterminal
symbols are printed as-is. A description of terminal symbols is given below.
There is also a notion of modifiers (marked with @) that change the meaning of
terminal symbols; for example, the sign '-' means minimum of (in terms of time). See the
table of modifiers below.

> **Caution:** Threads must be pinned in order to get consistent frequency.

| abbreviation  | description
|:------------  |:-----------
| %d            | problem descriptor
| %D            | expanded problem descriptor (reorder parameters in csv format)
| %n            | dimensionality of the problem
| %@O           | number of elements being reordered
| %@t           | time in ms
| %@p           | elements per second

| modifier  | description
|:--------  |:-----------
|           | default
| -         | min (time) -- default
| 0         | avg (time)
| +         | max (time)
|           |
| K         | Kilo (1e3)
| M         | Mega (1e6)
| G         | Giga (1e9)

The definition of expanded problem descriptor is:
`idt,odt,itag,otag,attrs,dims`.

The default template can be found in reorder/bench_reorder.cpp and is defined as
`perf,%n,%D,%O,%-t,%-Gp,%0t,%0Gp`. That will produce the following output
in CSV format:
```
string: perf
dimensionality of the problem
expanded reorder problem descriptor
number of elements being reordered
minimum time spent in ms
best gigaops (since it corresponds to mimimum time)
average time spent in ms
average gigaops (since it corresponds to average time)
```
Here is an example of performance output:
```
 perf,4,f32,f32,nchw,nchw,oscale=per_oc:0.125;post_ops='',2x64x3x3,1152,4.00244,0.000287824,24.0279,4.79442e-05
```
expanded reorder problem descriptor is `f32,f32,nchw,nchw,oscale=per_oc:0.125;post_ops='',2x64x3x3` in the above example.

### Examples (reorder harness)

Run the set of reorder from reorder/test_default file with default minibatch:
```
    $ ./benchdnn --reorder \
        --batch=inputs/reorder/test_default
```

Run the same as previous but also measure performance:
```
    $ ./benchdnn --reorder  --mode=CORRnPERF \
        --batch=inputs/reorder/test_default
```

## Usage (self harness)

```
    ./benchdnn --self ...
```

Check enumlation type, attributes, flags, and descriptions. 



## Installation

**benchdnn** is automatically built with Intel MKL-DNN. For convenience, you can
build **benchdnn** using cmake or make.


## Essence of convolution testing

Intel MKL-DNN supports different data types, such as single-precision floating
point (`mkldnn_f32`) and signed/unsigned integer of different length
(`mkldnn_{s,u}{8,16,32}`). We need to cover all those cases with tests. It is
essential to test real convolution sizes, because Intel MKL-DNN provides
different optimizations depending on convolution parameters. There is no
single unified approach inside, so it would not be enough to test only a few
convolutions (also known as unit tests).

But even for a given convolution, the correctness convolution test is not as
simple as it might seem at first sight. One of the biggest problems we
encountered is numerical instability. For every output point, a lot of
operations may occur. For instance, on backward propagation with respect to
filter, each filter point requires `mb * oh * ow` operations (see the *Notation*
section below). That large amount of compute operations may lead to either
integer overflow or accuracy loss if initial data was chosen inadequately.

These two main issues complicate testing. **benchdnn** tries to address these
by using integers for initialization with uniform distribution in a
range `[cfg->f_min .. cfg->f_max]`, with the step `cfg->f_step`
(see `struct dt_conf_t` in conv/conv.hpp). `f_min` and `f_max` are chosen so
that most of the results would belong in the `[cfg->min .. cfg->max]` range. Also
for floating point all integers in both ranges have exact representation (that is,
the absolute numbers are less than `2^size_of_mantissa`). Uniform distribution
leads to results that are uniformly distributed and quite small. `f_min/f_max` keep
the result in a reasonable range. Yet another trick: not all the points are
initialized with non-zero values: see `fill_{src,wei,bia,dst}` in
conv/conv.cpp.


## Further plans

Please see TODO.md in the **benchdnn** root directory for development plans.


## Issues and contributions

We welcome community contributions to **benchdnn** as well as to Intel MKL-DNN.
If you have any ideas or issues please submit an issue or pull request. For
clarity, please include ''benchdnn: '' in the title.


## Inspiration

bench{yet another 3 letters where the first one equals second)...
