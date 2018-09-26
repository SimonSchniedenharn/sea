#include "sealib/dfs.h"
#include <iostream>
#include "./inplacerunner.h"
#include "./segmentstack.h"

using Sealib::DFS;
using Sealib::SegmentStack;
using Sealib::BasicSegmentStack;
using Sealib::ExtendedSegmentStack;
using Sealib::CompactArray;
using Sealib::Graph;
using Sealib::Node;

static void process_standard(uint u0, Graph *g, uint *color,
                             UserFunc1 preProcess, UserFunc2 preExplore,
                             UserFunc2 postExplore, UserFunc1 postProcess) {
  std::stack<Pair> *s = new std::stack<Pair>;
  s->push(Pair(u0, 0));
  while (!s->empty()) {
    Pair x = s->top();
    s->pop();
    uint u = x.head();
    uint k = x.tail();
    if (color[u] == DFS_WHITE) {
      if (preProcess != DFS_NOP_PROCESS) preProcess(u);
      color[u] = DFS_GRAY;
    }
    if (k < g->getNodeDegree(u)) {
      s->push(Pair(u, k + 1));
      uint v = g->head(u, k);
      if (preExplore != DFS_NOP_EXPLORE) preExplore(u, v);
      if (color[v] == DFS_WHITE) {
        s->push(Pair(v, 0));
      } else {
        if (postExplore != DFS_NOP_EXPLORE) postExplore(u, v);
      }
    } else {
      if (postExplore != DFS_NOP_EXPLORE && u != u0) {
        uint pu = s->top().head();
        postExplore(pu, u);
      }
      color[u] = DFS_BLACK;
      if (postProcess != DFS_NOP_PROCESS) postProcess(u);
    }
  }
  delete s;
}

template <class SS>
static void process_small(uint u0, Graph *g, CompactArray *color, SS *s,
                          void (*restoration)(uint, Graph *, CompactArray *,
                                              SS *),
                          UserFunc1 preProcess, UserFunc2 preExplore,
                          UserFunc2 postExplore, UserFunc1 postProcess) {
  s->push(Pair(u0, 0));
  Pair x;
  while (!s->isEmpty()) {
    int sr = s->pop(&x);
    if (sr == DFS_NO_MORE_NODES) {
      return;
    }
    uint u, k;
    u = x.head();
    k = x.tail();
    if (color->get(u) == DFS_WHITE) {
      if (preProcess != DFS_NOP_PROCESS) preProcess(u);
      color->insert(u, DFS_GRAY);
    }
    if (k < g->getNodeDegree(u)) {
      s->push(Pair(u, k + 1));
      uint v = g->head(u, k);
      if (preExplore != DFS_NOP_EXPLORE) preExplore(u, v);
      if (color->get(v) == DFS_WHITE) {
        s->push(Pair(v, 0));
      } else {
        if (postExplore != DFS_NOP_EXPLORE) postExplore(u, v);
      }
    } else {
      if (postExplore != DFS_NOP_EXPLORE && u != u0) {
        Pair px;
        sr = s->pop(&px);
        if (sr == DFS_DO_RESTORE) {
          restoration(u0, g, color, s);
          s->pop(&px);
        }
        uint pu = px.head();
        postExplore(pu, u);
        s->push(px);
      }
      color->insert(u, DFS_BLACK);
      if (postProcess != DFS_NOP_PROCESS) postProcess(u);
    }
  }
}

static void restore_full(uint u0, Graph *g, CompactArray *color,
                         BasicSegmentStack *s) {
  s->saveTrailer();
  s->dropAll();
  for (uint a = 0; a < g->getOrder(); a++) {
    if (color->get(a) == DFS_GRAY) {
      color->insert(a, DFS_WHITE);
    }
  }
  s->push(Pair(u0, 0));
  Pair x;
  while (!s->isAligned()) {
    s->pop(&x);
    uint u = x.head(), k = x.tail();
    if (color->get(u) == DFS_WHITE) color->insert(u, DFS_GRAY);
    if (k < g->getNodeDegree(u)) {
      s->push(Pair(u, k + 1));
      if (s->isAligned()) break;
      uint v = g->head(u, k);
      if (color->get(v) == DFS_WHITE) s->push(Pair(v, 0));
    } else {
      color->insert(u, DFS_BLACK);
    }
  }
}

static void restore_top(uint u0, Graph *g, CompactArray *color,
                        ExtendedSegmentStack *s) {
  std::cout << "entering restoration\n";
  BasicSegmentStack tmp(3);
  Pair x;
  if (s->getRestoreTrailer(&x) == 1) {
    std::cout << "starting at bottom " << u0 << ",0\n";
    tmp.push(Pair(u0, 0));
  } else {
    std::cout << "starting at " << x.head() << "," << x.tail() << " \n";
    tmp.push(x);
  }
  while (!s->isAligned()) {
    tmp.pop(&x);
    uint u = x.head(), k = x.tail();
    if (k < g->getNodeDegree(u)) {
      tmp.push(Pair(u, k + 1));
      uint v = g->head(u, k);
      //std::cout << "exploring " << v << " (color: " << color->get(v) << ")\n";
      if (color->get(v) == DFS_GRAY) {
        std::cout << "gray node: " << v << "," << s->getOutgoingEdge(v) << "\n";
        tmp.push(Pair(v, s->getOutgoingEdge(v)));
        if (s->isInTopSegment(v)) {
          std::cout << " => pushing to S'\n";
          Pair p(v, s->getOutgoingEdge(v));
          s->push(p);
          color->insert(u, DFS_WHITE);
        }
      }
      if (tmp.isEmpty()) throw std::logic_error("restoration failed");
    } else {
      std::cout << "finished node " << u << "\n";
    }
  }
  s->recolorLow(DFS_GRAY);
}

void DFS::standardDFS(Graph *g, UserFunc1 preProcess, UserFunc2 preExplore,
                      UserFunc2 postExplore, UserFunc1 postProcess) {
  uint *color = new uint[g->getOrder()];
  for (uint a = 0; a < g->getOrder(); a++) color[a] = DFS_WHITE;
  for (uint u = 0; u < g->getOrder(); u++) {
    if (color[u] == DFS_WHITE) {
      process_standard(u, g, color, preProcess, preExplore, postExplore,
                       postProcess);
    }
  }
  delete[] color;
}

void DFS::nBitDFS(Graph *g, UserFunc1 preProcess, UserFunc2 preExplore,
                  UserFunc2 postExplore, UserFunc1 postProcess) {
  unsigned int n = g->getOrder();
  double e = 0.2;
  unsigned q = static_cast<unsigned>(ceil(
      ceil(e / 6 * n) /
      (8 *
       sizeof(Pair))));  // 2q entries on S shall take up at most (e/3)n bits
  unsigned qs = 3;       // stable segment size (?)
  if (q < qs) q = qs;

  // printf("e=%3.2f, q=%u, n=%u\n", e, q, n);
  BasicSegmentStack s(q);
  CompactArray color(n, 3);
  for (uint a = 0; a < n; a++) color.insert(a, DFS_WHITE);
  for (uint a = 0; a < n; a++) {
    if (color.get(a) == DFS_WHITE)
      process_small(a, g, &color, &s, restore_full, preProcess, preExplore,
                    postExplore, postProcess);
  }
}

void DFS::nloglognBitDFS(Graph *g, UserFunc1 preProcess, UserFunc2 preExplore,
                         UserFunc2 postExplore, UserFunc1 postProcess) {
  unsigned int n = g->getOrder();

  // printf("e=%3.2f, q=%u, n=%u\n", e, q, n);
  CompactArray color(n, 3);
  ExtendedSegmentStack s(n, g, &color);
  for (uint a = 0; a < n; a++) color.insert(a, DFS_WHITE);
  for (uint a = 0; a < n; a++) {
    if (color.get(a) == DFS_WHITE)
      process_small(a, g, &color, &s, restore_top, preProcess, preExplore,
                    postExplore, postProcess);
  }
}

void DFS::runLinearTimeInplaceDFS(uint *graph, UserFunc1 preProcess,
                                  UserFunc1 postProcess, uint startVertex) {
  auto *ilDFSRunner =
      new LinearTimeInplaceDFSRunner(graph, preProcess, postProcess);
  ilDFSRunner->run(startVertex);
  delete ilDFSRunner;
}
