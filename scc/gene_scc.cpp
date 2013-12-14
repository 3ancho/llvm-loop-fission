#include "gene_scc.h"
/* Helper function for Depth First Search.  */

static void
dfs_rdgp_visit (prdg_p g, prdg_vertex_p v, unsigned int *t, unsigned int scc)
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
/*
    for (i = 0; VEC_iterate (rdg_vertex_p, PRDGV_PV (v), i, rdg_v); i++)
      VEC_safe_push (int, heap, RDGV_SCCS (rdg_v), scc);*/
/*  
  for (i = 0; VEC_iterate (prdg_edge_p, PRDG_E (g), i, e); i++)
    if (PRDGE_SOURCE (e) == v)
      {
	prdg_vertex_p u = PRDGE_SINK (e);
      
	if (PRDGV_COLOR (u) == VERTEX_WHITE)
	  {
	    PRDGV_PRED (u) = v;
	    dfs_rdgp_1 (g, u, t, scc);
	  }
      }
*/
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


/* Depth First Search.  This is an adaptation of the depth first search
   described in Cormen et al., "Introduction to Algorithms", MIT Press.
   Returns the max of "finishing times" for the partition graph G.  */

static int
dfs_rdgp (prdg_p g)
{
  unsigned int i;
  /* t represents the max of finishing times.  */
  unsigned int t = 0;
  prdg_vertex_p v;

//  for (i = 0; VEC_iterate (prdg_vertex_p, PRDG_V (g), i, v); i++)
  for(std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin();it!=(PRDG_V (g)).end();++it)
    {
    	v= *it;
      PRDGV_COLOR (v) = VERTEX_WHITE;
      PRDGV_PRED (v) = NULL;
    }

//  for (i = 0; VEC_iterate (prdg_vertex_p, PRDG_V (g), i, v); i++)
  for (std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin(); it!=(PRDG_V (g)).end() ;++it)
    if (PRDGV_COLOR (v) == VERTEX_WHITE)
      dfs_rdgp_visit (g, v, &t, 0);
  
  return t;
}


/* Comparison function to compare "finishing times" of
   two vertices.  */

static bool
rdgp_vertex_less_than_p (const prdg_vertex_p a,
                         const prdg_vertex_p b)
{
  return (PRDGV_F (a) < PRDGV_F (b));
}


/* Helper function for the computation of strongly connected components.  */
unsigned int lower_bound(std::vector<prdg_vertex_p> sorted_vertices, prdg_vertex_p v)
{
 unsigned int idx=0;
 for(std::vector<prdg_vertex_p>::iterator it=(sorted_vertices).begin();it!=(sorted_vertices).end();++it,++idx)
 {if(rdgp_vertex_less_than_p(*it,v)){it=(sorted_vertices).end();}}
 return idx;
}


static unsigned int
scc_rdgp_1 (prdg_p g, int max_f)
{
  unsigned int i;
  unsigned int t = 0;
  unsigned int scc = 0;
  prdg_vertex_p v;
//  VEC (prdg_vertex_p, heap) *sorted_vertices;
//  std::vector<prdg_vertex_p> sorted_vertices;
  std::vector<prdg_vertex_p> *psorted_vertices;

//  for (i = 0; VEC_iterate (prdg_vertex_p, PRDG_V (g), i, v); i++)
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
  
//  sorted_vertices = VEC_alloc (prdg_vertex_p, heap, max_f);
//   prdg_vertex_p *p;
//   p=sorted_vertices.get_allocator().allocate(max_f);
     psorted_vertices=new std::vector<prdg_vertex_p>[max_f];
 // for (i = 0; VEC_iterate (prdg_vertex_p, PRDG_V (g), i, v); i++)
    for(std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin();it!=(PRDG_V (g)).end();++it)
    {
	v=*it;
      unsigned int idx = lower_bound (*psorted_vertices,v);

      (*psorted_vertices).insert((((*psorted_vertices).begin())+idx),v);
//      VEC_safe_insert (prdg_vertex_p, heap, sorted_vertices, idx, v);
    }

//  gcc_assert (VEC_length (prdg_vertex_p, sorted_vertices));

  
  while ((*psorted_vertices).size())
    {
//      v = VEC_pop (prdg_vertex_p, sorted_vertices);
//      v = sorted_vertices[sorted_vertices.size()-1];
	v = (*psorted_vertices).back();      
      if (PRDGV_COLOR (v) == VERTEX_WHITE)
	dfs_rdgp_visit (g, v, &t, ++scc);
	(*psorted_vertices).pop_back();
    }

//  VEC_free (prdg_vertex_p, heap, sorted_vertices);
//  sorted_vertices.get_allocator().deallocate(p,max_f);
  delete[] psorted_vertices;
  return scc;
}

/* Change the directions of all edges.  */

static void
transpose_rdgp (prdg_p g)
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


/* Computes the strongly connected components of G.  */

static unsigned int
scc_rdgp (prdg_p g)
{
  unsigned int nb_sccs;
  int max_f;
  
  max_f = dfs_rdgp (g);
  transpose_rdgp (g);
  nb_sccs = scc_rdgp_1 (g, max_f);
  transpose_rdgp (g);

  return nb_sccs;
}

static bool
vertex_in_partition_p (rdg_vertex_p v, int p)
{
  int i;
  int vp;
  
//  for (i = 0; VEC_iterate (int, RDGV_PARTITIONS (v), i, vp); i++)
  for (std::vector<int>::iterator it=(RDGV_PARTITIONS (v)).begin();it!=(RDGV_PARTITIONS (v)).end(); ++it)
  { 	vp=*it;
	if (vp == p)
      	return true;
  }
  return false;
}


static bool
vertex_in_scc_p (rdg_vertex_p v, int s)
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

static prdg_vertex_p
new_prdg_vertex (unsigned int p)
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

static void
free_prdg_vertex (prdg_vertex_p v)
{
    PRDGV_PV (v).~vector();
    delete[] v;
}


static prdg_edge_p
new_prdg_edge (rdg_edge_p re, 
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

static void
free_prdg_edge (prdg_edge_p e)
{
  delete e;
}


static prdg_p
new_prdg (rdg_p rdg)
{
  prdg_p rdgp = new (struct prdg);

  PRDG_RDG (rdgp) = rdg;
//  PRDG_V (rdgp) = VEC_alloc (prdg_vertex_p, heap, RDG_VS);
//  PRDG_E (rdgp) = VEC_alloc (prdg_edge_p, heap, RDG_VS);

  return rdgp;
}


static void
free_prdg (prdg_p g)
{
//  unsigned int i;
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
//  VEC_free (prdg_vertex_p, heap, PRDG_V (g));
//  VEC_free (prdg_edge_p, heap, PRDG_E (g));
	(PRDG_V (g)).~vector();
	(PRDG_E (g)).~vector();
   delete g;
}

static prdg_p
build_scc_graph (prdg_p g)
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
//	    VEC_safe_push (rdg_vertex_p, heap, PRDGV_PV (v), rdg_v);
		(PRDGV_PV (v)).push_back(rdg_v);
	}
      
      PRDGV_SCC (v) = current_scc;
//      VEC_safe_push (prdg_vertex_p, heap, PRDG_V (sccg), v);
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
//	  int source_idx = VEC_index (int, RDGV_SCCS (RDGE_SOURCE (e)), 0);
	  int source_idx = (RDGV_SCCS (RDGE_SOURCE (e))).at(0);
//	  int sink_idx = VEC_index (int, RDGV_SCCS (RDGE_SINK (e)), 0);
          int sink_idx = (RDGV_SCCS (RDGE_SINK (e))).at(0);
//	  gcc_assert (source_idx && sink_idx);   
          
	pe = new_prdg_edge (e,((sccg->vertices).at(source_idx - 1)),((sccg->vertices).at(sink_idx - 1)));
 
//  VEC_safe_push (prdg_edge_p, heap, sccg->edges, pe);
	(sccg->edges).push_back(pe);
	}	
    }
    
  return sccg;
}

static bool
can_recompute_vertex_p (rdg_vertex_p v)
{
  rdg_edge_p in_edge;
  unsigned int i;
  
  if (RDGV_DD_P (v))
    return false;
  
//  for (i = 0; VEC_iterate (rdg_edge_p, RDGV_IN (v), i, in_edge); i++)
    for (std::vector<rdg_edge_p>::iterator it=(RDGV_IN (v)).begin();it!=(RDGV_IN (v)).end();++it)
    {
	in_edge = *it; 
    if (RDGE_SCALAR_P (in_edge))
      if (!can_recompute_vertex_p (RDGE_SOURCE (in_edge)))
        return false;
    }
  return true;
}

static void
one_prdg (rdg_p rdg, rdg_vertex_p v, int p)
{
  rdg_edge_p o_edge, i_edge;
  unsigned int i;
  
  if (vertex_in_partition_p (v, p))
    return;

//  VEC_safe_push (int, heap, RDGV_PARTITIONS (v), p);
  (RDGV_PARTITIONS (v)).push_back(p);

//  for (i = 0; VEC_iterate (rdg_edge_p, RDGV_IN (v), i, i_edge); i++)
    for(std::vector<rdg_edge_p>::iterator it=(RDGV_IN (v)).begin();it!=(RDGV_IN (v)).end();++it)
    {
      i_edge = *it;
      if (RDGE_SCALAR_P (i_edge))
      one_prdg (rdg, RDGE_SOURCE (i_edge), p);
    }
  if (!can_recompute_vertex_p (v))
//    for (i = 0; VEC_iterate (rdg_edge_p, RDGV_OUT (v), i, o_edge); i++)
      for(std::vector<rdg_edge_p>::iterator it=(RDGV_OUT (v)).begin();it!=(RDGV_OUT (v)).end();++it)
      {
      if (RDGE_SCALAR_P (o_edge))
        one_prdg (rdg, RDGE_SINK (o_edge), p);
      }
}



static bool
correct_partitions_p (rdg_p rdg, int p)
{
  unsigned int i;
  
  if (!p)
    return false;
  
  /* All vertices must have color != 0.  */
  for (i = 0; i < RDG_NBV (rdg); i++)
    {
//      if (RDGV_DD_P (RDG_VERTEX (rdg, i))
//	  && !VEC_length (int, RDGV_PARTITIONS (RDG_VERTEX (rdg, i))) == 1)
        if(RDGV_DD_P (RDG_VERTEX (rdg, i))
	  &&!((RDGV_PARTITIONS(RDG_VERTEX (rdg, i)).size()==1)))
	return false;
    
      if (!(RDGV_PARTITIONS (RDG_VERTEX (rdg, i)).size()))
        return false;
    }

  return true;
}


static unsigned int
mark_partitions (rdg_p rdg)
{
  rdg_vertex_p rdg_v;
  unsigned int i;
  int k, p = 0;

  /* Clear all existing partitions.  */
  for (i = 0; i < RDG_NBV (rdg); i++)
    RDGV_PARTITIONS (RDG_VERTEX (rdg,i)).resize(0);
    //VEC_truncate (int, RDGV_PARTITIONS (RDG_VERTEX (rdg,i)), 0);
  
  /* If there are no dd_vertices, put all in one single partition.  */
  if ((RDG_DDV (rdg)).size() == 0)
    {
     /* Mark all vertices with p=1.  */
      for (i = 0; i < RDG_NBV (rdg); i++)
	(RDGV_PARTITIONS (RDG_VERTEX (rdg, i))).push_back(1);	
//        VEC_safe_push (int, heap, RDGV_PARTITIONS (RDG_VERTEX (rdg, i)), 1);
	
      return 1;
    }
    
  /* Mark each vertex with its own color and propagate.  */
//  for (i = 0; VEC_iterate (rdg_vertex_p, RDG_DDV (rdg), i, rdg_v); i++)
  for (std::vector<rdg_vertex_p>::iterator it=(RDG_DDV (rdg)).begin();it!=(RDG_DDV (rdg)).end();++it)
    {
	rdg_v = *it;
//    if (VEC_length (int, RDGV_PARTITIONS (rdg_v)) == 0)
     if ( (RDGV_PARTITIONS (rdg_v)).size() == 0)
      one_prdg (rdg, rdg_v, ++p);
    }
  /* Add the vertices that are not in a partition in all partitions.
     Those vertices does not contain any ARRAY_REF (otherwise, they would
     have been added by the previous loop on dd_vertices).  */
  for (i = 0; i < RDG_NBV (rdg); i++)
    if ((RDGV_PARTITIONS (RDG_VERTEX (rdg, i))).size() == 0)
      for (k = 1; k <= p; k++)
        //VEC_safe_push (int, heap, RDGV_PARTITIONS (RDG_VERTEX (rdg, i)), k);
	(RDGV_PARTITIONS (RDG_VERTEX (rdg, i))).push_back(k);
    
//  gcc_assert (correct_partitions_p (rdg, p));
	if(correct_partitions_p (rdg, p)) std::cout<<"wrong partition"<<std::endl;
  
  return p;
}


static prdg_p
build_prdg (rdg_p rdg)
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
//	    VEC_safe_push (rdg_vertex_p, heap, PRDGV_PV (v), rdg_v);
	  (PRDGV_PV (v)).push_back(rdg_v);
	}

//      VEC_safe_push (prdg_vertex_p, heap, PRDG_V (rdgp), v);
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
//	  prdg_edge_p pe = new_prdg_edge (e, PRDG_VERTEX (rdgp, so_idx-1),
//					  PRDG_VERTEX (rdgp, si_idx-1));
	  prdg_edge_p pe = new_prdg_edge (e,  (rdgp->vertices[so_idx-1]),
					  (rdgp->vertices[si_idx-1]));

//	  VEC_safe_push (prdg_edge_p, heap, PRDG_E (rdgp), pe);
	  (PRDG_E (rdgp)).push_back(pe);

	}
    }
  
  return rdgp;
}

//////////////////NEED TO BUILD DUMP FUNCTIONS ////////////

//////////////END NEED TO BUILD DUMP FUNCTIONS ////////////


static rdg_vertex_p
find_vertex_with_instrs (rdg_p rdg, Instruction instrs)
{
  rdg_vertex_p vertex = NULL;
  unsigned int i;
  
  for (i = 0; i < RDG_NBV (rdg) && vertex == NULL; i++)
    if (RDGV_INSTRS(RDG_VERTEX (rdg,i)) == instrs)
      vertex = RDG_VERTEX (rdg, i);
  
  return vertex;
}


static bool
contains_dr_p ( Instruction instrs, std::vector<ddr_p> pddr)
{
  ddr_p dd;
//  unsigned int i;
  
//  for (i = 0; VEC_iterate (data_reference_p, datarefs, i, dr); i++)
    for(std::vector<ddr_p>::iterator it=(pddr).begin();it!=(pddr).end();++it)
    {
	dd=*it;
    if (DD_INSTR (dd) == instrs)
      return true;
    }
  return false;

}


static int
number_of_vertices (rdg_p rdg)
{
  BasicBlock bb;
  unsigned int i;
  unsigned int nb_stmts = 0;
  BasicBlock::iterator bsi;
  Loop *loop_nest = RDG_LOOP (rdg);
  BasicBlock *bbs = loop_nest->getBlocks();
  
  for (i = 0; i < loop_nest->getNumBlocks(); i++)
    {
      bb = bbs[i];
    
      /* Test whether the basic block is a direct son of the loop,
         the bbs array contains all basic blocks in DFS order.  */
      if (bb->getParentLoop() == loop_nest)
        /* Iterate of each statement of the basic block.  */
        for (bsi = bb->begin(); bsi!=bb->end(); ++bsi)
//          if (!loop_nest_control_p (rdg, bsi_stmt (bsi)))
            nb_stmts++;
    }
  
  free (bbs);

  return nb_stmts;
}



static void
create_vertices (rdg_p rdg)
{
//  BasicBlock *bbs;
  BasicBlock bb;
  unsigned int i;
  unsigned int vertex_index;
  BasicBlock::iterator bsi;
  Loop *loop_nest = RDG_LOOP (rdg);
  
  RDG_NBV (rdg) = number_of_vertices (rdg);
//  RDG_V (rdg) = XCNEWVEC (struct rdg_vertex, RDG_NBV (rdg));
  
  vertex_index = 0;
  std::vector<BasicBlock * > bbs = loop_nest->getBlocks();
  
  for (i = 0; i < (loop_nest->getNumBlocks()); i++)
    {
      bb = bbs[i];
    
      for (bsi = bb->begin(); bsi!= bb->end(); ++bsi)
        {
	  Instruction instrs = *bsi;
//          if (!loop_nest_control_p (rdg, stmt))
//            {
              rdg_vertex_p v = RDG_VERTEX (rdg, vertex_index);
              RDGV_RDGV_INSTRS (v) = instrs;
              RDGV_N (v) = vertex_index;
              RDGV_BB (v) = i;
              RDGV_COLOR (v) = 0;
              RDGV_DD_P (v) = contains_dr_p (instrs, RDG_DDR (rdg));
//              RDGV_IN (v) = VEC_alloc (rdg_edge_p, heap, RDG_VS) ;
//              RDGV_OUT (v) = VEC_alloc (rdg_edge_p, heap, RDG_VS) ;
//              RDGV_PARTITIONS (v) = VEC_alloc (int, heap, RDG_VS) ;
//              RDGV_SCCS (v) = VEC_alloc (int, heap, RDG_VS) ;
              vertex_index++;
//            }
        }
    }
//  free (bbs);
}

/*
static int
number_of_data_dependences (rdg_p rdg)
{
  unsigned int nb_deps = 0;
  ddr_p ddr;
  unsigned int i;

  for (i = 0; VEC_iterate (ddr_p, RDG_DDR (rdg), i, ddr); i++)
    if (DDR_ARE_DEPENDENT (ddr) == NULL_TREE)
      nb_deps += DDR_NUM_DIST_VECTS (ddr);
  
  return nb_deps;
}
*/
std::vector<ddr>  
compute_data_dependences_for_loop (Loop *loop_nest)
// should be a function of class DG
{
  std::vector<ddr> ddr_0; 
  std::map<Loop*, std::map<Instruction*, std::set<Instruction*> > >::iterator mapit1;
  for (mapit1 = dgOfLoops.begin(); mapit1 != dgOfLoops.end(); ++mapit1) {
    int count = 0;
    std::map<Instruction*, std::set<Instruction*> > dg_temp = mapit1->seconnd;
    std::map<Instruction*, std::set<Instruction*> >::iterator mapit2;
    for (mapit2 = dg_temp.begin; mapit2 != dg_temp.end(); ++mapit2) {
      Instruction *inst = mapit2->first;
      std::set<Instruction*> set_temp = mapit2->second;
      std::set<Instruction*>::iterator setit;
      for (setit = set_temp.begin(); setit != set_temp.end(); ++setit) {
        ddr s;
        s.a = inst;
        s.b = setit;
        ddr_0.pushback(s); 
      }
    }
  }
  return ddr_0;

}

