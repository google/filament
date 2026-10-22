## EdgeBreaker Traversal Prediction Degree

### AddCornerToTraversalStack()

~~~~~
void AddCornerToTraversalStack(ci, priority) {
  traversal_stacks_[priority].push_back(ci);
  if (priority < best_priority_)
    best_priority_ = priority;
}
~~~~~
{:.draco-syntax }


### ComputePriority()

~~~~~
int ComputePriority(corner_id) {
  CornerToVerts(curr_att_dec, corner_id, &v_tip, &next_vert, &prev_vert);
  priority = 0;
  if (!is_vertex_visited_[v_tip]) {
    degree = ++prediction_degree_[v_tip];
    priority = (degree > 1 ? 1 : 2);
  }
  if (priority >= kMaxPriority)
    priority = kMaxPriority - 1;
  return priority;
}
~~~~~
{:.draco-syntax }


### PopNextCornerToTraverse()

~~~~~
int PopNextCornerToTraverse() {
  for (i = best_priority_; i < kMaxPriority; ++i) {
    if (!traversal_stacks_[i].empty()) {
      ret = traversal_stacks_[i].pop_back();
      best_priority_ = i;
      return ret;
    }
  }
  return kInvalidCornerIndex;
}
~~~~~
{:.draco-syntax }


### PredictionDegree_TraverseFromCorner()

~~~~~
void PredictionDegree_TraverseFromCorner(corner_id) {
  traversal_stacks_[0].push_back(corner_id);
  best_priority_ = 0;
  CornerToVerts(curr_att_dec, corner_id, &tip_vertex, &next_vert, &prev_vert);
  if (!is_vertex_visited_[next_vert]) {
    is_vertex_visited_[next_vert] = true;
    next_c = Next(corner_id);
    OnNewVertexVisited(next_vert, next_c);
  }
  if (!is_vertex_visited_[prev_vert]) {
    is_vertex_visited_[prev_vert] = true;
    prev_c = Previous(corner_id);
    OnNewVertexVisited(prev_vert, prev_c);
  }
  if (!is_vertex_visited_[tip_vertex]) {
    is_vertex_visited_[tip_vertex] = true;
    OnNewVertexVisited(tip_vertex, corner_id);
  }
  while ((corner_id = PopNextCornerToTraverse()) >= 0) {
    face_id = corner_id / 3;
    if (IsFaceVisited(face_id)) {
      continue;
    }
    while (true) {
      face_id = corner_id / 3;
      is_face_visited_[face_id] = true;
      CornerToVerts(curr_att_dec, corner_id, &vert_id, &next_vert, &prev_vert);
      if (!is_vertex_visited_[vert_id]) {
        is_vertex_visited_[vert_id] = true;
        OnNewVertexVisited(vert_id, corner_id);
      }
      right_corner_id = GetRightCorner(corner_id);
      left_corner_id = GetLeftCorner(corner_id);
      right_face_id = right_corner_id < 0 ? -1 : right_corner_id / 3;
      left_face_id = left_corner_id < 0 ? -1 : left_corner_id / 3;
      is_right_face_visited = IsFaceVisited(right_face_id);
      is_left_face_visited = IsFaceVisited(left_face_id);
      if (!is_left_face_visited) {
        priority = ComputePriority(left_corner_id);
        if (is_right_face_visited && priority <= best_priority_) {
          corner_id = left_corner_id;
          continue;
        } else {
          AddCornerToTraversalStack(left_corner_id, priority);
        }
      }
      if (!is_right_face_visited) {
        priority = ComputePriority(right_corner_id);
        if (priority <= best_priority_) {
          corner_id = right_corner_id;
          continue;
        } else {
          AddCornerToTraversalStack(right_corner_id, priority);
        }
      }
      break;
    }
  }
}
~~~~~
{:.draco-syntax }
