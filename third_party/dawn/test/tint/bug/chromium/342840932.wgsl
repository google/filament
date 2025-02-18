@group(0) @binding(0) var image_dup_src: texture_storage_1d<r32uint,read>;
@group(0) @binding(1) var image_dst: texture_storage_1d<r32uint,write>;

// for #1307 loads with ivec2 coords.
