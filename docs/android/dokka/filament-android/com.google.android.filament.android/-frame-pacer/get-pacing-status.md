//[filament-android](../../../index.md)/[com.google.android.filament.android](../index.md)/[FramePacer](index.md)/[getPacingStatus](get-pacing-status.md)

# getPacingStatus

[main]\
open fun [getPacingStatus](get-pacing-status.md)(): [FramePacer.PacingStatus](-pacing-status/index.md)

Returns the current flow control status of the pacing pipeline. If the pipeline is DISPLAY_STARVING, the application may choose to recover by skipping a frame to rebuild queue depth and calling resetPacing().

#### Return

The active PacingStatus.
