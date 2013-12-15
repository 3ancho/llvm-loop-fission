typedef std::vector<Instruction*> inst_vec;
typedef std::set<Instruction*> inst_set;
typedef std::map<Instruction*, bool> inst_visit;
typedef std::map<Instruction*, std::set<Instruction*> > inst_map_set;

//std::vector<*Instruction> instr_in_loop <Loop *L>

inst_vec dfs(Instruction *start_inst, inst_map_set dg_of_loop, inst_set all_insts, inst_visit *visited){
  inst_vec group;
  inst_set dep_insts = dg_of_loop[start_inst];  //all insts that start_inst related to
  group.push_back(start_inst);
  for (inst_set::iterator it = dep_insts.begin(); it != dep_insts.end(); it++)
    if (!(*visited)[it]) { // not visited
      (*visited)[it] = True;
      group.push_back(inst_vec(it, dg_of_loop, all_insts, *visited));
    }
    
  

