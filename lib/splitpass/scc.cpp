/*
This is a Hello pass that demonstrates how to read the analysis results from
the DG pass. There are a few things:
1. #include "DG.h"
2. declare DG *depmap;
3. set depmap = &getAnalysis<DG>();
4. add AU.addRequired<DG>();
5. add hello pass after the dg pass when running the llvm optimization, see dg_test/test.sh.
   opt -load ../Release+Asserts/lib/splitpass.so -basicaa -da -dg -hello < INPUTFILE > /dev/null
*/

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Analysis/LoopInfo.h"
#include "scc.h"

using namespace llvm;
/////////////////////NOTICE these two have to appear in scc.h///////////
//	LoopInfo *LI;
//	DG *depmap;
/////////////////END NOTICE/////////////////////////////////////////////

char scc::ID = 0;
static RegisterPass<scc> X("scc", "scc Pass", false, false);
////////////public implementation //////////////////////
void scc::outputSCC(Loop *L) {
	
      std::vector<Loop*> subLoops = L->getSubLoops();
	  if (subLoops.empty()) { // analysis only applies to innermost loops, so check for that
      errs() << "scc: printing the data from DG pass for ";
      errs() <<*L << '\n';
      if (depmap->dgOfLoops.count(L) == 0) {
        errs() << "scc: No analysis found\n";
      } else {
        std::map<Instruction*, std::set<Instruction*> > dg_temp = depmap->dgOfLoops[L];
        if (dg_temp.empty()) {
          errs() << "Hello: Dependence graph is empty\n";
        } else {
	////////////NOTICE//////////
	/////CALL do_distribution HERE////////////
/*
          std::map<Instruction*, std::set<Instruction*> >::iterator mapit2;
          for (mapit2 = dg_temp.begin(); mapit2 != dg_temp.end(); ++mapit2) {
            Instruction *inst = mapit2->first;
            errs() << "\t\tHello: Instruction " << *inst << "\n";
            std::set<Instruction*> set_temp = mapit2->second;
            std::set<Instruction*>::iterator setit;
            for (setit = set_temp.begin(); setit != set_temp.end(); ++setit) {
              errs() << "\t\t\t\t Hello: -------->" << *(*setit) << "\n";
            }
          }
          errs() << "Hello: There are " << depmap->numOfNodes[L] << " nodes in the loop\n";
          errs() << "Hello: There are " << depmap->numOfDeps[L] << " data dependencies in the loop\n";
*/
        }
      }
    } else {
	  for (std::vector<Loop*>::iterator it = subLoops.begin() ; it != subLoops.end(); ++it) {
        outputSCC(*it);
      }
	}

	}
bool scc:: runOnFunction(Function &F) {
	  LI = &getAnalysis<LoopInfo>();
	  depmap = &getAnalysis<DG>();
	  Loop *curLoop;
	  for (LoopInfo::iterator i = LI->begin(), e = LI->end(); i != e; ++i) {
	    curLoop = *i;
		if (curLoop->getParentLoop() != 0) continue; //skip non-toplevel loop in the iteration
		outputSCC(curLoop);
      }
      return false;
    }

void scc::getAnalysisUsage(AnalysisUsage &AU) const {
      AU.setPreservesAll();
      AU.addRequired<LoopInfo>();
	  AU.addRequired<DG>();
    }
////////////END public implementation//////////////////
//////////private implementation/////////////////////
std::vector<ddr> scc::compute_data_dependences_for_loop (Loop *loop_nest, DG *depmap)
{
  std::vector<ddr> ddr_0; 
  std::map<Instruction*, std::set<Instruction*> > dg_temp = depmap->dgOfLoops[loop_nest];
//  for (mapit1 = dgOfLoops.begin(); mapit1 != dgOfLoops.end(); ++mapit1) {
    int count = 0;
//    std::map<Instruction*, std::set<Instruction*> > dg_temp = mapit1->seconnd;
    std::map<Instruction*, std::set<Instruction*> >::iterator mapit2;
    for (mapit2 = dg_temp.begin; mapit2 != dg_temp.end(); ++mapit2) {
      Instruction *inst = mapit2->first;
      std::set<Instruction*> set_temp = mapit2->second;
      std::set<Instruction*>::iterator setit;
      for (setit = set_temp.begin(); setit != set_temp.end(); ++setit) {
        ddr s;
        s.a = inst;
        s.b = setit;
        ddr_0.push_back(s); 
      }
    }
//  }
  return ddr_0;

}

void scc::dfs_rdgp_visit (prdg_p g, prdg_vertex_p v, unsigned int *t, unsigned int scc)
{
  unsigned int i;
  prdg_edge_p e;
  rdg_vertex_p rdg_v;
  
  PRDGV_COLOR (v) = VERTEX_GRAY;
  (*t)++;
  PRDGV_D (v) = *t;
  PRDGV_SCC (v) = scc;

  /* If scc!=0, add this SCC to each vertex of the partition. */
  if (scc)
    {
	for(std::vector<rdg_vertex_p>::iterator it=(PRDGV_PV (v)).begin();it!=(PRDGV_PV (v)).end();++it)	
	{
		rdg_v=*it;
		(RDGV_SCCS(rdg_v)).push_back(scc);
	}
    }

  for(std::vector<prdg_edge_p>::iterator it=(PRDG_E (g)).begin();it!=(PRDG_E (g)).end();++it)
  {
	if (PRDGE_SOURCE (*it) == v)
      {
	prdg_vertex_p u = PRDGE_SINK (*it);
      
	if (PRDGV_COLOR (u) == VERTEX_WHITE)
	  {
	    PRDGV_PRED (u) = v;
	    dfs_rdgp_visit (g, u, t, scc);
	  }
      }
  }
  
  PRDGV_COLOR (v) = VERTEX_BLACK;
  (*t)++;
  PRDGV_F (v) = *t;
}


int scc::dfs_rdgp (prdg_p g)
{
  unsigned int i;
  /* t represents the max of finishing times.  */
  unsigned int t = 0;
  prdg_vertex_p v;

  for(std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin();it!=(PRDG_V (g)).end();++it)
    {
    	v= *it;
      PRDGV_COLOR (v) = VERTEX_WHITE;
      PRDGV_PRED (v) = NULL;
    }

  for (std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin(); it!=(PRDG_V (g)).end() ;++it)
    if (PRDGV_COLOR (v) == VERTEX_WHITE)
      dfs_rdgp_visit (g, v, &t, 0);
  
  return t;
}

bool scc::rdgp_vertex_less_than_p (const prdg_vertex_p a,
                         const prdg_vertex_p b)
{
  return (PRDGV_F (a) < PRDGV_F (b));
}


unsigned int scc::scc_rdgp_1 (prdg_p g, int max_f)
{
  unsigned int i;
  unsigned int t = 0;
  unsigned int scc = 0;
  prdg_vertex_p v;
  std::vector<prdg_vertex_p> *psorted_vertices;

  for (std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin();it!=(PRDG_V (g)).end(); ++it)
    {
	v = *it;
      PRDGV_COLOR (v) = VERTEX_WHITE;
      PRDGV_PRED (v) = NULL;
    }
  
  /* Here we build a vector containing the vertices sorted by increasing
     finishing times F (computed by DFS).   This is a contradiction with
     the complexity of the SCC algorithm that is in linear time
     O(V+E).   We could have used an array containing pointers to vertices,
     the index of this array representing F for the corresponding vertex.
     This array has a size equal to 'max_f' with holes.  */
  
     psorted_vertices=new std::vector<prdg_vertex_p>[max_f];
    for(std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin();it!=(PRDG_V (g)).end();++it)
    {
	v=*it;
      unsigned int idx = lower_bound (*psorted_vertices,v);

      (*psorted_vertices).insert((((*psorted_vertices).begin())+idx),v);
    }


  
  while ((*psorted_vertices).size())
    {
	v = (*psorted_vertices).back();      
      if (PRDGV_COLOR (v) == VERTEX_WHITE)
	dfs_rdgp_visit (g, v, &t, ++scc);
	(*psorted_vertices).pop_back();
    }

//  sorted_vertices.get_allocator().deallocate(p,max_f);
  delete[] psorted_vertices;
  return scc;
}


void scc::transpose_rdgp (prdg_p g)
{
  unsigned int i;
  prdg_edge_p e;
  
  for (std::vector<prdg_edge_p>::iterator it=(PRDG_E (g)).begin();it!=(PRDG_E (g)).end(); ++it)
    {
	e=*it;
      prdg_vertex_p tmp = PRDGE_SINK (e);

      PRDGE_SINK (e) = PRDGE_SOURCE (e);
      PRDGE_SOURCE (e) = tmp;
    }
}

unsigned int scc::lower_bound(std::vector<prdg_vertex_p> sorted_vertices, prdg_vertex_p v)
{
 unsigned int idx=0;
 for(std::vector<prdg_vertex_p>::iterator it=(sorted_vertices).begin();it!=(sorted_vertices).end();++it,++idx)
 {if(rdgp_vertex_less_than_p(*it,v)){it=(sorted_vertices).end();}}
 return idx;
}

unsigned int scc::scc_rdgp (prdg_p g)
{
  unsigned int nb_sccs;
  int max_f;
  
  max_f = dfs_rdgp (g);
  transpose_rdgp (g);
  nb_sccs = scc_rdgp_1 (g, max_f);
  transpose_rdgp (g);

  return nb_sccs;
}

bool scc::vertex_in_partition_p (rdg_vertex_p v, int p)
{
  int i;
  int vp;
  
  for (std::vector<int>::iterator it=(RDGV_PARTITIONS (v)).begin();it!=(RDGV_PARTITIONS (v)).end(); ++it)
  { 	vp=*it;
	if (vp == p)
      	return true;
  }
  return false;
}

bool scc::vertex_in_scc_p (rdg_vertex_p v, int s)
{
  int i;
  int vs;
  
  for (std::vector<int>::iterator it = (RDGV_SCCS (v)).begin(); it!=(RDGV_SCCS (v)).end(); ++it)
  {	vs=*it;
	if (vs == s)
      	return true;
  }
  return false;
}


prdg_vertex_p scc::new_prdg_vertex (unsigned int p)
{
  prdg_vertex_p v;
  
  v = new (struct prdg_vertex);
  PRDGV_N (v) = p;
  PRDGV_COLOR (v) = 0;
  PRDGV_D (v) = 0;
  PRDGV_F (v) = 0;
  PRDGV_PRED (v) = NULL;
  PRDGV_SCC (v) = 0;
  return v;
}


void scc::free_prdg_vertex (prdg_vertex_p v)
{
    PRDGV_PV (v).~vector();
    delete[] v;
}

prdg_edge_p scc::new_prdg_edge (rdg_edge_p re, 
	       prdg_vertex_p sink,
               prdg_vertex_p source)
{
  prdg_edge_p e;
  
  e = new (struct prdg_edge);
  PRDGE_RDG_EDGE (e) = re;
  PRDGE_SINK (e) = sink;
  PRDGE_SOURCE (e) = source;
  
  return e;
}

void scc::free_prdg_edge (prdg_edge_p e)
{
  delete e;
}

prdg_p scc::new_prdg (rdg_p rdg)
{
  prdg_p rdgp = new (struct prdg);

  PRDG_RDG (rdgp) = rdg;

  return rdgp;
}


void scc::free_prdg (prdg_p g)
{
  prdg_vertex_p v;
  prdg_edge_p e;
  
  for (std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin();it!=(PRDG_V (g)).end(); ++it)
 {
    v=*it;
    free_prdg_vertex (v);
 } 
  for (std::vector<prdg_edge_p>::iterator it=(PRDG_E (g)).begin(); it!=(PRDG_E (g)).end(); ++it)
 {  e=*it; 
    free_prdg_edge (e);
 }
	(PRDG_V (g)).~vector();
	(PRDG_E (g)).~vector();
   delete g;
}


prdg_p scc::build_scc_graph (prdg_p g)
{
  prdg_p sccg;
  unsigned int nb_sccs;
  unsigned int i, j;
  
  /* Computes the SCC of g.  */
  nb_sccs = scc_rdgp (g);

  /* Builds a new partition graph of the SCC of g.  */
  sccg = new_prdg (PRDG_RDG (g));
  
 /* Create SCC vertices.  */
  for (i = 0; i < nb_sccs; i++)
    {
      unsigned int current_scc = i + 1;
     unsigned int nbv = RDG_NBV (PRDG_RDG (sccg));
      prdg_vertex_p v = new_prdg_vertex (current_scc);
      
      for (j = 0; j < nbv; j++)
	{
	  rdg_vertex_p rdg_v = RDG_VERTEX (PRDG_RDG (sccg), j);
        
	  if (vertex_in_scc_p (rdg_v, current_scc))
		(PRDGV_PV (v)).push_back(rdg_v);
	}
      
      PRDGV_SCC (v) = current_scc;
	(PRDG_V (sccg)).push_back(v);
    }
  
  /* Create SCC edges.  */
  for (i = 0; i < RDG_NBE (PRDG_RDG (g)); i++)
    {
      rdg_edge_p e = RDG_EDGE (PRDG_RDG (g), i);
    
      /* Here we take only into account data dependences.  */
      if (!RDGE_SCALAR_P (e))
	{
	  prdg_edge_p pe;
	  int source_idx = (RDGV_SCCS (RDGE_SOURCE (e))).at(0);
          int sink_idx = (RDGV_SCCS (RDGE_SINK (e))).at(0);
          
	pe = new_prdg_edge (e,((sccg->vertices).at(source_idx - 1)),((sccg->vertices).at(sink_idx - 1)));
 
	(sccg->edges).push_back(pe);
	}	
    }
    
  return sccg;
}

bool scc::can_recompute_vertex_p (rdg_vertex_p v)
{
  rdg_edge_p in_edge;
  unsigned int i;
  
  if (RDGV_DD_P (v))
    return false;
  
    for(std::vector<rdg_edge_p>::iterator it=(RDGV_IN (v)).begin();it!=(RDGV_IN (v)).end();++it)
    {
	in_edge = *it; 
    if (RDGE_SCALAR_P (in_edge))
      if (!can_recompute_vertex_p (RDGE_SOURCE (in_edge)))
        return false;
    }
  return true;
}

void scc::one_prdg (rdg_p rdg, rdg_vertex_p v, int p)
{
  rdg_edge_p o_edge, i_edge;
  unsigned int i;
  
  if (vertex_in_partition_p (v, p))
    return;

  (RDGV_PARTITIONS (v)).push_back(p);

    for(std::vector<rdg_edge_p>::iterator it=(RDGV_IN (v)).begin();it!=(RDGV_IN (v)).end();++it)
    {
      i_edge = *it;
      if (RDGE_SCALAR_P (i_edge))
      one_prdg (rdg, RDGE_SOURCE (i_edge), p);
    }
  if (!can_recompute_vertex_p (v))
      for(std::vector<rdg_edge_p>::iterator it=(RDGV_OUT (v)).begin();it!=(RDGV_OUT (v)).end();++it)
      {
      if (RDGE_SCALAR_P (o_edge))
        one_prdg (rdg, RDGE_SINK (o_edge), p);
      }
}

bool scc::correct_partitions_p (rdg_p rdg, int p)
{
  unsigned int i;
  
  if (!p)
    return false;
  
  /* All vertices must have color != 0.  */
  for (i = 0; i < RDG_NBV (rdg); i++)
    {
        if(RDGV_DD_P (RDG_VERTEX (rdg, i))
	  &&!((RDGV_PARTITIONS(RDG_VERTEX (rdg, i)).size()==1)))
	return false;
    
      if (!(RDGV_PARTITIONS (RDG_VERTEX (rdg, i)).size()))
        return false;
    }

  return true;
}

std::vector <prdg_vertex_p>
topological_sort (prdg_p g)
{
  unsigned int max_f, i;
  prdg_vertex_p *vertices;
  prdg_vertex_p v;
  std::vector<prdg_vertex_p> sorted_vertices;
  
  /* Depth First Search.  */
  max_f = dfs_rdgp (g);
  
  /* Allocate array of vertices.  */
  vertices = XCNEWVEC (prdg_vertex_p, max_f+1);
  
  /* Allocate a vector for sorted vertices.  */ 
//  sorted_vertices = VEC_alloc (prdg_vertex_p, heap, RDG_VS);
  
  /* All vertices are set to NULL.  */
  for (i = 0; i <= max_f; i++)
    vertices[i] = NULL;
  
  /* Iterate on each vertex of the PRDG and put each vertex at
     the right place.  */
//  for (i = 0; VEC_iterate (prdg_vertex_p, PRDG_V (g), i, v); i++)
//    vertices[PRDGV_F (v)] = v;
//  for (i = 0; VEC_iterate (prdg_vertex_p, PRDG_V (g), i, v); i++)
  for (i = 0; i < max_f; i++){
    v = (PRDG_V (g))[i];
    vertices[PRDGV_F (v)] = v;
  }

  /* Push all non-NULL vertices to vector of vertices.  */
  for (i = max_f; i > 0; i--)
    if (vertices[i])
      sorted_vertices.push_back(vertices[i]);
//      VEC_safe_push (prdg_vertex_p, heap, sorted_vertices, vertices[i]);
  
  free (vertices);
  
  return sorted_vertices;
}

unsigned int scc::mark_partitions (rdg_p rdg)
{
  rdg_vertex_p rdg_v;
  unsigned int i;
  int k, p = 0;

  /* Clear all existing partitions.  */
  for (i = 0; i < RDG_NBV (rdg); i++)
    RDGV_PARTITIONS (RDG_VERTEX (rdg,i)).resize(0);
  
  /* If there are no dd_vertices, put all in one single partition.  */
  if ((RDG_DDV (rdg)).size() == 0)
    {
     /* Mark all vertices with p=1.  */
      for (i = 0; i < RDG_NBV (rdg); i++)
	(RDGV_PARTITIONS (RDG_VERTEX (rdg, i))).push_back(1);	
	
      return 1;
    }
    
  /* Mark each vertex with its own color and propagate.  */
  for (std::vector<rdg_vertex_p>::iterator it=(RDG_DDV (rdg)).begin();it!=(RDG_DDV (rdg)).end();++it)
    {
	rdg_v = *it;
     if ( (RDGV_PARTITIONS (rdg_v)).size() == 0)
      one_prdg (rdg, rdg_v, ++p);
    }
  /* Add the vertices that are not in a partition in all partitions.
     Those vertices does not contain any ARRAY_REF (otherwise, they would
     have been added by the previous loop on dd_vertices).  */
  for (i = 0; i < RDG_NBV (rdg); i++)
    if ((RDGV_PARTITIONS (RDG_VERTEX (rdg, i))).size() == 0)
      for (k = 1; k <= p; k++)
	(RDGV_PARTITIONS (RDG_VERTEX (rdg, i))).push_back(k);
    
	if(correct_partitions_p (rdg, p)) std::cout<<"wrong partition"<<std::endl;
  
  return p;
}

prdg_p scc:build_prdg (rdg_p rdg)
{
  unsigned int i, j;
  rdg_vertex_p rdg_v;  
  prdg_p rdgp = new_prdg (rdg);
  unsigned int nbp = mark_partitions (rdg);
  
  /* Create partition vertices.  */
  for (i = 0; i < nbp; i++)
    {
      unsigned int current_partition = i+1;
      prdg_vertex_p v = new_prdg_vertex (current_partition);
      
      for (j = 0; j < rdg->nb_vertices; j++)
	{
	  rdg_v = RDG_VERTEX (rdg, j);
        
	  if (vertex_in_partition_p (rdg_v, current_partition))
	  (PRDGV_PV (v)).push_back(rdg_v);
	}

      (PRDG_V (rdgp)).push_back(v);

    }

  /* Create partition edges.  */
  for (i = 0; i < rdg->nb_edges; i++)
    {
      rdg_edge_p e = RDG_EDGE (rdg, i);
    
      /* Here we take only into account data dependences.  */
      if (!RDGE_SCALAR_P (e))
	{
	  int so_idx = (RDGV_PARTITIONS (RDGE_SOURCE (e))).at(0);
          int si_idx = (RDGV_PARTITIONS (RDGE_SINK (e))).at(0);
	  prdg_edge_p pe = new_prdg_edge (e,  (rdgp->vertices[so_idx-1]),
					  (rdgp->vertices[si_idx-1]));

	  (PRDG_E (rdgp)).push_back(pe);

	}
    }
  
  return rdgp;
}

rdg_vertex_p scc::find_vertex_with_instrs (rdg_p rdg, Instruction instrs)
{
  rdg_vertex_p vertex = NULL;
  unsigned int i;
  
  for (i = 0; i < RDG_NBV (rdg) && vertex == NULL; i++)
    if (RDGV_INSTRS(RDG_VERTEX (rdg,i)) == instrs)
      vertex = RDG_VERTEX (rdg, i);
  
  return vertex;
}

bool scc::contains_dr_p ( Instruction instrs, std::vector<ddr_p> pddr)
{
  ddr_p dd;
  
    for(std::vector<ddr_p>::iterator it=(pddr).begin();it!=(pddr).end();++it)
    {
	dd=*it;
    if (DD_INSTR (dd) == instrs)
      return true;
    }
  return false;

}

int scc::number_of_vertices (rdg_p rdg, DG *depmap)
{
 return depmap->numOfNodes[RDG_LOOP(rdg)];
}

int scc::number_of_edges (rdg_p rdg, DG *depmap)
{  return depmap->numOfDeps[RDG_LOOP(rdg)];}

void scc::create_vertices (rdg_p rdg)
{
  BasicBlock bb;
  unsigned int i;
  unsigned int vertex_index;
  BasicBlock::iterator bsi;
  Loop *loop_nest = RDG_LOOP (rdg);
  
  RDG_NBV (rdg) = number_of_vertices (rdg);
  
  vertex_index = 0;
  std::vector<BasicBlock * > bbs = loop_nest->getBlocks();
//////////////////NOTICE/////////getBody!  
  for (i = 0; i < (loop_nest->getNumBlocks()); i++)
    {
      bb = bbs[i];
    
      for (bsi = bb->begin(); bsi!= bb->end(); ++bsi)
        {
	  Instruction instrs = *bsi;

              rdg_vertex_p v = RDG_VERTEX (rdg, vertex_index);
              RDGV_RDGV_INSTRS (v) = instrs;
              RDGV_N (v) = vertex_index;
              RDGV_BB (v) = i;
              RDGV_COLOR (v) = 0;
              RDGV_DD_P (v) = contains_dr_p (instrs, RDG_DDR (rdg));
              vertex_index++;

        }
    }
}

void scc::create_edges (rdg_p rdg)
{
  unsigned int i;
  unsigned int j;
  unsigned int edge_index;
  unsigned int edges;
  ddr_p ddrp;


  /* Allocate an array for scalar edges and data edges.  */

  edges = number_of_edges (rdg);
  if (edges == 0) {
      RDG_NBE (rdg) = 0;
      RDG_E (rdg) = NULL;
      return;
  }

  RDG_NBE (rdg) = edges;
  RDG_E (rdg) = XCNEWVEC (struct rdg_edge, RDG_NBE (rdg));

  /* Create data edges.  */
  edge_index = 0;
 
  for (int iter = 0; iter < vector_size; ++iter){
   update_edge_with_ddv(ddrp, ddrp[iter], rdg, edge_index++); 
  }

}

/* Creates an edge with a data dependence vector.  */

update_edge_with_ddv (ddr_p ddrp, ddr ddr0, rdg_p rdg,
                      unsigned int index_of_edge)
{
  Instruction *a;
  Instruction *b;
  rdg_edge_p edge = RDG_EDGE (rdg, index_of_edge);
  rdg_vertex_p va;
  rdg_vertex_p vb;
  

  /* Invert data references according to the direction of the 
     dependence.  */
/*
  if (DDR_REVERSE_P (ddr))
    {
      dra = DDR_B (ddr);
      drb = DDR_A (ddr);
    }
  else
    {
      dra = DDR_A (ddr);
      drb = DDR_B (ddr);
    }
*/

  /* Locate the vertices containing the statements that contain
     the data references.  */
  va = find_vertex_with_stmt (rdg, a);
  vb = find_vertex_with_stmt (rdg, b);
//  gcc_assert (va && vb);

  /* Update source and sink of the dependence.  */
  RDGE_SOURCE (edge) = va;
  RDGE_SINK (edge) = vb;
//  RDGE_SOURCE_REF (edge) = DR_REF (dra);
//  RDGE_SINK_REF (edge) = DR_REF (drb);
  
  /* Determines the type of the data dependence.  */
/*
  if (DR_IS_READ (dra) && DR_IS_READ (drb))
    RDGE_TYPE (edge) = input_dd;
  else if (!DR_IS_READ (dra) && !DR_IS_READ (drb))
    RDGE_TYPE (edge) = output_dd;
  else if (!DR_IS_READ (dra) && DR_IS_READ (drb))
    RDGE_TYPE (edge) = flow_dd;
  else if (DR_IS_READ (dra) && !DR_IS_READ (drb))
    RDGE_TYPE (edge) = anti_dd;

  RDGE_LEVEL (edge) = get_dependence_level (DDR_DIST_VECT (ddr, 
                                                           index_of_vector), 
					    DDR_NB_LOOPS (ddr));
*/
  RDGE_COLOR (edge) = 0;
  RDGE_SCALAR_P (edge) = false;
  
//  VEC_safe_push (rdg_edge_p, heap, RDGV_OUT (va), edge);
//  VEC_safe_push (rdg_edge_p, heap, RDGV_IN (vb), edge);
  RDGV_OUT(va).push_back(edge);
  RDGV_IN(vb).push_back(edge);
}

rdg_p scc::build_rdg (Loop *loop_nest)
{
  rdg_p rdg;
  std::vector<ddr> *dependence_relations;
  std::vector<rdg_vertex_p> dd_vertices;
  unsigned int i;
  rdg_vertex_p vertex;
  
  /* Compute array data dependence relations */



  /* OK, now we know that we can build our Reduced Dependence Graph
     where each vertex is a statement and where each edge is a data
     dependence between two references in statements. */
  rdg = XNEW (struct rdg);
  RDG_LOOP (rdg) = loop_nest;


  RDG_DDR (rdg) = compute_data_dependences_for_loop (loop_nest) 
  
  create_vertices (rdg);
  create_edges (rdg);



  for (i = 0; i < RDG_NBV (rdg); i++)
    {
      vertex = RDG_VERTEX (rdg, i);

      if (RDGV_DD_P (vertex))
//	VEC_safe_push (rdg_vertex_p, heap, RDG_DDV (rdg), vertex);
        RDG_DDV (rdg).push_back(vertex);
    }

  return rdg;
}

void scc::do_distribution (Loop *loop_nest)
{
  rdg_p rdg; /* Reduced dependence graph.  */
  prdg_p rdgp; /* Graph of RDG partitions.  */
  prdg_p sccg; /* Graph of Strongly Connected Components.  */
  std::vector <prdg_vertex_p> dloops; /* Distributed loops.  */  
  bool dump_file = 0;

  rdg = build_rdg (loop_nest);

  if (dump_file)
    {
      fprintf (dump_file, "<rdg>\n");
      dump_rdg (dump_file, rdg);
      fprintf (dump_file, "</rdg>\n");
    }

  rdgp = build_prdg (rdg);      
  
  if (dump_file)
    {
      fprintf (dump_file, "<prdg>\n");
      dump_prdg (dump_file, rdgp);
      fprintf (dump_file, "</prdg>\n");
    }
  
  sccg = build_scc_graph (rdgp);

  if (dump_file)
    {
      fprintf (dump_file, "<sccp>\n");
      dump_prdg (dump_file, sccg);
      fprintf (dump_file, "</sccp>\n");
    }
  
  dloops = topological_sort (sccg);

  outscc = split_scc(dloops, loop_nest);

  if (dump_file)
    {
      prdg_vertex_p v;
      int i;
      
      fprintf (dump_file, "<topological_sort>\n");
      
      for (std::vector<prdg_vertex_p>::iterator it=(dloops).begin();it!=(dloops).end();++it )
      {
       v = *it; 
       fprintf (dump_file, "  <dloop n=\"%d\">P%d</dloop>\n",i, PRDGV_N (v));
      }
      
      fprintf (dump_file, "</topological_sort>\n");
    }

  free_rdg (rdg);
  free_prdg (rdgp);
  free_prdg (sccg);
}

void scc::dump_prdg (FILE *outf, prdg_p rdgp)
{
  unsigned int p, i;
  prdg_vertex_p pv;
  prdg_edge_p pe;
  rdg_vertex_p v;

  fprintf (outf, "<graphviz><![CDATA[\n");
  fprintf (outf, "digraph ");
  fprintf (outf, " {\n");
    
  /* Print out vertices. Each vertex represents a partition, then it
    can contain several statements.  */
  for (std::vector<prdg_vertex_p>::iterator it=(PRDG_V (rdgp)).begin();it!=(PRDG_V (rdgp)).end();++it)
    {
	pv = *it;
      fprintf (outf, " P%d [ shape=rect,label = \" P%d(%d): ", 
	       PRDGV_N (pv), PRDGV_N (pv), PRDGV_SCC (pv));
    
      for (std::vector<rdg_vertex_p>::iterator it=(PRDGV_PV (pv)).begin();it!=(PRDGV_PV (pv)).end();++it )
      {
	v = *it;
	fprintf (outf, "S%d;", RDGV_N (v));
      } 
      fprintf (outf, "\"];\n");
    
      fprintf (outf, " v%d [ label = \" P%d(%d)",  PRDGV_N (pv), 
               PRDGV_N (pv), PRDGV_SCC (pv));   
      
      fprintf (outf, "\"];\n");
      
      fprintf (outf, "{rank=same; P%d; v%d; }\n",  PRDGV_N (pv), PRDGV_N (pv)); 
    } 
  
  for (std::vector<prdg_edge_p>::iterator it=(PRDG_E (rdgp)).begin();it!=(PRDG_E (rdgp)).end();++it)
  {
	pe = *it;
    fprintf (outf, "v%d -> v%d [style=dotted];\n",
	     PRDGV_N (PRDGE_SOURCE (pe)),
	     PRDGV_N (PRDGE_SINK (pe)) 
	     );


  }
  fprintf (outf, "}\n");
  fprintf (outf, "]]></graphviz>\n");
}

void
dump_rdg (FILE *outf, rdg_p rdg)
{
  unsigned int i;
  rdg_vertex_p vertex;
  
  fprintf (outf, "<graphviz><![CDATA[\n");
  fprintf (outf, "digraph ");
  fprintf (outf, " {\n");
  
  for (i = 0; i < RDG_NBV (rdg); i++)
    { 
      rdg_vertex_p v = RDG_VERTEX (rdg, i);
    
      fprintf (outf, " v%d [ label = \"", RDGV_N (v));
      fprintf (outf, "S%d : ", RDGV_N (v));
      fprintf (outf, "instrs%s",RDGV_INSTRS (v));
      fprintf (outf, "\"");
      
      if (RDGV_DD_P (v))
	fprintf (outf, " shape=rect style=filled color=\".7 .3 1.0\"]");
      else
	fprintf (outf, " shape=rect]");	
      
      fprintf (outf, ";\n");
    }
  
  for (i = 0; i < RDG_NBE (rdg); i++)
    {
      struct rdg_edge *e = RDG_EDGE (rdg, i);
      struct rdg_vertex *sink = RDGE_SINK (e);
      struct rdg_vertex *source = RDGE_SOURCE (e);
      
      fprintf (outf, " v%d -> v%d", RDGV_N (source), RDGV_N (sink));      

      
     fprintf(outf, "]\n");
    }
  
  fprintf (outf, "}\n");
  fprintf (outf, "]]></graphviz>\n");
  fprintf (outf, "<dd_vertices>\n");
  
  for (std::vector<rdg_vertex_p>::iterator it=(RDG_DDV (rdg)).begin();it!=(RDG_DDV (rdg)).end();++it)
    {
	 vertex = *it;
      fprintf (outf, "<dd_vertex s=\"s%d\">", RDGV_N (vertex));
      print_generic_expr (outf, RDGV_STMT (vertex), 0);
      fprintf (outf, "</dd_vertex>\n");
    }

  fprintf (outf, "</dd_vertices>\n");
}

void out_scc(std::vector<prdg_vertex_p> scc, Loop *loop_nest)
{
  std::vector<std::vector<Instruction*> > sccs;
  std::vector<Instruction*> single_scc;
  std::vector<rdg_vertex_p> pvertices;

  for (i = 0; i < scc.size(); i++){
    pvertices = scc[i].pvertices;
    for (j = 0; j < pvertices.size(); j++)
      single_scc.push_back(pvertices[j]->instrs);
    sccs.push_back(single_scc);
  }
  outscc[loop_nest] = sccs;
}

