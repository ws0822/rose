#include "chkptRangeAnalysis.h"

void ChkptRangeVerifAnalysis::visit(const Function& func, const DataflowNode& n, NodeState& state)
{
        //bool modified = false;
        ConstrGraph* cg = dynamic_cast<ConstrGraph*>(state.getLatticeBelow(rangeAnalysis, 0));
        
        printf("ChkptRangeVerifAnalysis::visit()   n.getNode()=%p<%s | %s>, cg=%p\n", n.getNode(), n.getNode()->class_name().c_str(), n.getNode()->unparseToString().c_str(), cg);
        cout << cg->str("    ") << "\n";
        if(isSgExpression(n.getNode()))
        {
                //printf("arrayElt::isValidVarExp(isSgExpression(n.getNode()))=%d, rwAccessLabeler::getAccessType(n.getNode()) == rwAccessLabeler::readAccess = %d\n", arrayElt::isValidVarExp(isSgExpression(n.getNode())), rwAccessLabeler::getAccessType(n.getNode()) == rwAccessLabeler::readAccess);
                // if this is an array reference
                if(arrayElt::isValidVarExp(isSgExpression(n.getNode())))
                {
                        arrayElt arr(n.getNode());
                        
                        // if this is a write to the given array
                        if(rwAccessLabeler::getAccessType(n.getNode()) == rwAccessLabeler::readAccess)
                        {
                                // we only deal with 1-dimensional matrixes
                                ROSE_ASSERT(arr.getIndexExprs()->size() == 1);
                                
                                SgExpression* indexExpr = *(arr.getIndexExprs()->begin());
                                varID arrayVar = arr.getArrayVar();
                                
                                varID j, k;
                                bool negJ, negK;
                                long c;
                                
                                // if the array's index expression is parseable as j+c
                                if(cfgUtils::parseAddition(indexExpr, j, negJ, k, negK, c) && k==zeroVar && !negJ)
                                {
                                        if(cg->mustOutsideRange(j, 1, c, arrayVar))
                                                cout << "    Safe!\n";
                                        else
                                                cout << "    Unsafe!\n";
                                        
                                        //modified = cg->shortenArrayRange(arrayVar, j, 1, c) || modified;
                                }
                                printf("cfgUtils::parseAddition(indexExpr, j, negJ, k, negK, c)=%d,   k=%s negJ=%d\n", cfgUtils::parseAddition(indexExpr, j, negJ, k, negK, c), k.str().c_str(), negJ);
                                // else, the array's index expression is complex and we don't change its must-overwrite range
                        }
                }
        }
}

map<varID, Lattice*> ChkptRangeAnalysis::constVars;

// generates the initial lattice state for the given dataflow node, in the given function, with the given NodeState
//vector<Lattice*> ChkptRangeAnalysis::genInitState(const Function& func, const DataflowNode& n, const NodeState& state)
void ChkptRangeAnalysis::genInitState(const Function& func, const DataflowNode& n, const NodeState& state,
                                           vector<Lattice*>& initLattices, vector<NodeFact*>& initFacts)
{
        //vector<Lattice*> initState;
/*printf("ChkptRangeAnalysis::genInitState() n=%p<%s | %s>\n", n.getNode(), n.getNode()->class_name().c_str(), n.getNode()->unparseToString().c_str());
printf("ChkptRangeAnalysis::genInitState() state=%p\n", &state);*/
        
        FiniteVarsExprsProductLattice* divL = dynamic_cast<FiniteVarsExprsProductLattice*>(state.getLatticeBelow(divAnalysis, 0));
        initLattices.push_back(new ConstrGraph(func, n, state, ldva, divL, false, string("    ")));
        
        //return initLattices;
}

// Returns a map of special constant variables (such as zeroVar) and the lattices that correspond to them
// These lattices are assumed to be constants: it is assumed that they are never modified and it is legal to 
//    maintain only one copy of each lattice may for the duration of the analysis.
map<varID, Lattice*>& ChkptRangeAnalysis::genConstVarLattices() const
{
        return constVars;
}
        
bool ChkptRangeAnalysis::transfer(const Function& func, const DataflowNode& n, NodeState& state, const vector<Lattice*>& dfInfo)
{
        bool modified = false;
        ConstrGraph* cg = dynamic_cast<ConstrGraph*>(dfInfo.front());
        
        cg->beginTransaction();
        
        // upgrade cg to bottom if it is currently uninitialized
        cg->setToTop(true);
        
        printf("ChkptRangeAnalysis::transfer()   isSgExpression(n.getNode())=%p\n", isSgExpression(n.getNode()));
        if(isSgExpression(n.getNode()))
        {
                // if this is an array reference
                if(arrayElt::isValidVarExp(isSgExpression(n.getNode())))
                {
                        arrayElt arr(n.getNode());
                        
                        // if this is a write to the given array
                        if(rwAccessLabeler::getAccessType(n.getNode()) == rwAccessLabeler::writeAccess)
                        {
                                // we only deal with 1-dimensional matrixes
                                ROSE_ASSERT(arr.getIndexExprs()->size() == 1);
                                
                                SgExpression* indexExpr = *(arr.getIndexExprs()->begin());
                                varID arrayVar = arr.getArrayVar();
                                
                                varID j, k;
                                bool negJ, negK;
                                long c;
                                
                                // if the array's index expression is parseable as j+c
                                if(cfgUtils::parseAddition(indexExpr, j, negJ, k, negK, c) && k==zeroVar && !negJ) {
// !!! Array functionality is not yet re-enabled in Constraint Graphs
// !!!                                  modified = cg->shortenArrayRange(arrayVar, j, 1, c) || modified;
                                        ROSE_ASSERT(0);
                                }
                                // else, the array's index expression is complex and we don't change its must-overwrite range
                        }
                }
                else
                {
                        short op;
                        varID i, j, k;
                        bool negJ, negK;
                        long c;
                        if(cfgUtils::parseAssignment(isSgExpression(n.getNode()), op, i, j, negJ, k, negK, c))
                        {
                                printf("ChkptRangeAnalysis::transfer()   cfgUtils::parseAssignment(%p, %d, %s, %s, %d, %s, %d, %ld)\n", 
                                 isSgExpression(n.getNode()), op, i.str().c_str(), j.str().c_str(), negJ, k.str().c_str(), negK, c);
                                // i = j or i = c
                                if(op == cfgUtils::none)
                                {
                                        modified = cg->assign(i, j, 1, 1, c) || modified;
                                }
                                // i = (-)j+(-)k+c
                                else if(op == cfgUtils::add)
                                {
                                        printf("ChkptRangeAnalysis::transfer() op == cfgUtils::add\n");
                                        // i = j + c
                                        if(k == zeroVar && negJ==false)
                                        {
                                        printf("ChkptRangeAnalysis::transfer() k == zeroVar && negJ==false\n");
                                                modified = cg->assign(i, j, 1, 1, c) || modified;
                                        }
                                }
                                // i = (-)j*(-)k*c
                                else if(op == cfgUtils::mult)
                                {
                                        printf("ChkptRangeAnalysis::transfer() op == cfgUtils::mult\n");
                                        // i = j*b + 0
                                        if(k == oneVar && negJ==false)
                                        {
                                                printf("ChkptRangeAnalysis::transfer() k == zeroVar && negJ==false\n");
                                                modified = cg->assign(i, j, 1, c, 0) || modified;
                                        }
                                }
                                // i = (-)j/(-)k/c
                                else if(op == cfgUtils::divide)
                                {
                                        printf("ChkptRangeAnalysis::transfer() op == cfgUtils::divide\n");
                                        // i = j/c == i*c = j
                                        if(k == oneVar && negJ==false)
                                        {
                                                modified = cg->assign(i, j, c, 1, 0) || modified;
                                        }
                                }
                        }
                }
        }
        else if(isSgInitializedName(n.getNode()))
        {
                SgInitializedName* initName = isSgInitializedName(n.getNode());
                varID var(initName);
                
                // if this is the declaration of an array
                ROSE_ASSERT(0);
// !!! Array functionality is not yet re-enabled in Constraint Graphs
// !!!          if(cg->isArray(var))
                {
                        // add the constraint 0<=$var to cg
                        cg->setVal(zeroVar, var, 1, 1, 0);
                }
        }
        
cout << "mid-Transfer Function:\n";
cout << cg->str("    ") << "\n";
        
        // incorporate this node's inequalities from conditionals
        incorporateConditionalsInfo(func, n, state, dfInfo);
        
cout << "mid2-Transfer Function:\n";
cout << cg->str("    ") << "\n";
        
        // incorporate this node's divisibility information
// noDivVars    incorporateDivInfo(func, n, state, dfInfo);

cout << "late-Transfer Function:\n";
cout << cg->str("    ") << "\n";
        
        cg->endTransaction();
        
        //cg->beginTransaction();
// noDivVars    removeConstrDivVars(func, n, state, dfInfo);
// noDivVars    cg->divVarsClosure();
        //cg->endTransaction();
        
        return modified;
}

// incorporates the current node's inequality information from conditionals (ifs, fors, etc.) into the current node's 
// constraint graph
// returns true if this causes the constraint graph to change and false otherwise
bool ChkptRangeAnalysis::incorporateConditionalsInfo(const Function& func, const DataflowNode& n, 
                                                     NodeState& state, const vector<Lattice*>& dfInfo)
{
        bool modified = false;
        ConstrGraph* cg = dynamic_cast<ConstrGraph*>(dfInfo.front());
        
        printf("incorporateConditionalsInfo()\n");
        affineInequalityFact* ineqFact = (affineInequalityFact*)state.getFact(affIneqPlacer, 0);
        if(ineqFact)
        {
                //cout << "Node <"<<n.getNode()->class_name()<<" | "<<n.getNode()->unparseToString()<<">\n";
                for(set<varAffineInequality>::iterator it = ineqFact->ineqs.begin(); it!=ineqFact->ineqs.end(); it++)
                {
                        varAffineInequality varIneq = *it;
                        //cout << varIneq.getIneq().str(varIneq.getX(), varIneq.getY(), "    ") << "\n";
                        modified = cg->setVal(varIneq.getX(), varIneq.getY(), varIneq.getIneq().getA(), 
                                              varIneq.getIneq().getB(), varIneq.getIneq().getC()) || modified;
                }
        }
        return modified;
}

// incorporates the current node's divisibility information into the current node's constraint graph
// returns true if this causes the constraint graph to change and false otherwise
// noDivVars bool ChkptRangeAnalysis::incorporateDivInfo(const Function& func, const DataflowNode& n, NodeState& state, const vector<Lattice*>& dfInfo)
// noDivVars {
// noDivVars    bool modified = false;
// noDivVars    ConstrGraph* cg = dynamic_cast<ConstrGraph*>(dfInfo.front());
// noDivVars    
// noDivVars    printf("incorporateDivInfo()\n");
// noDivVars    FiniteVarsExprsProductLattice* prodL = dynamic_cast<FiniteVarsExprsProductLattice*>(state.getLatticeBelow(divAnalysis, 0));
// noDivVars    set<varID> liveVars = getAllLiveVarsAt(ldva, n, state, "    ");
// noDivVars    for(varIDSet::iterator it = liveVars.begin(); it!=liveVars.end(); it++)
// noDivVars    {
// noDivVars            varID var = *it;
// noDivVars            cg->addDivVar(var);
// noDivVars            
// noDivVars            /* // create the divisibility variable for the current variable 
// noDivVars            varID divVar = ConstrGraph::getDivScalar(var);*/
// noDivVars            
// noDivVars            /*DivLattice* varDivL = dynamic_cast<DivLattice*>(prodL->getVarLattice(func, var));
// noDivVars            
// noDivVars            // incorporate this variable's divisibility information (if any)
// noDivVars            if(varDivL->getLevel() == DivLattice::divKnown && !(varDivL->getDiv()==1 && varDivL->getRem()==0))
// noDivVars            {
// noDivVars                    modified = cg->addDivVar(var, varDivL->getDiv(), varDivL->getRem()) || modified;
// noDivVars            }
// noDivVars            else if(varDivL->getLevel() != DivLattice::bottom)
// noDivVars            {
// noDivVars                    modified = cg->addDivVar(var, 1, 0) || modified;
// noDivVars            }*/
// noDivVars    }
// noDivVars    
// noDivVars    return modified;
// noDivVars }

// For any variable for which we have divisibility info, remove its constraints to other variables (other than its
// divisibility variable)
// noDivVars bool ChkptRangeAnalysis::removeConstrDivVars(const Function& func, const DataflowNode& n, NodeState& state, const vector<Lattice*>& dfInfo)
// noDivVars {
// noDivVars    bool modified = false;
// noDivVars    ConstrGraph* cg = dynamic_cast<ConstrGraph*>(dfInfo.front());
// noDivVars    
// noDivVars    printf("removeConstrDivVars()\n");
// noDivVars    
// noDivVars    FiniteVarsExprsProductLattice* prodL = dynamic_cast<FiniteVarsExprsProductLattice*>(state.getLatticeBelow(divAnalysis, 0));
// noDivVars    set<varID> liveVars = getAllLiveVarsAt(ldva, n, state, "    ");
// noDivVars    for(varIDSet::iterator it = liveVars.begin(); it!=liveVars.end(); it++)
// noDivVars    {
// noDivVars            varID var = *it;
// noDivVars            cg->disconnectDivOrigVar(var);
// noDivVars            /*DivLattice* varDivL = dynamic_cast<DivLattice*>(prodL->getVarLattice(func, var));
// noDivVars            
// noDivVars            // incorporate this variable's divisibility information (if any)
// noDivVars            if(varDivL->getLevel() == DivLattice::divKnown && !(varDivL->getDiv()==1 && varDivL->getRem()==0))
// noDivVars            {
// noDivVars                    cg->disconnectDivOrigVar(var, varDivL->getDiv(), varDivL->getRem());
// noDivVars            }
// noDivVars            else if(varDivL->getLevel() != DivLattice::bottom)
// noDivVars            {
// noDivVars                    cg->disconnectDivOrigVar(var, 1, 0);
// noDivVars            }*/
// noDivVars    }
// noDivVars    
// noDivVars    cout << cg->str("    ") << "\n";
// noDivVars    
// noDivVars    return modified;
// noDivVars }

