/*******************************************************************************
*                                                                              *
* Author    :  Angus Johnson                                                   *
* Version   :  4.8.8                                                           *
* Date      :  30 August 2012                                                  *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2012                                         *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
* Attributions:                                                                *
* The code in this library is an extension of Bala Vatti's clipping algorithm: *
* "A generic solution to polygon clipping"                                     *
* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
* http://portal.acm.org/citation.cfm?id=129906                                 *
*                                                                              *
* Computer graphics and geometric modeling: implementation and algorithms      *
* By Max K. Agoston                                                            *
* Springer; 1 edition (January 4, 2005)                                        *
* http://books.google.com/books?q=vatti+clipping+agoston                       *
*                                                                              *
* See also:                                                                    *
* "Polygon Offsetting by Computing Winding Numbers"                            *
* Paper no. DETC2005-85513 pp. 565-575                                         *
* ASME 2005 International Design Engineering Technical Conferences             *
* and Computers and Information in Engineering Conference (IDETC/CIE2005)      *
* September 24-28, 2005 , Long Beach, California, USA                          *
* http://www.me.berkeley.edu/~mcmains/pubs/DAC05OffsetPolygon.pdf              *
*                                                                              *
*******************************************************************************/

#ifndef clipper_hpp
#define clipper_hpp

#include <vector>
#include <stdexcept>
#include <cstring>
#include <cstdlib>
#include <ostream>

namespace ClipperLib {

enum ClipType { ctIntersection, ctUnion, ctDifference, ctXor };
enum PolyType { ptSubject, ptClip };
//By far the most widely used winding rules for polygon filling are
//EvenOdd & NonZero (GDI, GDI+, XLib, OpenGL, Cairo, AGG, Quartz, SVG, Gr32)
//Others rules include Positive, Negative and ABS_GTR_EQ_TWO (only in OpenGL)
//see http://glprogramming.com/red/chapter11.html
enum PolyFillType { pftEvenOdd, pftNonZero, pftPositive, pftNegative };

typedef signed long long long64;
typedef unsigned long long ulong64;

struct IntPoint {
public:
  long64 X;
  long64 Y;
  IntPoint(long64 x = 0, long64 y = 0): X(x), Y(y) {};
  friend std::ostream& operator <<(std::ostream &s, IntPoint &p);
};

typedef std::vector< IntPoint > Polygon;
typedef std::vector< Polygon > Polygons;

std::ostream& operator <<(std::ostream &s, Polygon &p);
std::ostream& operator <<(std::ostream &s, Polygons &p);

struct ExPolygon {
  Polygon  outer;
  Polygons holes;
};
typedef std::vector< ExPolygon > ExPolygons;

enum JoinType { jtSquare, jtRound, jtMiter };

bool Orientation(const Polygon &poly);
double Area(const Polygon &poly);
void OffsetPolygons(const Polygons &in_polys, Polygons &out_polys,
  double delta, JoinType jointype = jtSquare, double MiterLimit = 2);
void SimplifyPolygon(const Polygon &in_poly, Polygons &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(const Polygons &in_polys, Polygons &out_polys, PolyFillType fillType = pftEvenOdd);
void SimplifyPolygons(Polygons &polys, PolyFillType fillType = pftEvenOdd);

void ReversePolygon(Polygon& p);
void ReversePolygons(Polygons& p);

//used internally ...
enum EdgeSide { esNeither = 0, esLeft = 1, esRight = 2, esBoth = 3 };
enum IntersectProtects { ipNone = 0, ipLeft = 1, ipRight = 2, ipBoth = 3 };

struct TEdge {
  long64 xbot;
  long64 ybot;
  long64 xcurr;
  long64 ycurr;
  long64 xtop;
  long64 ytop;
  double dx;
  long64 tmpX;
  PolyType polyType;
  EdgeSide side;
  int windDelta; //1 or -1 depending on winding direction
  int windCnt;
  int windCnt2; //winding count of the opposite polytype
  int outIdx;
  TEdge *next;
  TEdge *prev;
  TEdge *nextInLML;
  TEdge *nextInAEL;
  TEdge *prevInAEL;
  TEdge *nextInSEL;
  TEdge *prevInSEL;
};

struct IntersectNode {
  TEdge          *edge1;
  TEdge          *edge2;
  IntPoint        pt;
  IntersectNode  *next;
};

struct LocalMinima {
  long64        Y;
  TEdge        *leftBound;
  TEdge        *rightBound;
  LocalMinima  *next;
};

struct Scanbeam {
  long64    Y;
  Scanbeam *next;
};

struct OutPt; //forward declaration

struct OutRec {
  int     idx;
  bool    isHole;
  OutRec *FirstLeft;
  OutRec *AppendLink;
  OutPt  *pts;
  OutPt  *bottomPt;
  OutPt  *bottomFlag;
  EdgeSide sides;
};

struct OutPt {
  int     idx;
  IntPoint pt;
  OutPt   *next;
  OutPt   *prev;
};

struct JoinRec {
  IntPoint  pt1a;
  IntPoint  pt1b;
  int       poly1Idx;
  IntPoint  pt2a;
  IntPoint  pt2b;
  int       poly2Idx;
};

struct HorzJoinRec {
  TEdge    *edge;
  int       savedIdx;
};

struct IntRect { long64 left; long64 top; long64 right; long64 bottom; };

typedef std::vector < OutRec* > PolyOutList;
typedef std::vector < TEdge* > EdgeList;
typedef std::vector < JoinRec* > JoinList;
typedef std::vector < HorzJoinRec* > HorzJoinList;

//ClipperBase is the ancestor to the Clipper class. It should not be
//instantiated directly. This class simply abstracts the conversion of sets of
//polygon coordinates into edge objects that are stored in a LocalMinima list.
class ClipperBase
{
public:
  ClipperBase();
  virtual ~ClipperBase();
  bool AddPolygon(const Polygon &pg, PolyType polyType);
  bool AddPolygons( const Polygons &ppg, PolyType polyType);
  virtual void Clear();
  IntRect GetBounds();
protected:
  void DisposeLocalMinimaList();
  TEdge* AddBoundsToLML(TEdge *e);
  void PopLocalMinima();
  virtual void Reset();
  void InsertLocalMinima(LocalMinima *newLm);
  LocalMinima      *m_CurrentLM;
  LocalMinima      *m_MinimaList;
  bool              m_UseFullRange;
  EdgeList          m_edges;
};

class Clipper : public virtual ClipperBase
{
public:
  Clipper();
  ~Clipper();
  bool Execute(ClipType clipType,
  Polygons &solution,
  PolyFillType subjFillType = pftEvenOdd,
  PolyFillType clipFillType = pftEvenOdd);
  bool Execute(ClipType clipType,
  ExPolygons &solution,
  PolyFillType subjFillType = pftEvenOdd,
  PolyFillType clipFillType = pftEvenOdd);
  void Clear();
  bool ReverseSolution() {return m_ReverseOutput;};
  void ReverseSolution(bool value) {m_ReverseOutput = value;};
protected:
  void Reset();
  virtual bool ExecuteInternal(bool fixHoleLinkages);
private:
  PolyOutList       m_PolyOuts;
  JoinList          m_Joins;
  HorzJoinList      m_HorizJoins;
  ClipType          m_ClipType;
  Scanbeam         *m_Scanbeam;
  TEdge           *m_ActiveEdges;
  TEdge           *m_SortedEdges;
  IntersectNode    *m_IntersectNodes;
  bool              m_ExecuteLocked;
  PolyFillType      m_ClipFillType;
  PolyFillType      m_SubjFillType;
  bool              m_ReverseOutput;
  void DisposeScanbeamList();
  void SetWindingCount(TEdge& edge);
  bool IsEvenOddFillType(const TEdge& edge) const;
  bool IsEvenOddAltFillType(const TEdge& edge) const;
  void InsertScanbeam(const long64 Y);
  long64 PopScanbeam();
  void InsertLocalMinimaIntoAEL(const long64 botY);
  void InsertEdgeIntoAEL(TEdge *edge);
  void AddEdgeToSEL(TEdge *edge);
  void CopyAELToSEL();
  void DeleteFromSEL(TEdge *e);
  void DeleteFromAEL(TEdge *e);
  void UpdateEdgeIntoAEL(TEdge *&e);
  void SwapPositionsInSEL(TEdge *edge1, TEdge *edge2);
  bool IsContributing(const TEdge& edge) const;
  bool IsTopHorz(const long64 XPos);
  void SwapPositionsInAEL(TEdge *edge1, TEdge *edge2);
  void DoMaxima(TEdge *e, long64 topY);
  void ProcessHorizontals();
  void ProcessHorizontal(TEdge *horzEdge);
  void AddLocalMaxPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  void AddLocalMinPoly(TEdge *e1, TEdge *e2, const IntPoint &pt);
  void AppendPolygon(TEdge *e1, TEdge *e2);
  void DoEdge1(TEdge *edge1, TEdge *edge2, const IntPoint &pt);
  void DoEdge2(TEdge *edge1, TEdge *edge2, const IntPoint &pt);
  void DoBothEdges(TEdge *edge1, TEdge *edge2, const IntPoint &pt);
  void IntersectEdges(TEdge *e1, TEdge *e2,
    const IntPoint &pt, IntersectProtects protects);
  OutRec* CreateOutRec();
  void AddOutPt(TEdge *e, const IntPoint &pt);
  void DisposeBottomPt(OutRec &outRec);
  void DisposeAllPolyPts();
  void DisposeOutRec(PolyOutList::size_type index);
  bool ProcessIntersections(const long64 botY, const long64 topY);
  void AddIntersectNode(TEdge *e1, TEdge *e2, const IntPoint &pt);
  void BuildIntersectList(const long64 botY, const long64 topY);
  void ProcessIntersectList();
  void ProcessEdgesAtTopOfScanbeam(const long64 topY);
  void BuildResult(Polygons& polys);
  void BuildResultEx(ExPolygons& polys);
  void SetHoleState(TEdge *e, OutRec *OutRec);
  void DisposeIntersectNodes();
  bool FixupIntersections();
  void FixupOutPolygon(OutRec &outRec);
  bool IsHole(TEdge *e);
  void FixHoleLinkage(OutRec *outRec);
  void CheckHoleLinkages1(OutRec *outRec1, OutRec *outRec2);
  void CheckHoleLinkages2(OutRec *outRec1, OutRec *outRec2);
  void AddJoin(TEdge *e1, TEdge *e2, int e1OutIdx = -1, int e2OutIdx = -1);
  void ClearJoins();
  void AddHorzJoin(TEdge *e, int idx);
  void ClearHorzJoins();
  void JoinCommonEdges(bool fixHoleLinkages);
};

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

class clipperException : public std::exception
{
  public:
    clipperException(const char* description): m_descr(description) {}
    virtual ~clipperException() throw() {}
    virtual const char* what() const throw() {return m_descr.c_str();}
  private:
    std::string m_descr;
};
//------------------------------------------------------------------------------

} //ClipperLib namespace

#endif //clipper_hpp


