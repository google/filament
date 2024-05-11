/*
 * Poly2Tri Copyright (c) 2009-2010, Poly2Tri Contributors
 * http://code.google.com/p/poly2tri/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * * Neither the name of Poly2Tri nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without specific
 *   prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "advancing_front.h"

namespace p2t {

AdvancingFront::AdvancingFront(Node& head, Node& tail)
{
  head_ = &head;
  tail_ = &tail;
  search_node_ = &head;
}

Node* AdvancingFront::LocateNode(double x)
{
  Node* node = search_node_;

  if (x < node->value) {
    while ((node = node->prev) != NULL) {
      if (x >= node->value) {
        search_node_ = node;
        return node;
      }
    }
  } else {
    while ((node = node->next) != NULL) {
      if (x < node->value) {
        search_node_ = node->prev;
        return node->prev;
      }
    }
  }
  return NULL;
}

Node* AdvancingFront::FindSearchNode(double x)
{
  (void)x; // suppress compiler warnings "unused parameter 'x'"
  // TODO: implement BST index
  return search_node_;
}

Node* AdvancingFront::LocatePoint(const Point* point)
{
  const double px = point->x;
  Node* node = FindSearchNode(px);
  const double nx = node->point->x;

  if (px == nx) {
    if (point != node->point) {
      // We might have two nodes with same x value for a short time
      if (point == node->prev->point) {
        node = node->prev;
      } else if (point == node->next->point) {
        node = node->next;
      } else {
        assert(0);
      }
    }
  } else if (px < nx) {
    while ((node = node->prev) != NULL) {
      if (point == node->point) {
        break;
      }
    }
  } else {
    while ((node = node->next) != NULL) {
      if (point == node->point)
        break;
    }
  }
  if(node) search_node_ = node;
  return node;
}

AdvancingFront::~AdvancingFront()
{
}

}