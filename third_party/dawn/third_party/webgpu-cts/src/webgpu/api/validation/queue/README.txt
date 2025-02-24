Tests for validation that occurs inside queued operations
(submit, writeBuffer, writeTexture, copyExternalImageToTexture).

BufferMapStatesToTest = {
  mapped -> unmapped,
  mapped at creation -> unmapped,
  mapping pending -> unmapped,
  pending -> mapped (await map),
  unmapped -> pending (noawait map),
  created mapped-at-creation,
}

Note writeTexture is tested in image_copy.
