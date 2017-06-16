/**
 * -==== MutantSelection.h
 *
 *                MuLL Multi-Language LLVM Mutation Framework
 *
 * This file is distributed under the University of Illinois Open Source
 * License. See LICENSE.TXT for details. 
 *  
 * \brief     Implementation of mutant selection based on dependency analysis and ... (this make use of DG - https://github.com/mchalupa/dg).
 */
 
////Uses graphviz for graph representation -- http://www.graphviz.org/pdf/Agraph.pdf  and  http://www.graphviz.org/pdf/libguide.pdf
//// Read/write dot files wit agread/agwrite
//// install 'libcgraph6'
//#include "cgraph.h"

#ifndef __KLEE_SEMU_GENMU_mutantsSelection_MutantSelection__
#define __KLEE_SEMU_GENMU_mutantsSelection_MutantSelection__

#include <unordered_set>

namespace dg { class LLVMDependenceGraph; }

#include "../usermaps.h"
#include "../typesops.h"    //JsonBox


/*class MutantNode : public Node<MutantDependenceGraph, MuLL::MutantIDType, MutantNode>
{
public:
    MutantNode(MuLL::MutantIDType id)
        :dg::Node<MutantDependenceGraph, MuLL::MutantIDType, MutantNode>(id){}

    ~MutantNode();

    llvm::Value *getID() const { return getKey(); }
};*/

class MutantDependenceGraph //: public DependenceGraph<MutantNode>
{
    struct MutantDepends
    {
        std::unordered_set<MuLL::MutantIDType> outDataDependents;   // x ---> x
        std::unordered_set<MuLL::MutantIDType> inDataDependents;   // this ---> x
        std::unordered_set<MuLL::MutantIDType> outCtrlDependents;   // x ---> x
        std::unordered_set<MuLL::MutantIDType> inCtrlDependents;   // this ---> x
        
        std::unordered_set<MuLL::MutantIDType> tieDependents;   // Share IR (mutants on same statement)
    };
  private:
    std::vector<MutantDepends> mutantDGraphData;  //Adjacent List
    std::unordered_map<MuLL::MutantIDType, std::unordered_set<llvm::Value const *>> mutant2IRset;
    std::unordered_map<llvm::Value const *, std::unordered_set<MuLL::MutantIDType>> IR2mutantset;
    
    void addDataDependency (MuLL::MutantIDType fromID, MuLL::MutantIDType toID)
    {
        bool isNew = mutantDGraphData[fromID].outDataDependents.insert(toID).second;
        //assert (isNew && "Error: mutant inserted twice as another's out Data.");
        isNew = mutantDGraphData[toID].inDataDependents.insert(fromID).second;
        //assert (isNew && "Error: mutant inserted twice as another's in Data.");
    }
    
    void addCtrlDependency (MuLL::MutantIDType fromID, MuLL::MutantIDType toID)
    {
        bool isNew = mutantDGraphData[fromID].outCtrlDependents.insert(toID).second;
        //assert (isNew && "Error: mutant inserted twice as another's out Ctrl.");
        isNew = mutantDGraphData[toID].inCtrlDependents.insert(fromID).second;
        //assert (isNew && "Error: mutant inserted twice as another's in CTRL.");
    }
    
    void addTieDependency (MuLL::MutantIDType id1, MuLL::MutantIDType id2)
    {
        bool isNew = mutantDGraphData[id1].tieDependents.insert(id2).second;
        //assert (isNew && "Error: mutant inserted twice as another's tie.");
        isNew = mutantDGraphData[id2].tieDependents.insert(id1).second;
        //assert (isNew && "Error: mutant inserted twice as another's tie.");
    }
    
    void addDataCtrlFor (dg::LLVMDependenceGraph const *subIRDg);
    
  public:
    std::unordered_set<llvm::Value const *> &getIRsOfMut (MuLL::MutantIDType id) {return mutant2IRset.at(id);}
    bool isBuilt () {return (! mutant2IRset.empty());}
    
    MuLL::MutantIDType getMutantsNumber() {return mutantDGraphData.size()-1;}
    
    const std::unordered_set<MuLL::MutantIDType> &getOutDataDependents(MuLL::MutantIDType mutant_id) {return mutantDGraphData[mutant_id].outDataDependents;}
    const std::unordered_set<MuLL::MutantIDType> &getInDataDependents(MuLL::MutantIDType mutant_id) {return mutantDGraphData[mutant_id].inDataDependents;}
    const std::unordered_set<MuLL::MutantIDType> &getOutCtrlDependents(MuLL::MutantIDType mutant_id) {return mutantDGraphData[mutant_id].outCtrlDependents;}
    const std::unordered_set<MuLL::MutantIDType> &getInCtrlDependents(MuLL::MutantIDType mutant_id) {return mutantDGraphData[mutant_id].inCtrlDependents;}
    
    const std::unordered_set<MuLL::MutantIDType> &getTieDependents(MuLL::MutantIDType mutant_id) {return mutantDGraphData[mutant_id].tieDependents;}
    
    bool build (llvm::Module const &mod, dg::LLVMDependenceGraph const *irDg, MutantInfoList const &mutInfos, std::string mutant_depend_filename);
    
    void dump(std::string filename);
    
    void load(std::string filename, MutantInfoList const &mutInfos);
};


class MutantSelection
{
  private:
    
    llvm::Module &subjectModule;
    MutantInfoList const &mutantInfos;
    
    //dg::LLVMDependenceGraph *IRDGraph;
    MutantDependenceGraph mutantDGraph;
    
    ////
    void buildDependenceGraphs(std::string mutant_depend_filename, bool rerundg, bool isFlowSensitive = false, bool isClassicCtrlDepAlgo = true);
    MuLL::MutantIDType pickMutant (std::unordered_set<MuLL::MutantIDType> const &candidates, std::vector<double> const &scores);
    void relaxMutant (MuLL::MutantIDType mutant_id, std::vector<double> &scores);
    
  public:
    MutantSelection (llvm::Module &inMod, MutantInfoList const &mInf, std::string mutant_depend_filename, bool rerundg, bool isFlowSensitive): subjectModule(inMod), mutantInfos(mInf) 
    { 
        buildDependenceGraphs(mutant_depend_filename, rerundg, isFlowSensitive); 
    }
    void smartSelectMutants (std::vector<MuLL::MutantIDType> &selectedMutants, double score_threshold=0.5);
    void randomMutants (std::vector<MuLL::MutantIDType> &spreadSelectedMutants, std::vector<MuLL::MutantIDType> &dummySelectedMutants, unsigned long number);
    void randomSDLMutants (std::vector<MuLL::MutantIDType> &selectedMutants, unsigned long number);   //only statement deletion mutants
};

#endif //__KLEE_SEMU_GENMU_mutantsSelection_MutantSelection__
