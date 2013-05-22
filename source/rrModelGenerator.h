#ifndef rrModelGeneratorH
#define rrModelGeneratorH
#include <string>
#include <vector>
#include <list>
#include "rrObject.h"
#include "rrStringList.h"
#include "rrSymbolList.h"
#include "rrCodeBuilder.h"
#include "rrNOMSupport.h"
#include "rrScanner.h"
#include "rrExecutableModel.h"
#include "rr-libstruct/lsMatrix.h"
#include "rr-libstruct/lsLibStructural.h"

using std::string;
using std::vector;
using std::list;
using namespace ls;
namespace rr
{
class Compiler;

/**
 * The interface which generates executable models from sbml source.
 * This can have different concrete implementations such as compiler
 * based generators, JIT'ed or interpreter based ones.
 */
class RR_DECLSPEC ModelGenerator : public rrObject
{
protected:

    /**
     * protected ctor, this is an partially abstract class.
     */
    ModelGenerator();

    bool                                mComputeAndAssignConsevationLaws;
    const string                        mDoubleFormat;
    const string                        mFixAmountCompartments;
    vector<int>                         mLocalParameterDimensions;
    string                              mModelName;

    StringList                          mFunctionNames;
    StringList                          mFunctionParameters;
    StringList                          mDependentSpeciesList;
    StringList                          mIndependentSpeciesList;

    vector<SymbolList>                  mLocalParameterList;

    StringList                          mWarnings;

    /**
     * get various information about the model in a user displayable format.
     */
    virtual string                      getInfo();

    //Pure Virtual functions... =====================================
    virtual string                      convertUserFunctionExpression(const string& equation) = 0;
    virtual void                        substituteEquation(const string& reactionName, Scanner& s, CodeBuilder& sb) = 0;
    virtual void                        substituteWords(const string& reactionName, bool bFixAmounts, Scanner& s, CodeBuilder& sb) = 0;
    virtual void                        substituteToken(const string& reactionName, bool bFixAmounts, Scanner& s, CodeBuilder& sb) = 0;
    virtual string                      findSymbol(const string& varName) = 0;
    virtual int                         readFloatingSpecies() = 0;
    virtual int                         readBoundarySpecies() = 0;
    virtual void                        writeOutSymbolTables(CodeBuilder& sb) = 0;
    virtual void                        writeComputeAllRatesOfChange(CodeBuilder& sb, const int& numIndependentSpecies, const int& numDependentSpecies, DoubleMatrix& L0) = 0;
    virtual void                        writeComputeConservedTotals(CodeBuilder& sb, const int& numFloatingSpecies, const int& numDependentSpecies) = 0;
    virtual void                        writeUpdateDependentSpecies(CodeBuilder& sb, const int& numIndependentSpecies, const int& numDependentSpecies, DoubleMatrix& L0) = 0;
    virtual void                        writeUserDefinedFunctions(CodeBuilder& sb) = 0;
    virtual void                        writeResetEvents(CodeBuilder& sb, const int& numEvents) = 0;
    virtual void                        writeSetConcentration(CodeBuilder& sb) = 0;
    virtual void                        writeGetConcentration(CodeBuilder& sb) = 0;
    virtual void                        writeConvertToAmounts(CodeBuilder& sb) = 0;
    virtual void                        writeConvertToConcentrations(CodeBuilder& sb) = 0;
    virtual void                        writeProperties(CodeBuilder& sb) = 0;
    virtual void                        writeAccessors(CodeBuilder& sb) = 0;
    virtual void                        writeOutVariables(CodeBuilder& sb) = 0;
    virtual void                        writeClassHeader(CodeBuilder& sb) = 0;
    virtual void                        writeTestConstraints(CodeBuilder& sb) = 0;
    virtual void                        writeEvalInitialAssignments(CodeBuilder& sb, const int& numReactions) = 0;
    virtual int                         writeComputeRules(CodeBuilder& sb, const int& numReactions) = 0;
    virtual void                        writeComputeReactionRates(CodeBuilder& sb, const int& numReactions) = 0;
    virtual void                        writeEvalEvents(CodeBuilder& sb, const int& numEvents, const int& numFloatingSpecies) = 0;
    virtual void                        writeEvalModel(CodeBuilder& sb, const int& numReactions, const int& numIndependentSpecies, const int& numFloatingSpecies, const int& numOfRules) = 0;
    virtual void                        writeEventAssignments(CodeBuilder& sb, const int& numReactions, const int& numEvents) = 0;
    virtual void                        writeSetParameterValues(CodeBuilder& sb, const int& numReactions) = 0;
    virtual void                        writeSetCompartmentVolumes(CodeBuilder& sb) = 0;
    virtual void                        writeSetBoundaryConditions(CodeBuilder& sb) = 0;
    virtual void                        writeSetInitialConditions(CodeBuilder& sb, const int& numFloatingSpecies) = 0;
    virtual string                      convertCompartmentToC(const string& compartmentName) = 0;
    virtual string                      convertSpeciesToBc(const string& speciesName) = 0;
    virtual string                      convertSpeciesToY(const string& speciesName) = 0;
    virtual string                      convertSymbolToC(const string& compartmentName) = 0;
    virtual string                      convertSymbolToGP(const string& parameterName) = 0;

    //Non virtuals..
    string                              substituteTerms(const int& numReactions, const string& reactionName, const string& equation);
    ASTNode*                            cleanEquation(ASTNode* ast);
    string                              cleanEquation(const string& equation);
    string                              substituteTerms(const string& reactionName, const string& inputEquation, bool bFixAmounts);
    ls::DoubleMatrix*                   initializeL0(int& nrRows, int& nrCols);
    bool                                expressionContainsSymbol(ASTNode* ast, const string& symbol);
    bool                                expressionContainsSymbol(const string& expression, const string& symbol);
    Symbol*                             getSpecies(const string& id);
    int                                 readGlobalParameters();
    void                                readLocalParameters(const int& numReactions,  vector<int>& localParameterDimensions, int& totalLocalParmeters);
    int                                 readCompartments();
    int                                 readModifiableSpeciesReferences();
    SymbolList                          mModifiableSpeciesReferenceList;


    virtual                             ~ModelGenerator();

    string                              writeDouble(const double& value, const string& format = "%G");

public:
    int                                 mNumModifiableSpeciesReferences;

    /**
     * Refernce to libstruct library
     * this are set by createModel, and for the time being remain after createModel
     * completes.
     */
    LibStructural*                      mLibStruct;

    /**
     * Object that provide some wrappers and new "NOM" functions.
     * this are set by createModel, and for the time being remain after createModel
     * completes.
     */
    NOMSupport*                         mNOM;


    IntStringHashTable                  mMapRateRule;
    SymbolList                          mBoundarySpeciesList;
    SymbolList                          mCompartmentList;
    SymbolList                          mConservationList;
    SymbolList                          mFloatingSpeciesAmountsList;
    SymbolList                          mFloatingSpeciesConcentrationList;
    SymbolList                          mGlobalParameterList;
    int                                 mNumBoundarySpecies;
    int                                 mNumCompartments;
    int                                 mNumDependentSpecies;
    int                                 mNumEvents;
    int                                 mNumFloatingSpecies;
    int                                 mNumGlobalParameters;
    int                                 mNumIndependentSpecies;
    int                                 mNumReactions;
    int                                 mTotalLocalParmeters;
    SymbolList                          mReactionList;

    void                                reset();
    int                                 getNumberOfReactions();
    int                                 numAdditionalRates();        //this variable is the size of moMapRateRule

    StringList                          getCompartmentList();
    StringList                          getConservationList();

    StringList                          getGlobalParameterList();
    StringList                          getLocalParameterList(int reactionId);

    StringList                          getReactionIds();
    SymbolList&                         getReactionListReference();

    StringList                          getFloatingSpeciesConcentrationList();    //Just returns the Ids...!
    SymbolList&                         getFloatingSpeciesConcentrationListReference();

    StringList                          getBoundarySpeciesList();
    SymbolList&                         getBoundarySpeciesListReference();
    SymbolList&                         getGlobalParameterListReference();
    SymbolList&                         getConservationListReference();

    /**
     * certain model generators, such as the compiler based ones
     * generate files such as shared libraries. This specifies the
     * location where they are stored.
     */
    virtual bool                        setTemporaryDirectory(const string& path) = 0;

    /**
     * certain model generators, such as the compiler based ones
     * generate files such as shared libraries. This specifies the
     * location where they are stored.
     */
    virtual string                      getTemporaryDirectory() = 0;

    /**
     * Create an executable model from an sbml string, a LibStructural and a NOMSupport.
     * The libstruct and nom objects must already have the sbml loaded into them.
     *
     * For the time being, this sets up a bunch of ivars, such as mLibStruct and mNOM,
     * and in order to preserve compatibility, thise will remain pointing to whatever
     * was passed in.
     * Eventually these ivars will either go away or will be cleared. The ModelGenerator
     * is intended ONLY to make models, not query NOM info.
     */
    virtual ExecutableModel             *createModel(const string& sbml, LibStructural *ls, NOMSupport *nom,
            bool forceReCompile, bool computeAndAssignConsevationLaws) = 0;

    /**
     * Get the compiler object that the model generator is using to
     * 'compile' sbml. Certain model generators may be interpreters, in this
     * case, the Compiler interface should still be sufficiently general to
     * manipulate interpreters as well.
     *
     * TODO: Make Compiler an interface.
     */
    virtual                             Compiler *getCompiler() = 0;

    /**
     * Set the name of the compiler to use. In the case of source code generating
     * model generators, this is the exectuable name of the external compiler, i.e.
     * 'gcc', 'icc', etc... For JITing generators, this may have no effect.
     */
    virtual                             bool setCompiler(const string& compiler) = 0;
};
}

#endif
