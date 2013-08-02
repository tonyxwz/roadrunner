/*
 * rrLLVMCodeGenerator.cpp
 *
 * Created on: Jun 16, 2013
 *
 * Author: Andy Somogyi,
 *     email decode: V1 = "."; V2 = "@"; V3 = V1;
 *     andy V1 somogyi V2 gmail V3 com
 */

#include "rrLLVMASTNodeCodeGen.h"
#include "rrLLVMException.h"
#include "rrOSSpecifics.h"
#include "rrLLVMIncludes.h"
#include "rrLogger.h"

#include <sbml/math/ASTNode.h>
#include <Poco/Logger.h>

using namespace libsbml;
using namespace llvm;
using namespace std;

namespace rr
{

LLVMASTNodeCodeGen::LLVMASTNodeCodeGen(llvm::IRBuilder<> &builder,
        LLVMLoadSymbolResolver &resolver) :
        builder(builder), resolver(resolver)
{
}

LLVMASTNodeCodeGen::~LLVMASTNodeCodeGen()
{
}

Value *LLVMASTNodeCodeGen::binaryExprCodeGen(const ASTNode *ast)
{
    Value *result = 0;

    Value *lhs = codeGen(ast->getLeftChild());
    Value *rhs = codeGen(ast->getRightChild());
    if (lhs == 0 || rhs == 0)
    {
        // TODO: report this error???
        return 0;
    }

    switch (ast->getType())
    {
    case AST_PLUS:
        result = builder.CreateFAdd(lhs, rhs, "addtmp");
        break;
    case AST_MINUS:
        result = builder.CreateFSub(lhs, rhs, "subtmp");
        break;
    case AST_TIMES:
        result = builder.CreateFMul(lhs, rhs, "multmp");
        break;
    case AST_DIVIDE:
        result = builder.CreateFDiv(lhs, rhs, "divtmp");
        break;
        /*
         case '<':
         L = Builder.CreateFCmpULT(L, R, "cmptmp");
         // Convert bool 0/1 to double 0.0 or 1.0
         return Builder.CreateUIToFP(L, Type::getDoubleTy(getGlobalContext()),
         "booltmp");
         */
    default:
        break;
    }

    return result;
}

llvm::Value* LLVMASTNodeCodeGen::codeGen(const libsbml::ASTNode* ast)
{
    Value *result = 0;

    if (ast == 0)
    {
        throw_llvm_exception("ASTNode is NULL");
    }

    switch (ast->getType())
    {
    case AST_PLUS:                  /* '+' */
    case AST_MINUS:                 /* '-' */
    case AST_TIMES:                 /* '*' */
    case AST_DIVIDE:                /* '/' */
        result = result = applyArithmeticCodeGen(ast);
        break;
    case AST_POWER:                 // '^' sbml considers this an operator,
                                    // left and right child nodes are the first
                                    // 2 child nodes for args.
        result = notImplemented(ast);
        break;
    case AST_INTEGER:
        result = integerCodeGen(ast);
        break;
    case AST_REAL:
    case AST_REAL_E:
    case AST_RATIONAL:
        result = realExprCodeGen(ast);
        break;
    case AST_NAME:
    case AST_NAME_AVOGADRO:
    case AST_NAME_TIME:
        result = nameExprCodeGen(ast);
        break;


    case AST_CONSTANT_E:
    case AST_CONSTANT_FALSE:
    case AST_CONSTANT_PI:
    case AST_CONSTANT_TRUE:

    case AST_LAMBDA:

    case AST_FUNCTION:
    case AST_FUNCTION_ABS:
    case AST_FUNCTION_ARCCOS:
    case AST_FUNCTION_ARCCOSH:
    case AST_FUNCTION_ARCCOT:
    case AST_FUNCTION_ARCCOTH:
    case AST_FUNCTION_ARCCSC:
    case AST_FUNCTION_ARCCSCH:
    case AST_FUNCTION_ARCSEC:
    case AST_FUNCTION_ARCSECH:
    case AST_FUNCTION_ARCSIN:
    case AST_FUNCTION_ARCSINH:
    case AST_FUNCTION_ARCTAN:
    case AST_FUNCTION_ARCTANH:
    case AST_FUNCTION_CEILING:
    case AST_FUNCTION_COS:
    case AST_FUNCTION_COSH:
    case AST_FUNCTION_COT:
    case AST_FUNCTION_COTH:
    case AST_FUNCTION_CSC:
    case AST_FUNCTION_CSCH:
    case AST_FUNCTION_DELAY:
    case AST_FUNCTION_EXP:
    case AST_FUNCTION_FACTORIAL:
    case AST_FUNCTION_FLOOR:
    case AST_FUNCTION_LN:
    case AST_FUNCTION_LOG:
    case AST_FUNCTION_POWER:
    case AST_FUNCTION_ROOT:
    case AST_FUNCTION_SEC:
    case AST_FUNCTION_SECH:
    case AST_FUNCTION_SIN:
    case AST_FUNCTION_SINH:
    case AST_FUNCTION_TAN:
    case AST_FUNCTION_TANH:
        result = notImplemented(ast);
        break;
    default:
        {
            stringstream msg;
            msg << "Unknown ASTNode type of " << ast->getType() << ", from " <<
                    ast->getParentSBMLObject()->toSBML();
            throw_llvm_exception(msg.str());
        }
        break;

    }
    return result;
}

llvm::Value* LLVMASTNodeCodeGen::intrinsicCallCodeGen(
        const libsbml::ASTNode* ast)
{
    return 0;
}

llvm::Value* LLVMASTNodeCodeGen::notImplemented(const libsbml::ASTNode* ast)
{
    char* formula = SBML_formulaToString(ast);
    string str = formula;
    free(formula);

    throw_llvm_exception("AST type Not Implemented Yet: " + str);

    return 0;
}

llvm::Value* LLVMASTNodeCodeGen::nameExprCodeGen(const libsbml::ASTNode* ast)
{
    return resolver.loadSymbolValue(ast->getName());
}

llvm::Value* LLVMASTNodeCodeGen::realExprCodeGen(const libsbml::ASTNode* ast)
{
    return ConstantFP::get(builder.getContext(), APFloat(ast->getReal()));
}

llvm::Value* LLVMASTNodeCodeGen::integerCodeGen(const libsbml::ASTNode* ast)
{
    return ConstantFP::get(builder.getContext(), APFloat((double)ast->getInteger()));
}

llvm::Value* LLVMASTNodeCodeGen::applyArithmeticCodeGen(
        const libsbml::ASTNode* ast)
{
    const int numChildren = ast->getNumChildren();
    const ASTNodeType_t type = ast->getType();

    if (numChildren < 2)
    {
        stringstream err;
        err << "MathML apply node from " << ast->getParentSBMLObject()->toSBML()
                << " must have at least two child nodes";
        throw_llvm_exception(err.str());
    }

    // start at the head of the list and evaluate each subsequent term.
    // accumulator
    Value *acc = codeGen(ast->getChild(0));

    for (int i = 1; i < numChildren; ++i)
    {
        Value *rhs = codeGen(ast->getChild(i));
        switch (type)
        {
        case AST_PLUS:
            acc = builder.CreateFAdd(acc, rhs, "addtmp");
            break;
        case AST_MINUS:
            acc = builder.CreateFSub(acc, rhs, "subtmp");
            break;
        case AST_TIMES:
            acc = builder.CreateFMul(acc, rhs, "multmp");
            break;
        case AST_DIVIDE:
            acc = builder.CreateFDiv(acc, rhs, "divtmp");
            break;
        default:
            break;
        }
    }
    return acc;
}

} /* namespace rr */
