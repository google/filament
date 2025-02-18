enable chromium_internal_input_attachments;

@group(0) @binding(0) var<storage, read_write> prevent_dce : vec4<u32>;

@input_attachment_index(3) @group(1) @binding(0) var arg_0 : input_attachment<u32>;

fn inputAttachmentLoad_fc4d97() -> vec4<u32> {
  var res : vec4<u32> = inputAttachmentLoad(arg_0);
  return res;
}

@fragment
fn fragment_main() {
  prevent_dce = inputAttachmentLoad_fc4d97();
}
