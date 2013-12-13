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
  std::vector<prdg_vertex_p> sorted_vertices;

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
   prdg_vertex_p *p;
   p=sorted_vertices.get_allocator().allocate(max_f);
  
 // for (i = 0; VEC_iterate (prdg_vertex_p, PRDG_V (g), i, v); i++)
    for(std::vector<prdg_vertex_p>::iterator it=(PRDG_V (g)).begin();it!=(PRDG_V (g)).end();++it)
    {
	v=*it;
      unsigned int idx = lower_bound (sorted_vertices,v);

      sorted_vertices.insert(((sorted_vertices.begin())+idx),v);
//      VEC_safe_insert (prdg_vertex_p, heap, sorted_vertices, idx, v);
    }

//  gcc_assert (VEC_length (prdg_vertex_p, sorted_vertices));

  
  while (sorted_vertices.size())
    {
//      v = VEC_pop (prdg_vertex_p, sorted_vertices);
      v = sorted_vertices[sorted_vertices.size()-1];
      if (PRDGV_COLOR (v) == VERTEX_WHITE)
	dfs_rdgp_visit (g, v, &t, ++scc);
	sorted_vertices.pop_back();
    }

//  VEC_free (prdg_vertex_p, heap, sorted_vertices);
  sorted_vertices.get_allocator().deallocate(p,max_f);

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


