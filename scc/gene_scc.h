#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <ctime>
#include <complex>
#include <iostream>
#include <string>
#include <fstream>
#include <stdexcept>
#include <vector>
/* Initial value for VEC data structures used in RDG.  */
# define RDG_VS 10

/* Values used in DFS algorithm.  */
# define VERTEX_WHITE 0
# define VERTEX_GRAY 1
# define VERTEX_BLACK 2

typedef struct rdg *rdg_p;
typedef struct rdg_vertex *rdg_vertex_p;
typedef struct rdg_edge *rdg_edge_p;
typedef struct prdg *prdg_p;
typedef struct prdg_vertex *prdg_vertex_p;
typedef struct prdg_edge *prdg_edge_p;

/////////using <vector> instead/////////////

/* A RDG (Reduced Dependence Graph) represents all data dependence
   constraints between the statements of a loop nest. */

//////////// data structure that we need to construct/////////


struct rdg 
{
/////////////////////NOTICE: COMMENT OUT BUT SHOULD BE NEEDED IN THE FUTURE

  // The loop nest represented by this RDG.  
  Loop *loop_nest;
/*   
  // The SSA_NAME used for loop index.  
  tree loop_index;
  
  // The MODIFY_EXPR used to update the loop index.  
  tree loop_index_update;
 
  // The COND_EXPR that is the exit condition of the loop.  
  tree loop_exit_condition;
  
  // The PHI_NODE of the loop index. 
  tree loop_index_phi_node;
*/
  
  /* The vertices of the graph.  There is one vertex per
     statement of the basic block of the loop.  */
  unsigned int nb_vertices;
  rdg_vertex_p vertices;

  /* The edges of the graph.  There is one edge per data dependence (between
     memory references) and one edge per scalar dependence.  */
  unsigned int nb_edges;  
  rdg_edge_p edges;
  
  /* Vertices that contain a statement containing an ARRAY_REF.  */
//  VEC (rdg_vertex_p, heap) *dd_vertices;
  std::vector<rdg_vertex_p> dd_vertices;
  
  /* Data references and array data dependence relations.  */
//////////////// NOTICE///////////////
//  VEC (ddr_p, heap) *dependence_relations;
  std::vector<ddr_p> dependence_relations;

//  std::vector<ddr> dependence_relations;
/////////////NOTICE///////////
//  VEC (data_reference_p, heap) *datarefs;
//  std::vector<data_reference_p> datarefs;
};

#define RDG_LOOP(G)	  (G)->loop_nest
//#define RDG_IDX(G)        (G)->loop_index
//#define RDG_IDX_UPDATE(G) (G)->loop_index_update
//#define RDG_EXIT_COND(G)  (G)->loop_exit_condition
//#define RDG_IDX_PHI(G)    (G)->loop_index_phi_node
#define RDG_NBV(G)        (G)->nb_vertices
#define RDG_NBE(G)        (G)->nb_edges
#define RDG_V(G)          (G)->vertices
#define RDG_VERTEX(G,i)   &((G)->vertices[i])
#define RDG_E(G)          (G)->edges
#define RDG_EDGE(G,i)     &((G)->edges[i])
#define RDG_DDV(G)        (G)->dd_vertices
//#define RDG_DR(G)         (G)->datarefs
#define RDG_DDR(G)        (G)->dependence_relations


/* A RDG vertex representing a statement.  */
struct rdg_vertex 
{
  /* This color is used for graph algorithms.  */
  int color;

  /* The number of the basic block in the loop body.  */
  unsigned int bb_number;
  /* The number of the vertex.  It represents the number of
     the statement in the basic block.  */
  unsigned int number;
    /* The statement represented by this vertex.  */
////////////////TODO it as instruction
//  tree stmt;
 	struct INSTRUCTION instrs;
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
//  VEC (int, heap) *partition_numbers;
  std::vector<int> partition_numbers;
  /* Strongly connected components the vertex is in.
     If 'has_dd_p' is true, the vertex can only be in one SCC.
     If not, the vertex can be in several SCCs.  */
//  VEC (int, heap) *scc_numbers;
  std::vector<int> scc_numbers;
};


#define RDGV_COLOR(V)      (V)->color
#define RDGV_BB(V)         (V)->bb_number
#define RDGV_N(V)          (V)->number
#define RDGV_INSTRS(V)     (V)->instrs
#define RDGV_DD_P(V)       (V)->has_dd_p
#define RDGV_IN(V)         (V)->in_edges
#define RDGV_OUT(V)        (V)->out_edges
#define RDGV_PARTITIONS(V) (V)->partition_numbers
#define RDGV_SCCS(V)       (V)->scc_numbers

struct data_dependence_relation
{
  Instruction *a;
  Instruction *b;
}

typedef struct data_dependence_relation ddr;
typedef struct data_dependence_relation *ddr_p;

std::vector<ddr> compute_data_dependences_for_loop (Loop *loop_nest);

/* Data dependence type.  */
/////////////?NOTICE:DO WE DIFFER DEP TYPE? OR we only build edge if dep existing
/*
enum rdg_dep_type 
{
  // Read After Write (RAW) (source is W, sink is R).  
  flow_dd = 'f',
  
//   Write After Read (WAR) (source is R, sink is W).  
  anti_dd = 'a',
  
//   Write After Write (WAW) (source is W, sink is W).  
  output_dd = 'o', 
  
//   Read After Read (RAR) (source is R, sink is R). 
     input_dd = 'i' 
};
*/

/* An edge of the RDG with dependence information.  */
struct rdg_edge 
{
  /* Color used for graph algorithms.  */  
  int color;
  
  /* The vertex source of the dependence.  */
  rdg_vertex_p source;
  
  /* The vertex sink of the dependence.  */
  rdg_vertex_p sink;

/////////NOTICE: ? not quiet understand these reference things 
//  /* The reference source of the dependence.  */
//  tree source_ref;
  
//  /* The reference sink of the dependence.  */
//  tree sink_ref;
  
  /* Type of the dependence.  */
//  enum rdg_dep_type type;
  
  /* Level of the dependence: the depth of the loop that
    carries the dependence.  */
  int level;
  
  /* true if the dependence is between two scalars.  Usually,
    it is known of a dependence between two memory elements
    of dimension 0.  */
  bool scalar_p;  
};



#define RDGE_COLOR(E)       (E)->color
#define RDGE_SOURCE(E)      (E)->source
#define RDGE_SINK(E)        (E)->sink
#define RDGE_SOURCE_REF(E)  (E)->source_ref
#define RDGE_SINK_REF(E)    (E)->sink_ref
//#define RDGE_TYPE(E)        (E)->type
#define RDGE_LEVEL(E)       (E)->level
#define RDGE_SCALAR_P(E)    (E)->scalar_p


/* This graph represents a partition: each vertex is a group of
   existing RDG vertices, each edge is a dependence between two
   partitions.  */
struct prdg 
{
  /* The RDG used for partitionning.  */
  rdg_p rdg;
  
  /* The vertices of the graph.  */
//  VEC (prdg_vertex_p, heap) *vertices;
  std::vector<prdg_vertex_p> vertices;
  
  /* The edges of the graph.  */
//  VEC (prdg_edge_p, heap) *edges;
  std::vector<prdg_edge_p> edges;
};


#define PRDG_RDG(G)       (G)->rdg
#define PRDG_NBV(G)       (prdg_vertex_p,(G)->vertices).size()
#define PRDG_V(G)         (G)->vertices
//#define PRDG_VERTEX(G,i)  ((prdg_vertex_p,(G)->vertices).at(i))
#define PRDG_NBE(G)       (prdg_edge_p,(G)->edges).size()
#define PRDG_E(G)         (G)->edges
//#define PRDG_EDGE(G,i)    ((prdg_edge_p,(G)->edges).at(i))


/* A vertex representing a group of RDG vertices.  */
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

#define PRDGV_N(V)           (V)->num
#define PRDGV_COLOR(V)       (V)->color
#define PRDGV_D(V)           (V)->d
#define PRDGV_F(V)           (V)->f
#define PRDGV_SCC(V)         (V)->scc
#define PRDGV_PRED(V)        (V)->pred
//#define PRDGV_NPV(V)         VEC_length (rdg_vertex_p,(V)->pvertices)
#define PRDGV_PV(V)          (V)->pvertices
#define PRDGV_PVERTEX(V,i)   VEC_index (rdg_vertex_p,(V)->pvertices,i)


/* Dependence egde of the partition graph.  */
struct prdg_edge 
{
  /* Vertex source of the dependence.  */
  prdg_vertex_p source;
  
  /* Vertex sink of the dependence.  */
  prdg_vertex_p sink;
  
  /* Original edge of the RDG.  */
  rdg_edge_p rdg_edge;
};


#define PRDGE_SOURCE(V)    (V)->source
#define PRDGE_SINK(V)      (V)->sink
#define PRDGE_RDG_EDGE(V)  (V)->rdg_edge

////////////////////////////NOTICE///////////////////////////////////
/* Array to check if a loop has already been distributed.  */
//bool *treated_loops;


/* Loop location.  */
//static LOC dist_loop_location;

/* Function prototype from tree-vectorizer.  */
//LOC find_loop_location (struct loop*);


/* Helper function for Depth First Search.  */

static void
dfs_rdgp_visit (prdg_p g, prdg_vertex_p v, unsigned int *t, unsigned int scc);


/* Depth First Search.  This is an adaptation of the depth first search
   described in Cormen et al., "Introduction to Algorithms", MIT Press.
   Returns the max of "finishing times" for the partition graph G.  */

static int
dfs_rdgp (prdg_p g);


/* Comparison function to compare "finishing times" of
   two vertices.  */

static bool
rdgp_vertex_less_than_p (const prdg_vertex_p a,
                         const prdg_vertex_p b);

/* Helper function for the computation of strongly connected components.  */

static unsigned int
scc_rdgp_1 (prdg_p g, int max_f);

/* Change the directions of all edges.  */

static void
transpose_rdgp (prdg_p g);


/* Computes the strongly connected components of G.  */
//////////////////////////////////////
unsigned int lower_bound(std::vector<prdg_vertex_p> sorted_vertices,prdg_vertex_p v);
///////////////////
static unsigned int
scc_rdgp (prdg_p g);
//////////////////////////////////


static unsigned int
scc_rdgp (prdg_p g);



static bool
vertex_in_partition_p (rdg_vertex_p v, int p);


static bool
vertex_in_scc_p (rdg_vertex_p v, int s);



static prdg_vertex_p
new_prdg_vertex (unsigned int p);


static void
free_prdg_vertex (prdg_vertex_p v);


static prdg_edge_p
new_prdg_edge (rdg_edge_p re, 
	       prdg_vertex_p sink,
               prdg_vertex_p source);

static void
free_prdg_edge (prdg_edge_p e);


static prdg_p
new_prdg (rdg_p rdg);

static void
free_prdg (prdg_p g);

//////////NOTICE LOOP STRUCT//////

static prdg_p
build_scc_graph (prdg_p g);



static bool
can_recompute_vertex_p (rdg_vertex_p v);


static void
one_prdg (rdg_p rdg, rdg_vertex_p v, int p);


static bool
correct_partitions_p (rdg_p rdg, int p);

static unsigned int
mark_partitions (rdg_p rdg);


static prdg_p
build_prdg (rdg_p rdg);


static rdg_vertex_p
find_vertex_with_instrs (rdg_p rdg, Instruction instrs);


static bool
contains_dr_p (Instruction instrs, ddr_p dp);

static int
number_of_vertices (rdg_p rdg);


static void
create_vertices (rdg_p rdg);

//static int
//number_of_data_dependences (rdg_p rdg);

