#ifndef SCC_H
#define SCC_H
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Analysis/DependenceAnalysis.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "DG.h"

# define RDG_VS 10

/* Values used in DFS algorithm.  */
# define VERTEX_WHITE 0
# define VERTEX_GRAY 1
# define VERTEX_BLACK 2

//////////// RDG 
#define RDG_LOOP(G)	  (G)->loop_nest
//#define RDG_IDX(G)        (G)->loop_index
#define RDG_NBV(G)        (G)->nb_vertices
#define RDG_NBE(G)        (G)->nb_edges
#define RDG_V(G)          (G)->vertices
#define RDG_VERTEX(G,i)   &((G)->vertices[i])
#define RDG_E(G)          (G)->edges
#define RDG_EDGE(G,i)     &((G)->edges[i])
#define RDG_DDV(G)        (G)->dd_vertices
#define RDG_DDR(G)        (G)->dependence_relations

//////////////// RDGV
#define RDGV_COLOR(V)      (V)->color
#define RDGV_BB(V)         (V)->bb_number
#define RDGV_N(V)          (V)->number
#define RDGV_INSTRS(V)     (V)->instrs
#define RDGV_DD_P(V)       (V)->has_dd_p
#define RDGV_IN(V)         (V)->in_edges
#define RDGV_OUT(V)        (V)->out_edges
#define RDGV_PARTITIONS(V) (V)->partition_numbers
#define RDGV_SCCS(V)       (V)->scc_numbers

#define XNEW(T)        ((T *) xmalloc (sizeof (T)))
#define XCNEWVEC(T, N) ((T *) xcalloc ((N), sizeof (T)))

//////////// RDGE
#define RDGE_COLOR(E)       (E)->color
#define RDGE_SOURCE(E)      (E)->source
#define RDGE_SINK(E)        (E)->sink
#define RDGE_LEVEL(E)       (E)->level
#define RDGE_SCALAR_P(E)    (E)->scalar_p


/////////////// PRDG
#define PRDG_RDG(G)       (G)->rdg
#define PRDG_NBV(G)       (prdg_vertex_p,(G)->vertices).size()
#define PRDG_V(G)         (G)->vertices
#define PRDG_NBE(G)       (prdg_edge_p,(G)->edges).size()
#define PRDG_E(G)         (G)->edges


////////////// PRDGV
#define PRDGV_N(V)           (V)->num
#define PRDGV_COLOR(V)       (V)->color
#define PRDGV_D(V)           (V)->d
#define PRDGV_F(V)           (V)->f
#define PRDGV_SCC(V)         (V)->scc
#define PRDGV_PRED(V)        (V)->pred
#define PRDGV_PV(V)          (V)->pvertices


////////////// PRDGE
#define PRDGE_SOURCE(V)    (V)->source
#define PRDGE_SINK(V)      (V)->sink
#define PRDGE_RDG_EDGE(V)  (V)->rdg_edge


typedef struct rdg *rdg_p;
typedef struct rdg_vertex *rdg_vertex_p;
typedef struct rdg_edge *rdg_edge_p;
typedef struct prdg *prdg_p;
typedef struct prdg_vertex *prdg_vertex_p;
typedef struct prdg_edge *prdg_edge_p;

typedef struct data_dependence_relation ddr;
typedef struct data_dependence_relation *ddr_p;

typedef std::map<Loop*, std::vector<std::vector<Instruction*> > > split_scc;

namespace llvm {
/////////////// data structure ////////////////
struct rdg 
{

  // The loop nest represented by this RDG.  
  Loop *loop_nest;
   
  /* The vertices of the graph.  There is one vertex per
     statement of the basic block of the loop.  */
  unsigned int nb_vertices;
  rdg_vertex_p vertices;

  /* The edges of the graph.  There is one edge per data dependence (between
     memory references) and one edge per scalar dependence.  */
  unsigned int nb_edges;  
  rdg_edge_p edges;
  
  /* Vertices that contain a statement containing an ARRAY_REF.  */
  std::vector<rdg_vertex_p> dd_vertices;
  
  /* Data references and array data dependence relations.  */
  std::vector<ddr_p> dependence_relations;

};

struct rdg_vertex 
{
  /* This color is used for graph algorithms.  */
  int color;

  /* The number of the basic block in the loop body.  */
  unsigned int bb_number;
  /* The number of the vertex.  It represents the number of
     the statement in the basic block.  */
  unsigned int number;
    /* The instruction represented by this vertex.  */
 	Instruction *instrs;
/////////////////// 
  /* True when this vertex contains a data reference 
     that is an ARRAY_REF.  */
  bool has_dd_p; 
  
  /* Vertex is the sink of those edges.  */
//  VEC (rdg_edge_p, heap) *in_edges;
  std::vector<rdg_edge_p> in_edges;
  
  /* Vertex is the source of those edges. */
//  VEC (rdg_edge_p, heap) *out_edges;
  std::vector<rdg_edge_p> out_edges;

  /* Partitions the vertex is in.  
     If 'has_dd_p' is true, the vertex can only be in one partition.
     If not, the vertex can be duplicated in several partitions.  */
  std::vector<int> partition_numbers;
  /* Strongly connected components the vertex is in.
     If 'has_dd_p' is true, the vertex can only be in one SCC.
     If not, the vertex can be in several SCCs.  */
  std::vector<int> scc_numbers;
};

struct data_dependence_relation
{
  Instruction *a;
  Instruction *b;
};

struct rdg_edge 
{
  /* Color used for graph algorithms.  */  
  int color;
  
  /* The vertex source of the dependence.  */
  rdg_vertex_p source;
  
  /* The vertex sink of the dependence.  */
  rdg_vertex_p sink;
  
  /* Level of the dependence: the depth of the loop that
    carries the dependence.  */
  int level;
  
  /* true if the dependence is between two scalars.  Usually,
    it is known of a dependence between two memory elements
    of dimension 0.  */
  bool scalar_p;  
};

struct prdg 
{
  /* The RDG used for partitionning.  */
  rdg_p rdg;
  
  /* The vertices of the graph.  */
  std::vector<prdg_vertex_p> vertices;
  
  /* The edges of the graph.  */
  std::vector<prdg_edge_p> edges;
};

struct prdg_vertex 
{
  /* The partition number.  */
  int num;
  
  /* Used for graph algorithms.  */
  int color; 
  
  /* Discovery time.  Used by DFS.  */
  int d;
  
  /* Finishing time.  Used by DFS and SCC.  */
  int f;
  
  /* SCC number.  */
  int scc; 
  
  /* Predecessor after DFS computation.  */
  prdg_vertex_p pred; 
   
  /* Vertices of the RDG that are in this partition.  */
//  VEC (rdg_vertex_p, heap) *pvertices;
  std::vector<rdg_vertex_p> pvertices;
};

struct prdg_edge 
{
  /* Vertex source of the dependence.  */
  prdg_vertex_p source;
  
  /* Vertex sink of the dependence.  */
  prdg_vertex_p sink;
  
  /* Original edge of the RDG.  */
  rdg_edge_p rdg_edge;
};

////////////END data structure/////////////////

  class AliasAnalysis;
  class DependenceAnalysis;
  class Value;
  class raw_ostream;
  class scc;
  
  class scc : public FunctionPass {
  private:
    AliasAnalysis *AA;
    Function *F;
    DependenceAnalysis *DA;
    LoopInfo *LI;

/////////////////////functions to generate scc///////////////////
std::vector<ddr> compute_data_dependences_for_loop (Loop *loop_nest, DG *depmap);

void dfs_rdgp_visit (prdg_p g, prdg_vertex_p v, unsigned int *t, unsigned int scc);

int dfs_rdgp (prdg_p g);

bool rdgp_vertex_less_than_p (const prdg_vertex_p a, const prdg_vertex_p b);

unsigned int scc_rdgp_1 (prdg_p g, int max_f);

void transpose_rdgp (prdg_p g);

unsigned int lower_bound(std::vector<prdg_vertex_p> sorted_vertices,prdg_vertex_p v);

unsigned int scc_rdgp (prdg_p g);

bool vertex_in_partition_p (rdg_vertex_p v, int p);

bool vertex_in_scc_p (rdg_vertex_p v, int s);

prdg_vertex_p new_prdg_vertex (unsigned int p);

void free_prdg_vertex (prdg_vertex_p v);

prdg_edge_p new_prdg_edge (rdg_edge_p re, prdg_vertex_p sink, prdg_vertex_p source);

void free_prdg_edge (prdg_edge_p e);

prdg_p new_prdg (rdg_p rdg);

void free_prdg (prdg_p g);

prdg_p build_scc_graph (prdg_p g);

bool can_recompute_vertex_p (rdg_vertex_p v);

void one_prdg (rdg_p rdg, rdg_vertex_p v, int p);

bool correct_partitions_p (rdg_p rdg, int p);

unsigned int mark_partitions (rdg_p rdg);

prdg_p build_prdg (rdg_p rdg);

rdg_vertex_p find_vertex_with_instrs (rdg_p rdg, Instruction * instrs);

bool contains_dr_p (Instruction instrs, ddr_p dp);

int number_of_vertices (rdg_p rdg, DG *depmap);

int number_of_edges (rdg_p rdg, DG *depmap);

void create_vertices (rdg_p rdg);

void create_edges (rdg_p rdg);

rdg_p build_rdg (Loop *loop_nest);

std::vector<prdg_vertex_p> topological_sort (prdg_p g);

void do_distribution (Loop *loop_nest, DG *depmap);

void open_loop_dump (Loop *loop_nest);

void close_loop_dump (Loop *loop_nest);

void dump_prdg (FILE *outf, prdg_p rdgp);

void dump_rdg (FILE *outf, rdg_p rdg);

split_scc out_scc(std::vector<prdg_vertex_p>);

//////////////////END functions to generate scc//////////////////

//////////////////scc//////////////////
split_scc outscc;
///////////////////////////////////////

  public:
    static char ID; 
    scc() : FunctionPass(ID) {};
    bool runOnFunction(Function &F);
    void getAnalysisUsage(AnalysisUsage &) const;
    LoopInfo *LI;
    DG *depmap;
  }; // class scc

} // namespace llvm

#endif
 
