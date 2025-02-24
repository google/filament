# Queue Present Wait Semaphore Management

The following shorthand notations are used throughout this document:

- PE: Presentation Engine
- ANI: vkAcquireNextImageKHR
- QS: vkQueueSubmit
- QP: vkQueuePresentKHR
- W: Wait
- S: Signal
- R: Render
- P: Present
- SN: Semaphore N
- IN: Swapchain image N

---

## Introduction

Vulkan requires the application (ANGLE in this case) to acquire swapchain images and queue them for
presentation, synchronizing GPU submissions with semaphores.  A single frame looks like the
following:

    CPU: ANI  ... QS   ... QP
         S:S1     W:S1     W:S2
                  S:S2
    GPU:          <------------ R ----------->
     PE:                                      <-------- P ------>

That is, the GPU starts rendering after submission, and the presentation is started when rendering is
finished.  Note that Vulkan tries to abstract a large variety of PE architectures, some of which do
not behave in a straight-forward manner.  As such, ANGLE cannot know what the PE is exactly doing
with the images or when the images are visible on the screen.  The only signal out of the PE is
received through the semaphore and fence that's used in ANI.

The issue is that, in the above diagram, it's unclear when S2 can be destroyed or recycled.  That
happens when rendering (R) is finished and before present (P) starts.  As a result, this time has to
be inferred by a future operation.

## Determining When a QP Semaphore is Waited On

The ANI call takes a semaphore, that is signaled once the image is acquired.  When that happens, it
can be inferred that the previous presentation of the image is done, which in turn implies that its
associated wait semaphore is no longer in use.

Assuming both ANI calls below return the same index:

    CPU: ANI  ... QS   ... QP         ANI  ... QS   ... QP
         S:S1     W:S1     W:S2       S:S3     W:S3     W:S4
                  S:S2                         S:S4
    GPU:          <------ R ------>            <------ R ------>
     PE:                           <-- P -->                    <-- P -->

The following holds:

    S3 is signaled
    => The PE has handed the image to the application
    => The PE is no longer presenting the image (the first P operation is finished)
    => The PE is done waiting on S2

At this point, we can destroy or recycle S2.  To implement this, a history of present operations is
maintained, which includes the wait semaphore used with that presentation.  Associated with each
present operation, is a QueueSerial that is used to determine when that semaphore can be destroyed.
This QueueSerial has execution dependency on the ANI semaphore (QS W:S3 in the above diagram).

Since the QueueSerial is not actually known at present time (QP), the present operation is kept in
history without an associated QueueSerial.  Once same index is presented again, the QueueSerial of
QS before QP is associated with the previous QP of that index.

At each present call, the present history is inspected.  Any present operation whose QueueSerial is
finished is cleaned up.

ANI fence cannot always be used instead of QueueSerial with dependency on ANI semaphore.  This is
because with Shared Present mode, the fence and semaphore are expected to be already signaled on the
second and later ANI calls.

## Swapchain recreation

When recreating the swapchain, all images are eventually freed and new ones are created, possibly
with a different count and present mode.  For the old swapchain, we can no longer rely on a future
ANI to know when a previous presentation's semaphore can be destroyed, as there won't be any more
acquisitions from the old swapchain.  Similarly, we cannot know when the old swapchain itself can be
destroyed.

ANGLE resolves this issue by deferring the destruction of the old swapchain and its remaining
present semaphores to the time when the semaphore corresponding to the first present of the new
swapchain can be destroyed.  Because once the first present semaphore of the new swapchain can be
destroyed, the first present operation of the new swapchain is done, which means the old swapchain
is no longer being presented.

Note that the swapchain may be recreated without a second acquire.  This means that the swapchain
could be recreated while there are pending old swapchains to be destroyed.  The destruction of both
old swapchains must now be deferred to when the first QP of the new swapchain has been processed.
If an application resizes the window constantly and at a high rate, ANGLE would keep accumulating
old swapchains and not free them until it stops.

## VK_EXT_swapchain_maintenance1

With the VK_EXT_swapchain_maintenance1, all the above is unnecessary.  Each QP operation can have an
associated fence, which can be used to know when the semaphore associated with it can be recycled.
The old swapchains are destroyed when QP fences are signaled.
