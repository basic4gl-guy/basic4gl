//---------------------------------------------------------------------------
/*  Created 5-Sep-2003: Thomas Mulgrew

    Used to compile source code in BASIC language to TomVM Op codes.
*/

#ifndef TomCompH                                         
#define TomCompH
//---------------------------------------------------------------------------

#include "TomVM.h"
#include "compParse.h"
#include "compFunction.h"
#include "compConstant.h"
#include "PluginLibrary.h"
#include <map>
#include <set>

#define TC_STEPSBETWEENREFRESH 1000
#define TC_MAXOVERLOADEDFUNCTIONS 256           // Allow 256 functions of the same name (should be more than enough for anything...)

////////////////////////////////////////////////////////////////////////////////
// Internal compiler types

// compOperator
// Used for tracking which operators are about to be applied to operands.
// Basic4GL converts infix expressions into reverse polish using an operator
// stack and an operand stack.

enum compOpType {
    OT_OPERATOR,
    OT_RETURNBOOLOPERATOR,
    OT_BOOLOPERATOR,
    OT_LAZYBOOLOPERATOR,
    OT_LBRACKET,
    OT_STOP                         // Forces expression evaluation to stop
};

struct compOperator {
    compOpType  m_type;
    vmOpCode    m_opCode;
    int         m_params;           // 1 -> Calculate "op Reg"          (e.g. "Not Reg")
                                    // 2 -> Calculate "Reg2 op Reg"     (e.g. "Reg2 - Reg")
    int         m_binding;          // Operator binding. Higher = tighter.

    compOperator (compOpType type, vmOpCode opCode, int params, int binding)
        : m_type (type), m_opCode (opCode), m_params (params), m_binding (binding) { ; }
    compOperator ()
        : m_type (OT_OPERATOR), m_opCode (OP_NOP), m_params (0), m_binding (0) { ; }
    compOperator (const compOperator& o)
        : m_type (o.m_type), m_opCode (o.m_opCode), m_params (o.m_params), m_binding (o.m_binding) { ; }
};

struct compStackedOperator {
    compOperator    m_op;               // Stacked operator
    int             m_lazyJumpAddr;     // Address of lazy jump op code (for "and" and "or" operations)

    compStackedOperator(const compOperator& o)
        : m_op(o), m_lazyJumpAddr(-1) { ; }
    compStackedOperator(const compOperator& o, int lazyJumpAddr)
        : m_op(o), m_lazyJumpAddr(lazyJumpAddr) { ; }
};

// CompLabel
// A program label, i.e. a named destination for "goto" and "gosub"s
struct compLabel {
    unsigned int m_offset;              // Instruction index in code
    unsigned int m_programDataOffset;   // Program data offset. (For use with "RESET labelname" command.)

    compLabel (unsigned int offset, int dataOffset)
        : m_offset (offset), m_programDataOffset (dataOffset) { ; }
    compLabel ()
        : m_offset (0), m_programDataOffset (0) { ; }

#ifdef VM_STATE_STREAMING
    void StreamOut(std::ostream& stream);
    void StreamIn(std::istream& stream);
#endif
};

// compJump
// Used to track program jumps. Actuall addresses are patched into jump
// instructions after the main compilation pass has completed. (Thus forward
// jumps are possible.)
struct compJump {
    unsigned int m_jumpInstruction;     // Instruction containing jump instruction
    std::string m_labelName;            // Label to which we are jumping

    compJump (unsigned int instruction, std::string& labelName)
        : m_jumpInstruction (instruction), m_labelName (labelName) { ; }
    compJump ()
        : m_jumpInstruction (0), m_labelName ("") { ; }
};

// compInstructionPos
// Marks a position within a source file
struct compInstructionPos {
    int m_sourceLine, m_sourceCol;

    compInstructionPos() :
        m_sourceLine(0),
        m_sourceCol(0) {}
    compInstructionPos(int line, int col) :
        m_sourceLine(line),
        m_sourceCol(col) {}
};

// compFlowControl
// Used to maintain a stack of open flow control structures, so that when an
// "endif", "else", "next" or "wend" is found we know what it corresponds to.
enum compFlowControlType {
    FCT_IF = 0,
    FCT_ELSE,
    FCT_FOR,
    FCT_WHILE,
    FCT_DO_PRE,         // Do with a pre-condition
    FCT_DO_POST};       // Do with a post-condition

struct compFlowControl {
    compFlowControlType m_type;                         // Type of flow control construct
    int                 m_jumpOut;                      // Index of instruction that jumps past (out) of flow control construct
    int                 m_jumpLoop;                     // Instruction to jump to to loop
    compInstructionPos  m_sourcePos;
    std::string         m_data;                         // Misc data
    bool                m_impliedEndif;                 // If/elseif/else only. True if there is an implied endif after the explict endif
    bool                m_blockIf;                      // True if is a block if. Block ifs require "endifs". Non-block ifs have an implicit endif at the end of the line

    compFlowControl (compFlowControlType type, int jumpOut, int jumpLoop, int line, int col, bool impliedEndif = false, const std::string& data = (std::string) "", bool blockIf = false)
        :   m_type (type),
            m_jumpOut (jumpOut),
            m_jumpLoop (jumpLoop),
            m_sourcePos(line, col),
            m_impliedEndif (impliedEndif),
            m_data (data),
            m_blockIf (blockIf) { ; }
    compFlowControl ()
        :   m_type ((compFlowControlType) 0),
            m_jumpOut (0),
            m_jumpLoop (0),
            m_impliedEndif (false),
            m_data ("") { ; }
};

// Language extension: Operator overloading
//
typedef bool (*compUnOperExt)(  vmValType& regType,     // IN: Current type in register.                                                        OUT: Required type cast before calling function
                                vmOpCode oper,          // IN: Operator being applied
                                int& operFunction,      // OUT: Index of VM_CALL_OPERATOR_FUNC function to call
                                vmValType& resultType,  // OUT: Resulting value type
                                bool& freeTempData);    // OUT: Set to true if temp data needs to be freed
typedef bool (*compBinOperExt)( vmValType& regType,     // IN: Current type in register.                                                        OUT: Required type cast before calling function
                                vmValType& reg2Type,    // IN: Current type in second register (operation is reg2 OP reg1, e.g reg2 + reg1):    OUT: Required type cast before calling function
                                vmOpCode oper,          // IN: Operator being applied
                                int& operFunction,      // OUT: Index of VM_CALL_OPERATOR_FUNC function to call
                                vmValType& resultType,  // OUT: Resulting value type
                                bool& freeTempData);    // OUT: Set to true if temp data needs to be freed

// Container types
typedef std::map<std::string,compOperator> compOperatorMap;
typedef std::set<std::string> compStringSet;
typedef std::map<std::string,compLabel> compLabelMap;
typedef std::map<unsigned int,std::string> compLabelIndex;          // Index offset to label
typedef std::vector<compFuncSpec> compFuncSpecArray;
typedef std::multimap<std::string,unsigned int> compFuncIndex;

// Misc
struct compParserPos {
    unsigned int m_line;
    int m_col;
    compToken m_token;
};

enum compLanguageSyntax {
    LS_TRADITIONAL          = 0,    // As compatible as possible with other BASICs
    LS_BASIC4GL             = 1,    // Standard Basic4GL syntax for backwards compatibility with existing code.
    LS_TRADITIONAL_PRINT    = 2,    // Traditional mode PRINT, but otherwise standard Basic4GL syntax
    LS_TRADITIONAL_SUFFIX   = 3     // Like LS_TRADITIONAL, but also tries to match the variable suffixes of other BASIC types.
};

/// A runtime function
struct compRuntimeFunction {
    int prototypeIndex;

    compRuntimeFunction() : prototypeIndex(-1) {}
    compRuntimeFunction(int _prototypeIndex) : prototypeIndex(_prototypeIndex) {}

    // State streaming
#ifdef VM_STATE_STREAMING
    void StreamOut(std::ostream& stream);
    void StreamIn(std::istream& stream);
#endif
};

enum compUserFunctionType {

	// Function implementation
    UFT_IMPLEMENTATION,

	// Forward declaration
    UFT_FWDDECLARATION,

	// Declaring a function to be implemented in a script
    UFT_RUNTIMEDECLARATION,

	// Declaring a function pointer type
	UFT_POINTER
};

enum compFunctionType
{
	FT_NORMAL,
	FT_OPERATOR_UNARY,
	FT_OPERATOR_BINARY
};

////////////////////////////////////////////////////////////////////////////////
//  compRollbackPoint
//
/// Allows the compiler to rollback cleanly if an error occurs during
/// compilation. Used during runtime compilation to ensure the compiler does
/// not leave the VM in an unstable state.
///
/// Note: Currently not everything is rolled back, just enough to keep the VM
/// stable. There may still be resources used (such as code instructions
/// allocated), but they should be benign and unreachable.
struct compRollbackPoint {

    // Virtual machine rollback
    vmRollbackPoint vmRollback;

    // Runtime functions
    int runtimeFunctionCount;
};

////////////////////////////////////////////////////////////////////////////////
// TomBasicCompiler
//
// Basic4GL v2 language compiler.

class TomBasicCompiler : public HasErrorState {

    // Virtual machine
    TomVM&                          m_vm;

    // Parser
    compParser                      m_parser;

    // DLL manager
	PluginManager&					m_plugins;

    // Settings
    bool                            m_caseSensitive;
    compOperatorMap                 m_unaryOperators;       // Recognised operators. Unary have one operand (e.g NOT x)
    compOperatorMap                 m_binaryOperators;      // Binary have to (e.g. x + y)
	std::map<vmOpCode, std::string> m_operatorOverloads;
    compStringSet                   m_reservedWords;
    compConstantMap                 m_constants;            // Permanent constants.
    compConstantMap                 m_programConstants;     // Constants declared using the const command.
    compFuncSpecArray               m_functions;
    compFuncIndex                   m_functionIndex;        // Maps function name to index of function (in m_functions array)
    compLanguageSyntax              m_syntax;
    compLanguageSyntax              m_defaultSyntax;        // Default language syntax
    std::string                     m_symbolPrefix;         // Prefix all symbols with this text

    // Compiler state
    vmValType                           m_regType, m_reg2Type;
    std::vector<vmValType>              m_operandStack;
    std::vector<compStackedOperator>    m_operatorStack;
    compStackedOperator& OperatorTOS () { return m_operatorStack [m_operatorStack.size () - 1]; }
    compLabelMap                        m_labels;
    compLabelIndex                      m_labelIndex;
    std::vector<compJump>               m_jumps;                // Jumps to fix up
    std::vector<compJump>               m_resets;               // Resets to fix up
    std::vector<compFlowControl>        m_flowControl;          // Flow control structure stack
    compToken                           m_token;
    bool                                m_needColon;            // True if next instruction must be separated by a colon (or newline)
    bool                                m_freeTempData;         // True if need to generate code to free temporary data before the next instruction
    unsigned int                        m_lastLine,
                                        m_lastCol;
    bool                                m_inFunction;
    compInstructionPos                  m_functionStart;
    int                                 m_functionJumpOver;
    std::map<std::string,int>           m_globalUserFunctionIndex;  // Maps function name to index of function
    std::map<int,std::string>           m_userFunctionReverseIndex; // Index->Name lookup. For debug views.
    int                                 m_currentFunction;          // Index of current active user function. Usually this will be the last in the vm.UserFunctions() vector,
                                                                    // can be different in special cases (e.g. when compiler is called from debugger to evaluate an expression).
    std::vector<compRuntimeFunction>    m_runtimeFunctions;
    std::map<std::string,int>           m_runtimeFunctionIndex;
	int									m_currentCodeBlockIndex;

    void ClearState ();
    bool GetToken (bool skipEOL = false, bool dataMode = false);
    bool CheckParser ();

    // Compilation
    void AddInstruction (vmOpCode opCode, vmBasicValType type, const vmValue& val);
	void AddFunctionCallInstruction(const compExtFuncSpec& spec);
    bool AtSeparator            ();
    bool AtSeparatorOrSpecial   ();
    bool SkipSeparators         ();
	bool CompileBeginCodeBlock	();
	bool CompileEndCodeBlock	();
	bool CompileInstruction     ();
    bool CompileStructure       ();
	bool CompileDim				(bool forStruc, bool forFuncParam);
	bool CompileDim				(bool forStruc, bool forFuncParam, vmUserFuncPrototype& prototype);
    bool CompileTokenName       (std::string& name, compTokenType& tokenType, bool allowSuffix);
    bool CompileDataType        (std::string& name, vmValType& type, compTokenType& tokenType);
    bool CompileDimField        (std::string& name, vmValType& type, bool forStruc, bool forFuncParam);
    bool CompileAs              (std::string& name, vmValType& type);
    bool CompileLoadVar         ();
    bool CompileDeref           ();
    bool CompileDerefs          ();
	bool CompileDataLookup		(bool takeAddress);
    bool CompileExpression      (bool mustBeConstant = false);
    bool CompilePush            ();
    bool CompilePop             ();
    bool CompileConvert         (vmBasicValType type);
    bool CompileConvert2        (vmBasicValType type);
    bool CompileConvert         (vmValType type);
    bool CompileConvert2        (vmValType type);
    bool CompileTakeAddress     ();
    bool CompileAssignment      ();
    bool InternalCompileAssignment();
    bool CompileLoad            ();
    bool CompileExpressionLoad  (bool mustBeConstant = false);
    bool CompileLoadConst       ();
    bool CompileOperation       ();
	bool CompileOverloadedOperator(vmOpCode opCode, compFunctionType type);
	bool CompileGoto(vmOpCode jumpType = OP_JUMP);
    bool CompileIf              (bool elseif);
    bool CompileElse            (bool elseif);
    bool CompileEndIf           (bool automatic);
    bool CompileFor             ();
    bool CompileNext            ();
    bool CompileDo              ();
    bool CompileLoop            ();
    bool CompileWhile           ();
    bool CompileWend            ();
    bool CheckName              (std::string name);
    bool CompileFunction        (bool needResult = false);
	bool CompileFunctionCall	(std::string name, compFunctionType type, bool needResult);
	bool CompileSwap            ();
    bool CompileConstant        ();
    bool CompileFreeTempData    ();
    bool CompileAlloc           ();
    bool CompileData            ();
    bool CompileDataRead        ();
    bool CompileDataReset       ();
    bool CompilePrint           (bool forceNewLine);
    bool CompileInput           ();
    bool CompileLanguage        ();
	bool CompileFwdDecl			();
	bool CompileUserFunctionFwdDecl();
    bool CompileUserFunctionRuntimeDecl();
	bool CompileUserFunction    (compUserFunctionType funcType, vmValType& fnPtrType);
	bool CompileUserFunction	(compUserFunctionType funcType);
    bool CompileEndUserFunction (bool hasReturnVal);
	bool CompileStructureFwdDecl();
    bool CheckUnclosedFlowControl();
    bool CheckUnclosedUserFunction();
    bool CompileUserFunctionCall(bool mustReturnValue, bool isRuntimeFunc, bool isFunctionPtr);
    bool CompileUserFuncParam(vmUserFuncPrototype& prototype, int i);
    bool CompileReturn          ();
    bool CompileNull            ();
    bool InternalCompileBindCode();
    bool CompileBindCode        ();
    bool CompileExec            ();
    bool CheckFwdDeclFunctions  ();
	bool CheckFwdDeclStructures ();
	bool CheckUnclosedCodeBlock();
	void AddTimeshareOpCode(compFuncSpec* fn);
	void AddTimeshareOpCode(compExtFuncSpec& extFn) { AddTimeshareOpCode(extFn.m_spec); }
	bool CompileArrayMax		();
	int GetFnPointerPrototypeIndex(const vmUserFuncPrototype& prototype);
	bool CompileUserFunctionAddress();
	bool CheckTypeIsDefined(const vmValType& type);

    bool EvaluateConstantExpression (vmBasicValType& type, vmValue& result, std::string& stringResult);
    bool CompileConstantExpression (vmBasicValType type = VTP_UNDEFINED);
    void InternalCompile        ();

    // Misc
    bool LabelExists (std::string& labelText) {
        return m_labels.find (labelText) != m_labels.end ();
    }
    compLabel& Label (std::string& labelText) {
        assert (LabelExists (labelText));
        return m_labels [labelText];
    }
    void AddLabel (std::string& labelText, const compLabel& label) {
        assert (!LabelExists (labelText));
        m_labels [labelText] = label;
        m_labelIndex [label.m_offset] = labelText;
    }
    compFlowControl& FlowControlTOS () {
        assert (!m_flowControl.empty ());
        return m_flowControl [m_flowControl.size () - 1];
    }
    bool FlowControlTopIs (compFlowControlType type) {
        return      !m_flowControl.empty ()
                &&  FlowControlTOS ().m_type == type;
    }
    compParserPos SavePos ();
    void RestorePos (compParserPos& pos);
    compExtFuncSpec FindFunction(std::string name, vmBasicValType paramType);
	compExtFuncSpec FindFunctionExt(std::string name, bool hasParam, vmBasicValType paramType);
	bool NeedAutoEndif();
    vmUserFunc& _UserFunc() {
        // Return function currently being declared
        assert(m_vm.UserFunctions().size() > 0);
        assert(m_currentFunction >= 0);
        assert((unsigned)m_currentFunction < m_vm.UserFunctions().size());
        return m_vm.UserFunctions()[m_currentFunction];
    }
    vmUserFuncPrototype& UserPrototype() {
        // Return prototype of function currently being declared
        assert(m_vm.UserFunctionPrototypes().size() > 0);
        assert(_UserFunc().prototypeIndex >= 0);
        assert((unsigned)_UserFunc().prototypeIndex < m_vm.UserFunctionPrototypes().size());
        return m_vm.UserFunctionPrototypes()[_UserFunc().prototypeIndex];
    }
    vmBasicValType GetDefaultDataType() {
        // In traditional suffix mode, the default data type is floating point.
        // Otherwise the default Basic4GL data type is integer.
        return m_syntax == LS_TRADITIONAL_SUFFIX
                ? VTP_REAL
                : VTP_INT;
    }

public:

	TomBasicCompiler(TomVM& vm, PluginManager& plugins, bool caseSensitive = false);

    TomVM&              VM ()           { return m_vm; }
    compParser&         Parser ()       { return m_parser; }
    bool                CaseSensitive (){ return m_caseSensitive; }
	PluginManager&		Plugins()       { return m_plugins; }

    void New();
    bool Compile();
    bool CompileOntoEnd();

    ////////////
    // Settings
    compLanguageSyntax Syntax() { return m_syntax; }
    compLanguageSyntax& DefaultSyntax() { return m_defaultSyntax; }
    std::string& SymbolPrefix() { return m_symbolPrefix; }

    //////////////////////
    // Language extension

    // Constants
    void AddConstant (std::string name, compConstant c)   { m_constants [LowerCase (name)] = c; }
    void AddConstant (std::string name, std::string s)    { AddConstant (name, compConstant (s)); }
    void AddConstant (std::string name, int i)            { AddConstant (name, compConstant (i)); }
    void AddConstant (std::string name, unsigned int i)   { AddConstant (name, compConstant (i)); }
    void AddConstant (std::string name, float r)          { AddConstant (name, compConstant (r)); }
    void AddConstant (std::string name, double r)         { AddConstant (name, compConstant (r)); }
    compConstantMap& Constants () { return m_constants; }

    // Functions
    bool IsBuiltinFunction(std::string& name) {
        compFuncIndex::iterator i = m_functionIndex.find (LowerCase (name));
        return i != m_functionIndex.end () && (*i).first == LowerCase (name);
    }
    bool IsUserFunction(std::string name) {
		return IsLocalUserFunction(name) || IsGlobalUserFunction(name);
    }
    bool IsLocalUserFunction(std::string name) {
		return CurrentCodeBlock().userFunctions.find(LowerCase(name)) != CurrentCodeBlock().userFunctions.end();
    }
    bool IsGlobalUserFunction(std::string name) {
        return m_globalUserFunctionIndex.find(LowerCase(name)) != m_globalUserFunctionIndex.end();
    }
	int GetUserFunctionIndex(std::string name)
    {
		assert(IsUserFunction(name));
		if (IsLocalUserFunction(name)) 
			return CurrentCodeBlock().userFunctions[LowerCase(name)];
		else
			return m_globalUserFunctionIndex[LowerCase(name)];
    }
    bool IsRuntimeFunction(std::string& name) {
        return m_runtimeFunctionIndex.find(LowerCase(name)) != m_runtimeFunctionIndex.end();
    }
    bool IsFunction(std::string& name);
    void AddFunction (  std::string         name,
                        vmFunction          func,
                        compParamTypeList   params,
                        bool                brackets,
                        bool                isFunction,
                        vmValType           returnType,
                        bool                timeshare = false,
                        bool                freeTempData = false,
                        compParamValidationCallback paramValidationCallback = NULL,
						bool				conditionalTimeshare = false);
    std::string FunctionName (int index);           // Find function name for function #. Used for debug reporting
    compFuncSpecArray& Functions ()				{ return m_functions; }
    compFuncSpecArray const& Functions () const { return m_functions; }
    compFuncIndex& FunctionIndex ()				{ return m_functionIndex; }
	compFuncIndex const& FunctionIndex() const	{ return m_functionIndex; }
	std::map<std::string, int> const& GlobalUserFunctionIndex() const { return m_globalUserFunctionIndex; }

    // Language features (for context highlighting)
    bool IsReservedWord (std::string& text) {
        compStringSet::iterator si = m_reservedWords.find (text);
        return si != m_reservedWords.end ();
    }
    bool IsConstant (std::string& text) {

        // Check built in constants
        if (m_constants.find (text) != m_constants.end())
            return true;

        // Check DLL constants
        compConstantMap::const_iterator itor;
        return (m_plugins.FindConstant(text, itor));
    }
    bool IsBinaryOperator (std::string& text) {
        compOperatorMap::iterator si = m_binaryOperators.find (text);
        return si != m_binaryOperators.end ();
    }
    bool IsUnaryOperator (std::string& text) {
        compOperatorMap::iterator si = m_unaryOperators.find (text);
        return si != m_unaryOperators.end ();
    }
    bool IsOperator (std::string& text) { return IsBinaryOperator (text) || IsUnaryOperator (text); }
    unsigned int Line () { return m_token.m_line; }
    unsigned int Col  () { return m_token.m_col; }
	vmCodeBlock& CurrentCodeBlock()
	{
		assert(m_vm.IsCodeBlockValid(m_currentCodeBlockIndex));
		return m_vm.GetCodeBlock(m_currentCodeBlockIndex);
	}
	int CurrentCodeBlockIndex() { return m_currentCodeBlockIndex; }

    // Compiler rollback
    compRollbackPoint GetRollbackPoint();
    void Rollback(compRollbackPoint rollbackPoint);

    // Debugging
    std::string DescribeStackCall (unsigned int returnAddr);
    bool TempCompileExpression (
        std::string expression,
        vmValType& valType,
        bool inFunction,
        int currentFunction);
    std::string GetUserFunctionName(int index);

    // Misc
    void InitPlugins();

    // State streaming
#ifdef VM_STATE_STREAMING
    void StreamOut(std::ostream& stream);
    bool StreamIn(std::istream& stream);
#endif
};

#endif
