// RUN: %dxc -T lib_6_8 -verify %s

// Check the WaveSize attribute emits diagnostics in a variety of cases.

struct INPUT_RECORD
{
  uint DispatchGrid1 : SV_DispatchGrid;
  uint2 a;
};



[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize(4, 16, 32)] /* expected-error{{Preferred WaveSize value 32 must be between 4 and 16}} */
void node01(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize(16, 16, 32)] /* expected-error{{Wave Size Range minimum and maximum are equal}} */
void node02(DispatchNodeInputRecord<INPUT_RECORD> input) { }

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize(16, 16, 16)] /* expected-error{{Wave Size Range minimum and maximum are equal}} */
void node03(DispatchNodeInputRecord<INPUT_RECORD> input) { }

// the non-power of 2 diagnostic gets emitted once, regardless of how many arguments aren't powers of 2.

[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
[WaveSize(15, 17, 32)]
void node04(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
/* expected-warning@+1{{attribute 'WaveSize' must have a uint literal argument}} */
[WaveSize(-15, 16, 8)] 
void node05(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize(4, 16, 8, 8)] /* expected-error{{'WaveSize' attribute takes no more than 3 arguments}} */
void node06(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
[WaveSize()] /* expected-error{{'WaveSize' attribute takes at least 1 argument}} */
void node07(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
// expected-error@+2{{WaveSize arguments must be between 4 and 128 and a power of 2}}
// expected-error@+1{{'WaveSize' attribute requires an integer constant}}
[WaveSize(4, 8, node07)]
void node08(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{shader attribute type 'wavesize' conflicts with shader attribute type 'wavesize'}} */
/* expected-note@+2{{conflicting attribute is here}} */
[WaveSize(8, 32, 8)]
[WaveSize(4, 32, 8)]
void node09(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{shader attribute type 'wavesize' conflicts with shader attribute type 'wavesize'}} */
/* expected-note@+2{{conflicting attribute is here}} */
[WaveSize(8, 32, 8)]
[WaveSize(8, 16, 8)]
void node10(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{shader attribute type 'wavesize' conflicts with shader attribute type 'wavesize'}} */
/* expected-note@+2{{conflicting attribute is here}} */
[WaveSize(4, 8, 8)]
[WaveSize(4, 8, 4)]
void node11(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
// no expected failure here
[WaveSize(4, 8, 8)]
[WaveSize(4, 8, 8)]
void node12(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{Wave Size Range minimum and maximum are equal}} */
[WaveSize(4, 4, 4)]
void node13(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
[WaveSize(0, 0, 0)]
void node14(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
/* expected-warning@+1{{attribute 'WaveSize' must have a uint literal argument}} */
[WaveSize(-2, 16, 8)]
void node15(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
/* expected-warning@+1{{attribute 'WaveSize' must have a uint literal argument}} */
[WaveSize(2, 16, 3.5)]
void node16(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
// no errors expected
[WaveSize(4, 16, 3+5)]
void node17(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
[WaveSize(2, 16, 8)]
void node18(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
[WaveSize(4, 512, 8)]
void node19(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{WaveSize arguments must be between 4 and 128 and a power of 2}} */
[WaveSize(4, 58, 128)]
void node20(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{'WaveSize' attribute takes at least 1 argument}} */
[WaveSize]
void node21(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+1{{'WaveSize' attribute takes no more than 3 arguments}} */
[WaveSize(32, 32, 32, 32)]
void node22(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{shader attribute type 'wavesize' conflicts with shader attribute type 'wavesize'}} */
/* expected-note@+2{{conflicting attribute is here}} */
[WaveSize(4, 8, 4)]
[WaveSize(4)]
void node23(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{shader attribute type 'wavesize' conflicts with shader attribute type 'wavesize'}} */
/* expected-note@+2{{conflicting attribute is here}} */
[WaveSize(32, 64, 32)]
[WaveSize(32, 64)]
void node24(DispatchNodeInputRecord<INPUT_RECORD> input) { }


[Shader("node")]
[NodeLaunch("broadcasting")]
[NumThreads(1,1,1)]
[NodeMaxDispatchGrid(32,1,1)]
/* expected-error@+2{{shader attribute type 'wavesize' conflicts with shader attribute type 'wavesize'}} */
/* expected-note@+2{{conflicting attribute is here}} */
[WaveSize(32, 64)]
[WaveSize(32)]
void node25(DispatchNodeInputRecord<INPUT_RECORD> input) { }