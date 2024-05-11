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

#ifndef ADVANCED_FRONT_H
#define ADVANCED_FRONT_H

#include "../common/shapes.h"

namespace p2t {

struct Node;

// Advancing front node
struct Node {
  Point* point;
  Triangle* triangle;

  Node* next;
  Node* prev;

  double value;

  Node(Point& p) : point(&p), triangle(NULL), next(NULL), prev(NULL), value(p.x)
  {
  }

  Node(Point& p, Triangle& t) : point(&p), triangle(&t), next(NULL), prev(NULL), value(p.x)
  {
  }

};

// Advancing front
class AdvancingFront {
public:

AdvancingFront(Node& head, Node& tail);
// Destructor
~AdvancingFront();

Node* head();
void set_head(Node* node);
Node* tail();
void set_tail(Node* node);
Node* search();
void set_search(Node* node);

/// Locate insertion point along advancing front
Node* LocateNode(double x);

Node* LocatePoint(const Point* point);

private:

Node* head_, *tail_, *search_node_;

Node* FindSearchNode(double x);
};

inline Node* AdvancingFront::head()
{
  return head_;
}
inline void AdvancingFront::set_head(Node* node)
{
  head_ = node;
}

inline Node* AdvancingFront::tail()
{
  return tail_;
}
inline void AdvancingFront::set_tail(Node* node)
{
  tail_ = node;
}

inline Node* AdvancingFront::search()
{
  return search_node_;
}

inline void AdvancingFront::set_search(Node* node)
{
  search_node_ = node;
}

}

#endif