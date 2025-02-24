# Experimental extensions

Sometimes a language feature proposed for WGSL requires experiementation
to prove its worth.  Tint needs to support these, in general to enable
that experimentation.

The steps for doing so are:

1. Choose a name for the feature, to be used in an `enable` directive.
   An experimental extension should use prefix of `google_experimental_`
   Example:

      enable google_experimental_f16;

2. Write down what the feature is supposed to mean.
   This informs the Tint implementation, and tells shader authors what
   has changed.
   Ideally, this will take the form of one of the following:

   - A PR against the WGSL spec.

   - A description of what the contents of that PR would be, committed
     as a document in this Tint repository.

3. File a tracking bug for adding the feature.
   Note: Should the Tint repo have a label for experimental features?

4. File a tracking bug for removing the feature or converting it to
   non-experimental.

5. Write a plan for removal of the experiment.
   - Ideally, this plan is committed to this repository, especially the
     description of public activities and commitments. However, we recognize
     that some internal goals or metrics may be sensitive, and can be hidden.
   - The plan is about process, not technical details.  It should include:
       - Who is the point of contact for this feature? The point of contact
         is responsible when the feature causes an issue or gets in the way.
       - What is your target date for declaring the experiment a success or
         failure. In Chrome an experiment must be shipped or removed, in
         finite time.
       - What experience are you hoping to gain?  Do you have target metrics?
       - What approvals, if any, do you need from W3C? What is your plan to
         present your case to W3C?
       - The bug tracking removal of the experiment.
