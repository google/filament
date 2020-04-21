## Descriptions

### Constants

* Mesh encoding methods
  * 0: MESH_SEQUENTIAL_ENCODING
  * 1: MESH_EDGEBREAKER_ENCODING

* Metadata constants
  * 32768: METADATA_FLAG_MASK

* Sequential attribute encoding methods
  * 0: SEQUENTIAL_ATTRIBUTE_ENCODER_GENERIC
  * 1: SEQUENTIAL_ATTRIBUTE_ENCODER_INTEGER
  * 2: SEQUENTIAL_ATTRIBUTE_ENCODER_QUANTIZATION
  * 3: SEQUENTIAL_ATTRIBUTE_ENCODER_NORMALS

* Sequential indices encoding methods
  * 0: SEQUENTIAL_COMPRESSED_INDICES
  * 1: SEQUENTIAL_UNCOMPRESSED_INDICES

* Prediction encoding methods
  * -2: PREDICTION_NONE
  * 0: PREDICTION_DIFFERENCE
  * 1: MESH_PREDICTION_PARALLELOGRAM
  * 4: MESH_PREDICTION_CONSTRAINED_MULTI_PARALLELOGRAM
  * 5: MESH_PREDICTION_TEX_COORDS_PORTABLE
  * 6: MESH_PREDICTION_GEOMETRIC_NORMAL

* Prediction scheme transform methods
  * 1: PREDICTION_TRANSFORM_WRAP
  * 3: PREDICTION_TRANSFORM_NORMAL_OCTAHEDRON_CANONICALIZED

* Mesh traversal methods
  * 0: MESH_TRAVERSAL_DEPTH_FIRST
  * 1: MESH_TRAVERSAL_PREDICTION_DEGREE

* Mesh attribute encoding methods
  * 0: MESH_VERTEX_ATTRIBUTE
  * 1: MESH_CORNER_ATTRIBUTE

* EdgeBreaker encoding methods
  * 0: STANDARD_EDGEBREAKER
  * 2: VALENCE_EDGEBREAKER

* EdgeBreaker constants
  * -1: kInvalidCornerIndex
  * 0: LEFT_FACE_EDGE
  * 1: RIGHT_FACE_EDGE
  * 2: kTexCoordsNumComponents
  * 4: kMaxNumParallelograms
  * 3: kMaxPriority

* EdgeBreaker bit pattern constants
  * 0: TOPOLOGY_C
  * 1: TOPOLOGY_S
  * 3: TOPOLOGY_L
  * 5: TOPOLOGY_R
  * 7: TOPOLOGY_E

* Valence EdgeBreaker constants
  * 2: MIN_VALENCE
  * 7: MAX_VALENCE
  * 6: NUM_UNIQUE_VALENCES

* ANS constants
  * 256: rabs_ans_p8_precision
  * 1024: rabs_ans_p10_precision
  * 4096: rabs_l_base
  * 256: IO_BASE
  * 4096: L_RANS_BASE
  * 16384: TAGGED_RANS_BASE
  * 4096: TAGGED_RANS_PRECISION

* Symbol encoding methods
  * 0: TAGGED_SYMBOLS
  * 1: RAW_SYMBOLS


### Variables

#### Header
* draco_string
  * Must equal "DRACO"
* major_version
  * bitstream major version number
* minor_version
  * bitstream minor version number
* encoder_type
  * 0: POINT_CLOUD
  * 1: TRIANGULAR_MESH
* encoder_method
  * 0: MESH_SEQUENTIAL_ENCODING
  * 1: MESH_EDGEBREAKER_ENCODING
* flags

#### Metadata
* num_att_metadata
  * Attribute metadata count
* att_metadata_id
  * Array of attribute metadata ids
* att_metadata
  * Array of attribute metadata
* file_metadata
  * Global metadata

#### Sequential Encoding
* num_points
  * Number of encoded points
* connectivity_method

#### EdgeBreaker Encoding
* edgebreaker_traversal_type
  * 0: MeshEdgeBreakerTraversalDecoder
  * 1: MeshEdgeBreakerTraversalPredictiveDecoder
  * 2: MeshEdgeBreakerTraversalValenceDecoder
* num_new_vertices
  * Number of new vertices
* num_encoded_vertices
  * Number of encoded vertices
* num_faces
  * Number of encoded faces
* num_attribute_data
  * Number of encoded attributes
* num_encoded_symbols
  * Number of encoded EdgeBreaker symbols
* num_encoded_split_symbols
  * Number of encoded EdgeBreaker split symbols
* encoded_connectivity_size
  * Size of encoded connectivity data in bytes
* num_topology_splits
* source_id_delta
  * Array of delta encoded source symbol ids
* split_id_delta
  * Array of delta encoded split symbol ids
* source_edge_bit
  * Array of source edge types
  * 0: LEFT_FACE_EDGE
  * 1: RIGHT_FACE_EDGE
* source_symbol_id
  * Array of source symbol ids
* split_symbol_id
  * Array of split symbol ids
* last_symbol_
  * Last EdgeBreaker symbol decoded
* last_vert_added
  * Id of the last vertex decoded
* active_corner_stack
  * Array of current working corners used during EdgeBreaker decoding
* edge_breaker_symbol_to_topology_id
  * Array of EdgeBreaker symbols
  * 0: TOPOLOGY_C
  * 1: TOPOLOGY_S
  * 2: TOPOLOGY_L
  * 3: TOPOLOGY_R
  * 4: TOPOLOGY_E
* topology_split_id
  * List of decoder split ids encountered during a topology split.
* split_active_corners
  * List of corners encountered during a topology split.

#### EdgeBreaker Traversal
* eb_symbol_buffer_size
* eb_symbol_buffer
  * Standard EdgeBreaker encoded symbol data
* eb_start_face_buffer_prob_zero
    * Face configuration encoded probability
* eb_start_face_buffer_size
* eb_start_face_buffer
  * EdgeBreaker encoded face configuration data
* attribute_connectivity_decoders_prob_zero
  * Array of encoded attribute probabilities
* attribute_connectivity_decoders_size
  * Array of attribute connectivity size
* attribute_connectivity_decoders_buffer
  * Array of attribute connectivity data

#### EdgeBreaker Valence Traversal
* ebv_context_counters
  * Array of number of context symbols
* ebv_context_symbols
  * Array of encoded context symbol data
* active_context_
  * Index to the current valence
* vertex_valences_
  * Array of current vertices valences

#### Attribute Encoding
* num_attributes_decoders
* att_dec_data_id
  * Array of attribute decoder ids
* att_dec_decoder_type
  * Array of attribute decoder types
  * 0: MESH_VERTEX_ATTRIBUTE
  * 1: MESH_CORNER_ATTRIBUTE
* att_dec_traversal_method
  * Array of attribute traversal methods
  * 0: MESH_TRAVERSAL_DEPTH_FIRST
  * 1: MESH_TRAVERSAL_PREDICTION_DEGREE
* att_dec_num_values_to_decode
  * Number of values to decode per attribute
* att_dec_num_attributes
  * Array of number of attributes encoded per attribute type
* att_dec_att_type
* att_dec_data_type
  * Attribute's data type
* att_dec_num_components
  * Attribute's component count
* att_dec_normalized
* att_dec_unique_id
  * Attribute's unique encoded id
* seq_att_dec_decoder_type
  * Array of attribute encoding type
  * 0: SEQUENTIAL_ATTRIBUTE_ENCODER_GENERIC
  * 1: SEQUENTIAL_ATTRIBUTE_ENCODER_INTEGER
  * 2: SEQUENTIAL_ATTRIBUTE_ENCODER_QUANTIZATION
  * 3: SEQUENTIAL_ATTRIBUTE_ENCODER_NORMALS
* seq_att_dec_prediction_scheme
  * Array of attribute prediction scheme method
  * -2: PREDICTION_NONE
  * 0: PREDICTION_DIFFERENCE
  * 1: MESH_PREDICTION_PARALLELOGRAM
  * 4: MESH_PREDICTION_CONSTRAINED_MULTI_PARALLELOGRAM
  * 5: MESH_PREDICTION_TEX_COORDS_PORTABLE
  * 6: MESH_PREDICTION_GEOMETRIC_NORMAL
* seq_att_dec_prediction_transform_type
  * Array of attribute prediction transform method
  * 1: PREDICTION_TRANSFORM_WRAP
  * 3: PREDICTION_TRANSFORM_NORMAL_OCTAHEDRON_CANONICALIZED
* seq_int_att_dec_compressed
* pred_cons_multi_is_cease_edge
  * Array for multi parallelogram prediction signifying if the edge is the last edge
* pred_tex_coords_orientations
  * Array signifying orientation for the texture coordinate prediction
* pred_trasnform_wrap_min
  * Array of minimum clamp values used by the wrap transform
* pred_trasnform_wrap_max
  * Array of maximum clamp values used by the wrap transform
* pred_trasnform_normal_max_q_val
  * Maximum quantization array used by the normal transform
* pred_transform_normal_flip_normal_bits
  * Array of flags used by the normal transform
* seq_int_att_dec_decoded_values
  * Array of attribute decoded symbols
* seq_int_att_dec_symbols_to_signed_ints
  * Array of decoded symbols converted to signed ints
* seq_int_att_dec_original_values
  * Array containing the attribute's original quantized values
* seq_int_att_dec_dequantized_values
  * Array containing the attribute's original values
* quantized_data_min_values
  * Array of minimum quantization values
* quantized_data_max_value_df
  * Array of quantization range
* quantized_data_quantization_bits
  * Array of number of quantization bits

#### Attribute Traversal
* curr_att_dec
  * Current attribute decoder type
* curr_att
  * Current attribute within a decoder type
* vertex_visited_point_ids
  * Array of the last vertex visited per attribute
* att_connectivity_seam_opp
* att_connectivity_seam_src
* att_connectivity_seam_dest
* corner_to_point_map
* is_edge_on_seam_
  * Array of bools signifying if the corner's opposite edge is on a seam
* encoded_attribute_value_index_to_corner_map
  * Array for storing the corner ids in the order their associated attribute entries were encoded
* vertex_to_encoded_attribute_value_index_map
  * Array for storing encoding order of attribute entries for each vertex
* indices_map_
* prediction_rans_prob_zero
  * Current rans zero probability
* prediction_rans_data_size
  * Current size of rans encoded data
* prediction_rans_data_buffer
  * Ans encoded prediction data for an attribute
* tex_coords_num_orientations
  * Current number of orientations for encoded Texture data
* traversal_stacks_
  * Array of available corners
* best_priority_
  * Current best available priority
* prediction_degree_
  * Array of current degree prediction for each vertex
* constrained_multi_num_flags



  
