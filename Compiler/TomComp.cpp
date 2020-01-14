//---------------------------------------------------------------------------
/*  Created 5-Sep-2003: Thomas Mulgrew                                              

    Used to compile source code in BASIC language to TomVM Op codes.
*/

#pragma hdrstop
#include "TomComp.h"
#include "FilenameRoutines.h"
#include <functional>

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif

////////////////////////////////////////////////////////////////////////////////
//  compLabel
#ifdef VM_STATE_STREAMING
void compLabel::StreamOut(std::ostream& stream) {
    WriteLong(stream, m_offset);
    WriteLong(stream, m_programDataOffset);
}

void compLabel::StreamIn(std::istream& stream) {
    m_offset = ReadLong(stream);
    m_programDataOffset = ReadLong(stream);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//  compConstant
#ifdef VM_STATE_STREAMING
void compConstant::StreamOut(std::ostream& stream) {
    WriteLong(stream, (int) m_valType);
    switch(m_valType) {
        case VTP_INT:       WriteLong(stream, m_intVal);        break;
        case VTP_REAL:      WriteFloat(stream, m_realVal);      break;
        case VTP_STRING:    WriteString(stream, m_stringVal);   break;
    }
}

void compConstant::StreamIn(std::istream& stream) {
    m_valType = (vmBasicValType) ReadLong(stream);
    switch(m_valType) {
        case VTP_INT:       m_intVal    = ReadLong(stream);     break;
        case VTP_REAL:      m_realVal   = ReadFloat(stream);    break;
        case VTP_STRING:    m_stringVal = ReadString(stream);   break;
    }
}
#endif

////////////////////////////////////////////////////////////////////////////////
// compRuntimeFunction

#ifdef VM_STATE_STREAMING
void compRuntimeFunction::StreamOut(std::ostream& stream) {
    WriteLong(stream, prototypeIndex);
}

void compRuntimeFunction::StreamIn(std::istream& stream) {
    prototypeIndex = ReadLong(stream);
}
#endif


////////////////////////////////////////////////////////////////////////////////
// TomBasicCompiler

TomBasicCompiler::TomBasicCompiler(TomVM& vm, PluginManager& plugins, bool caseSensitive)
         : m_vm(vm), m_plugins(plugins), m_caseSensitive (caseSensitive), m_defaultSyntax(LS_BASIC4GL) {
    ClearState ();

    // Setup operators
    // Note: From experimentation it appears QBasic binds "xor" looser than
    // "and" and "or". So for compatibility, we will too..
    m_binaryOperators   ["xor"] = compOperator (OT_BOOLOPERATOR, OP_OP_XOR, 2,					10);
    m_binaryOperators   ["or"]  = compOperator (OT_BOOLOPERATOR, OP_OP_OR, 2,					11);
    m_binaryOperators   ["and"] = compOperator (OT_BOOLOPERATOR, OP_OP_AND, 2,					12);
    m_binaryOperators   ["lor"]  = compOperator (OT_LAZYBOOLOPERATOR, OP_OP_OR, 2,				11);
    m_binaryOperators   ["land"] = compOperator (OT_LAZYBOOLOPERATOR, OP_OP_AND, 2,				12);
    m_unaryOperators    ["not"] = compOperator (OT_BOOLOPERATOR, OP_OP_NOT, 1,					20);
    m_binaryOperators   ["="]   = compOperator (OT_RETURNBOOLOPERATOR, OP_OP_EQUAL, 2,			30);
    m_binaryOperators   ["<>"]  = compOperator (OT_RETURNBOOLOPERATOR, OP_OP_NOT_EQUAL, 2,		30);
    m_binaryOperators   [">"]   = compOperator (OT_RETURNBOOLOPERATOR, OP_OP_GREATER, 2,        30);
    m_binaryOperators   [">="]  = compOperator (OT_RETURNBOOLOPERATOR, OP_OP_GREATER_EQUAL, 2,  30);
    m_binaryOperators   ["<"]   = compOperator (OT_RETURNBOOLOPERATOR, OP_OP_LESS, 2,           30);
    m_binaryOperators   ["<="]  = compOperator (OT_RETURNBOOLOPERATOR, OP_OP_LESS_EQUAL, 2,     30);
    m_binaryOperators   ["+"]   = compOperator (OT_OPERATOR, OP_OP_PLUS, 2,						40);
    m_binaryOperators   ["-"]   = compOperator (OT_OPERATOR, OP_OP_MINUS, 2,					40);
    m_binaryOperators   ["*"]   = compOperator (OT_OPERATOR, OP_OP_TIMES, 2,					41);
    m_binaryOperators   ["/"]   = compOperator (OT_OPERATOR, OP_OP_DIV, 2,						42);
    m_binaryOperators   ["%"]   = compOperator (OT_OPERATOR, OP_OP_MOD, 2,						43);
    m_unaryOperators    ["-"]   = compOperator (OT_OPERATOR, OP_OP_NEG, 1,						50);

	// Operator overload function names
	m_operatorOverloads[OP_OP_EQUAL]			= "operator=";
	m_operatorOverloads[OP_OP_NOT_EQUAL]		= "operator<>";
	m_operatorOverloads[OP_OP_GREATER]			= "operator>";
	m_operatorOverloads[OP_OP_GREATER_EQUAL]	= "operator>=";
	m_operatorOverloads[OP_OP_LESS]				= "operator<";
	m_operatorOverloads[OP_OP_LESS_EQUAL]		= "operator<=";
	m_operatorOverloads[OP_OP_PLUS]				= "operator+";
	m_operatorOverloads[OP_OP_MINUS]			= "operator-";
	m_operatorOverloads[OP_OP_TIMES]			= "operator*";
	m_operatorOverloads[OP_OP_DIV]				= "operator/";
	m_operatorOverloads[OP_OP_MOD]				= "operator%";
	m_operatorOverloads[OP_OP_NEG]				= "operator-";

    // Setup reserved words
    m_reservedWords.insert("dim");
    m_reservedWords.insert("goto");
    m_reservedWords.insert("if");
    m_reservedWords.insert("then");
    m_reservedWords.insert("elseif");
    m_reservedWords.insert("else");
    m_reservedWords.insert("endif");
    m_reservedWords.insert("end");
    m_reservedWords.insert("gosub");
    m_reservedWords.insert("return");
    m_reservedWords.insert("for");
    m_reservedWords.insert("to");
    m_reservedWords.insert("step");
    m_reservedWords.insert("next");
    m_reservedWords.insert("while");
    m_reservedWords.insert("wend");
    m_reservedWords.insert("run");
    m_reservedWords.insert("struc");
    m_reservedWords.insert("endstruc");
    m_reservedWords.insert("const");
    m_reservedWords.insert("alloc");
    m_reservedWords.insert("null");
    m_reservedWords.insert("data");
    m_reservedWords.insert("read");
    m_reservedWords.insert("reset");
    m_reservedWords.insert("type");        // QBasic/Freebasic compatibility
    m_reservedWords.insert("as");          //
    m_reservedWords.insert("integer");     //
    m_reservedWords.insert("single");      //
    m_reservedWords.insert("double");      //
    m_reservedWords.insert("string");      //
    m_reservedWords.insert("language");    // Language syntax
    m_reservedWords.insert("traditional");
    m_reservedWords.insert("basic4gl");
    m_reservedWords.insert("traditional_print");
    m_reservedWords.insert("traditional_suffix");
    m_reservedWords.insert("input");
    m_reservedWords.insert("do");
    m_reservedWords.insert("loop");
    m_reservedWords.insert("until");
    m_reservedWords.insert("function");
    m_reservedWords.insert("sub");
    m_reservedWords.insert("endfunction");
    m_reservedWords.insert("endsub");
    m_reservedWords.insert("declare");
    m_reservedWords.insert("runtime");
    m_reservedWords.insert("bindcode");
    m_reservedWords.insert("exec");
	m_reservedWords.insert("include");
	m_reservedWords.insert("arraymax");		// Library mechanism doesn't support any-type parameters. Compiler needs to treat arraymax() as a special case
	m_reservedWords.insert("begincodeblock");
	m_reservedWords.insert("endcodeblock");
}

void TomBasicCompiler::ClearState () {
    m_regType   = VTP_INT;
    m_reg2Type  = VTP_INT;
    m_freeTempData = false;
    m_operandStack.clear ();
    m_operatorStack.clear ();
    m_jumps.clear ();
    m_resets.clear ();
    m_flowControl.clear ();
    m_syntax = m_defaultSyntax;
    m_inFunction = false;
}

void TomBasicCompiler::InitPlugins() {
    m_plugins.StructureManager().AddVMStructures(m_vm.DataTypes());
    m_plugins.CreateVMFunctionSpecs();
}

void TomBasicCompiler::New() {

    // Clear existing program
    m_vm.New ();
    m_lastLine = 0;
    m_lastCol  = 0;

    // Clear state
    m_globalUserFunctionIndex.clear();
    m_userFunctionReverseIndex.clear();
    m_runtimeFunctionIndex.clear();
    ClearState ();
    m_parser.Reset ();
    m_programConstants.clear ();
    m_labels.clear ();
    m_labelIndex.clear ();
    m_currentFunction = -1;
	m_currentCodeBlockIndex = -1;
	InitPlugins();
    m_runtimeFunctions.clear();
}

bool TomBasicCompiler::Compile () {

    // Clear existing program
    New();

    // Compile source code
    InternalCompile ();

    return !Error ();
}

bool TomBasicCompiler::CompileOntoEnd() {

    // Compile code and append to end of program.
    // Like ::Compile(), but does not clear out the existing program first.
    ClearState();
    m_lastLine = 0;
    m_lastCol  = 0;
    m_parser.Reset();
    InternalCompile();
	return !Error();
}

bool TomBasicCompiler::CheckParser () {
    // Check parser for error
    // Copy error state (if any)
    if (m_parser.Error ()) {
        SetError ((std::string) "Parse error: " + m_parser.GetError());
        m_parser.ClearError ();
        return false;
    }
    return true;
}

bool TomBasicCompiler::GetToken (bool skipEOL, bool dataMode) {

    // Read a token
    m_token = m_parser.NextToken (skipEOL, dataMode);
    if (!CheckParser ())
        return false;

    // Text token processing
    if (m_token.m_type == CTT_TEXT) {

        // Apply case sensitivity
        if (!m_caseSensitive)
            m_token.m_text = LowerCase (m_token.m_text);

        // Match against reserved keywords
        compStringSet::iterator si = m_reservedWords.find (m_token.m_text);
        if (si != m_reservedWords.end ())
            m_token.m_type = CTT_KEYWORD;

        // Match against external functions
        else if (IsFunction (m_token.m_text))
            m_token.m_type = CTT_FUNCTION;

        else if (IsUserFunction(m_token.m_text))
            m_token.m_type = CTT_USER_FUNCTION;

        else if (IsRuntimeFunction(m_token.m_text))
            m_token.m_type = CTT_RUNTIME_FUNCTION;

        else {

            // Match against constants

            // Try permanent constants first
            bool isConstant;
            compConstantMap::const_iterator smi = m_constants.find (m_token.m_text);
            isConstant = smi != m_constants.end ();

            // Try plugin DLL constants next
            if (!isConstant)
                isConstant = m_plugins.FindConstant(m_token.m_text, smi);

            // Otherwise try program constants
            if (!isConstant) {
                smi = m_programConstants.find (m_token.m_text);
                isConstant = smi != m_programConstants.end ();
            }

            if (isConstant) {
                m_token.m_type      = CTT_CONSTANT;             // Replace token with constant

                // Replace text with text value of constant
                switch ((*smi).second.m_valType) {
                case VTP_INT:
                    m_token.m_text = IntToString ((*smi).second.m_intVal);
                    break;
                case VTP_REAL:
                    m_token.m_text = RealToString ((*smi).second.m_realVal);
                    break;
                case VTP_STRING:
                    m_token.m_text = (*smi).second.m_stringVal;
                    break;
                }
                m_token.m_valType   = (*smi).second.m_valType;
            }
        }
    }
    else if (m_token.m_type == CTT_CONSTANT && m_token.m_valType == VTP_STRING) {

        // 19-Jul-2003
        // Prefix string text constants with "S". This prevents them matching
        // any recognised keywords (which are stored in lower case).
        // (This is basically a work around to existing code which doesn't
        // check the token type if it matches a reserved keyword).
        m_token.m_text = (std::string) "S" + m_token.m_text;
    }
    return true;
}

bool TomBasicCompiler::NeedAutoEndif() {

    // Look at the top-most control structure
    if (m_flowControl.empty())
        return false;

    compFlowControl top = FlowControlTOS ();

    // Auto endif required if top-most flow control is a non-block "if" or "else"
    return (top.m_type == FCT_IF || top.m_type == FCT_ELSE) && !top.m_blockIf;
}

bool TomBasicCompiler::IsFunction(std::string& name) {
    return IsBuiltinFunction(name) || m_plugins.IsPluginFunction(name);
}

void TomBasicCompiler::InternalCompile () {

    // Allocate a new code block
	m_currentCodeBlockIndex = m_vm.NewCodeBlock();
	bool isMainProgram = m_currentCodeBlockIndex == 0;

    // Clear error state
    ClearError ();
    m_parser.ClearError ();

    // Read initial token
    if (!GetToken (true))
        return;

    // Compile code
    while (!m_parser.Eof () && CompileInstruction ())
        ;

    // Terminate program
    AddInstruction (OP_END, VTP_INT, vmValue ());

    if (!Error ()) {
        // Link up gotos
        std::vector<compJump>::iterator i;
        for (i = m_jumps.begin (); i != m_jumps.end (); i++) {

            // Find instruction
            assert ((*i).m_jumpInstruction < m_vm.InstructionCount ());
            vmInstruction& instr = m_vm.Instruction ((*i).m_jumpInstruction);

            // Point token to goto instruction, so that it will be displayed
            // if there is an error.
            m_token.m_line = instr.m_sourceLine;
            m_token.m_col  = instr.m_sourceChar;

            // Label must exist
            if (!LabelExists ((*i).m_labelName)) {
                SetError ((std::string) "Label: " + (*i).m_labelName + " does not exist");
                return;
            }

            // Patch in offset
            instr.m_value.IntVal () = Label ((*i).m_labelName).m_offset;
        }

        // Link up resets
        for (i = m_resets.begin (); i != m_resets.end (); i++) {

            // Find instruction
            assert ((*i).m_jumpInstruction < m_vm.InstructionCount ());
            vmInstruction& instr = m_vm.Instruction ((*i).m_jumpInstruction);

            // Point token to reset instruction, so that it will be displayed
            // if there is an error.
            m_token.m_line = instr.m_sourceLine;
            m_token.m_col  = instr.m_sourceChar;

            // Label must exist
            if (!LabelExists ((*i).m_labelName)) {
                SetError ((std::string) "Label: " + (*i).m_labelName + " does not exist");
                return;
            }

            // Patch in data offset
            instr.m_value.IntVal () = Label ((*i).m_labelName).m_programDataOffset;
        }

        // Check for open function or flow control structures
        if (!CheckUnclosedUserFunction())
            return;

        if (!CheckUnclosedFlowControl())
            return;

        // Check for not implemented forward declared functions
        if (!CheckFwdDeclFunctions())
            return;

		if (!CheckFwdDeclStructures())
			return;

		if (isMainProgram && !CheckUnclosedCodeBlock())
			return;

/*        if (!m_flowControl.empty ()) {

            // Find topmost structure
            compFlowControl top = FlowControlTOS ();

            // Point parser to it
            m_parser.SetPos (top.m_sourceLine, top.m_sourceCol);

            // Set error message
            switch (FlowControlTOS ().m_type) {
            case FCT_IF:    SetError ("'if' without 'endif'");          break;
            case FCT_ELSE:  SetError ("'else' without 'endif'");        break;
            case FCT_FOR:   SetError ("'for' without 'next'");          break;
            case FCT_WHILE: SetError ("'while' without 'wend'");        break;
            case FCT_DO_PRE:
            case FCT_DO_POST:
                            SetError ("'do' without 'loop'");           break;
            default:        SetError ("Open flow control structure");   break;
            }

            return;
        }    */
    }
}

bool TomBasicCompiler::CheckUnclosedUserFunction() {
    if (m_inFunction) {

        // Point parser to function
        m_parser.SetPos(m_functionStart.m_sourceLine, m_functionStart.m_sourceCol);

        // Return error
        if (UserPrototype().hasReturnVal)
            SetError("'function' without 'endfunction'");
        else
            SetError("'sub' without 'endsub'");
        return false;
    }
    else
        return true;
}

bool TomBasicCompiler::CheckUnclosedFlowControl() {

    // Check for open flow control structures
    if (!m_flowControl.empty ()) {

        // Find topmost structure
        compFlowControl top = FlowControlTOS ();

        // Point parser to it
        m_parser.SetPos (top.m_sourcePos.m_sourceLine, top.m_sourcePos.m_sourceCol);

        // Set error message
        switch (FlowControlTOS ().m_type) {
        case FCT_IF:    SetError ("'if' without 'endif'");          break;
        case FCT_ELSE:  SetError ("'else' without 'endif'");        break;
        case FCT_FOR:   SetError ("'for' without 'next'");          break;
        case FCT_WHILE: SetError ("'while' without 'wend'");        break;
        case FCT_DO_PRE:
        case FCT_DO_POST:
                        SetError ("'do' without 'loop'");           break;
        default:        SetError ("Open flow control structure");   break;
        }

        return false;
    }
    else
        return true;
}

bool TomBasicCompiler::CheckFwdDeclFunctions() {

	for (int i = 0; i < m_vm.UserFunctions().size(); i++)
	{
		const auto& fn = m_vm.UserFunctions()[i];
		if (!fn.implemented)
		{
			std::string name = m_userFunctionReverseIndex[i];
			SetError((std::string)"Function/sub '" + name + "' was DECLAREd, but not implemented");
			return false;
		}
	}

    return true;
}

bool TomBasicCompiler::CheckFwdDeclStructures()
{
	for (const auto& s : m_vm.DataTypes().Structures())
	{
		if (!s.m_isDefined)
		{
			SetError("Structure '" + s.m_name + "' was DECLAREd, but not implemented");
			return false;
		}
	}

	return true;
}

bool TomBasicCompiler::CheckUnclosedCodeBlock()
{
	if (m_currentCodeBlockIndex != 0)
	{
		SetError("'begincodeblock' without 'endcodeblock'");
		return false;
	}

	return true;
}

void TomBasicCompiler::AddTimeshareOpCode(compFuncSpec* fn)
{
	// Add forced or conditional timesharing break op-code if necessary
	if (fn->m_timeshare)
		AddInstruction(OP_TIMESHARE, VTP_INT, vmValue());
	else if (fn->m_conditionalTimeshare)
		AddInstruction(OP_COND_TIMESHARE, VTP_INT, vmValue());
}

void TomBasicCompiler::AddInstruction (vmOpCode opCode, vmBasicValType type, const vmValue& val) {

    // Add instruction, and include source code position audit
    unsigned int line = m_token.m_line, col = m_token.m_col;

    // Prevent line and col going backwards. (Debugging tools rely on source-
    // offsets never decreasing as source code is traversed.)
    if (line < m_lastLine || (line == m_lastLine && col < m_lastCol)) {
        line = m_lastLine;
        col  = m_lastCol;
    }
    m_vm.AddInstruction (vmInstruction (opCode, type, val, line, col));
    m_lastLine = line;
    m_lastCol  = col;
}

void TomBasicCompiler::AddFunctionCallInstruction(const compExtFuncSpec& spec) {

	// Generate code to call function
    if (spec.m_builtin)

        // Builtin function
        AddInstruction(OP_CALL_FUNC, VTP_INT, vmValue(spec.m_spec->m_index));
    else

        // DLL function
        // Note: The DLL index is encoded as the high byte.
        // The 3 low bytes are the function index within the DLL.
        // This may be revised later...
        AddInstruction(OP_CALL_DLL, VTP_INT, vmValue((spec.m_pluginIndex << 24) | (spec.m_spec->m_index & 0x00ffffff)));
}

bool TomBasicCompiler::CompileInstruction () {
    m_needColon = true;                     // Instructions by default must be separated by colons

    // Is it a label?
    compToken nextToken = m_parser.PeekToken(false);
    if (!CheckParser ())
        return false;
    if (    m_token.m_newLine && m_token.m_type == CTT_TEXT
        &&  nextToken.m_text == ":") {

        // Labels cannot exist inside subs/functions
        if (m_inFunction) {
            SetError((std::string)"You cannot use a label inside a function or subroutine");
            return false;
        }

        // Label declaration
        std::string labelName = m_symbolPrefix + m_token.m_text;
        
        // Must not already exist
        if (LabelExists (labelName)) {
            SetError ("Duplicate label name: " + labelName);
            return false;
        }

        // Create new label
        AddLabel (labelName, compLabel (m_vm.InstructionCount (), m_vm.ProgramData ().size ()));

        // Skip label
        if (!GetToken ())
            return false;
    }
    // Determine the type of instruction, based on the current token
    else if (m_token.m_text == "struc" || m_token.m_text == "type") {
        if (!CompileStructure ())
            return false;
    }
    else if (m_token.m_text == "dim") {
        if (!CompileDim (false, false))
            return false;
    }
    else if (m_token.m_text == "goto") {
        if (!GetToken ())
            return false;
        if (!CompileGoto (OP_JUMP))
            return false;
    }
    else if (m_token.m_text == "gosub") {
        if (!GetToken())
            return false;
        if (!CompileGoto (OP_CALL))
            return false;
    }
    else if (m_token.m_text == "return") {
        if (!CompileReturn())
            return false;
    }
    else if (m_token.m_text == "if") {
        if (!CompileIf (false))
            return false;
    }
    else if (m_token.m_text == "elseif") {
        if (!(CompileElse (true) && CompileIf (true)))
            return false;
    }
    else if (m_token.m_text == "else") {
        if (!CompileElse (false))
            return false;
    }
    else if (m_token.m_text == "endif") {
        if (!CompileEndIf (false))
            return false;
    }
    else if (m_token.m_text == "for") {
        if (!CompileFor ())
            return false;
    }
    else if (m_token.m_text == "next") {
        if (!CompileNext ())
            return false;
    }
    else if (m_token.m_text == "while") {
        if (!CompileWhile ())
            return false;
    }
    else if (m_token.m_text == "wend") {
        if (!CompileWend ())
            return false;
    }
    else if (m_token.m_text == "do") {
        if (!CompileDo())
            return false;
    }
    else if (m_token.m_text == "loop") {
        if (!CompileLoop())
            return false;
    }
    else if (m_token.m_text == "end") {
        if (!GetToken ())
            return false;

        // Special case! "End" immediately followed by "if" is syntactically equivalent to "endif"
        if (m_token.m_text == "if") {
            if (!CompileEndIf (false))
                return false;
        }
        // Special case! "End" immediately followed by "function" is syntactically equivalent to "endfunction"
        else if (m_token.m_text == "function") {
            if (!CompileEndUserFunction(true))
                return false;
        }
        else if (m_token.m_text == "sub") {
            if (!CompileEndUserFunction(false))
                return false;
        }
        else
            // Otherwise is "End" program instruction
            AddInstruction (OP_END, VTP_INT, vmValue ());
    }
    else if (m_token.m_text == "run") {
        if (!GetToken ())
            return false;
        AddInstruction (OP_RUN, VTP_INT, vmValue ());
    }
    else if (m_token.m_text == "const") {
        if (!CompileConstant ())
            return false;
    }
    else if (m_token.m_text == "alloc") {
        if (!CompileAlloc ())
            return false;
    }
    else if (m_token.m_text == "data") {
        if (!CompileData ())
            return false;
    }
    else if (m_token.m_text == "read") {
        if (!CompileDataRead ())
            return false;
    }
    else if (m_token.m_text == "reset") {
        if (!CompileDataReset ())
            return false;
    }
    else if (m_token.m_text == "print") {
        if (!CompilePrint(false))
            return false;
    }
    else if (m_token.m_text == "printr") {
        if (!CompilePrint(true))
            return false;
    }
    else if (m_token.m_text == "input") {
        if (!CompileInput ())
            return false;
    }
    else if (m_token.m_text == "language") {
        if (!CompileLanguage())
            return false;
    }
    else if (m_token.m_text == "function" || m_token.m_text == "sub") {
        if (!CompileUserFunction(UFT_IMPLEMENTATION))
            return false;
    }
    else if (m_token.m_text == "endfunction") {
        if (!CompileEndUserFunction(true))
            return false;
    }
    else if (m_token.m_text == "endsub") {
        if (!CompileEndUserFunction(false))
            return false;
    }
    else if (m_token.m_text == "declare") {
        if (!CompileFwdDecl())
            return false;
    }
    else if (m_token.m_text == "runtime") {
        if (!CompileUserFunctionRuntimeDecl())
            return false;
    }
    else if (m_token.m_text == "bindcode") {
        if (!CompileBindCode())
            return false;
    }
    else if (m_token.m_text == "exec") {
        if (!CompileExec())
            return false;
    }
	else if (m_token.m_text == "begincodeblock")
	{
		if (!CompileBeginCodeBlock())
			return false;
	}
	else if (m_token.m_text == "endcodeblock")
	{
		if (!CompileEndCodeBlock())
			return false;
	}
    else if (m_token.m_type == CTT_FUNCTION) {
        if (!CompileFunction ())
            return false;
    }
    else if (m_token.m_type == CTT_USER_FUNCTION) {
        if (!CompileUserFunctionCall(false, false, false))
            return false;
    }
    else if (m_token.m_type == CTT_RUNTIME_FUNCTION) {
        if (!CompileUserFunctionCall(false, true, false))
            return false;
    }
    else if (!CompileAssignment ())
        return false;

    // Free any temporary data (if necessary)
    if (!CompileFreeTempData ())
        return false;

    // Skip separators (:, EOL)
    if (!SkipSeparators ())
        return false;

    return true;
}

bool TomBasicCompiler::AtSeparatorOrSpecial () {

    // Note: Endif is special, as it doesn't require a preceding colon to separate
    // from other instructions on the same line.
    // e.g.
    //      if CONDITION then INSTRUCTION(S) endif
    // Likewise else
    //      if CONDITION then INSTRUCTION(S) else INSTRUCTION(S) endif
    //
    // (unlike while, which would have to be written as:
    //      while CONDITION: INSTRUCTION(S): wend
    // )
    return AtSeparator () || m_token.m_text == "endif" || m_token.m_text == "else" || m_token.m_text == "elseif";
}

bool TomBasicCompiler::AtSeparator () {
    return      m_token.m_type == CTT_EOL
            ||  m_token.m_type == CTT_EOF
            ||  m_token.m_text == ":";
}

bool TomBasicCompiler::SkipSeparators () {

    // Expect separator. Either as an EOL or EOF or ':'
    if (    m_needColon
        &&  !AtSeparatorOrSpecial ()) {
        SetError ("Expected ':'");
        return false;
    }

    // Skip separators
    while (     m_token.m_type == CTT_EOL
            ||  m_token.m_type == CTT_EOF
            ||  m_token.m_text == ":") {

        // If we reach the end of the line, insert any necessary implicit "endifs"
        if (m_token.m_type == CTT_EOL || m_token.m_type == CTT_EOF) {

            // Generate any automatic endifs for the previous line
            while (NeedAutoEndif())
                if (!CompileEndIf(true))
                    return false;

            // Convert any remaining flow control structures to block
            for (std::vector<compFlowControl>::iterator i = m_flowControl.begin();
                i != m_flowControl.end();
                i++)
                i->m_blockIf = true;
        }
        
        if (!GetToken (true))
            return false;
    }

    return true;
}

bool TomBasicCompiler::CompileBeginCodeBlock()
{
	// A code block is compiled to mimic how the code would be compiled at runtime.
	// The compiler will allocate a code block number, and insert "jump around" and
	// "return" op codes around the compiled code.
	// This affects "runtime" function definitions and "GetFunctionByName" behaviour.
	// Programs can lookup code blocks at runtime with "GetCodeBlockByName" function.
	
	// Note: This was implemented mainly so I could port my Thunderbolt game to Basic4GL Mobile,
	// without having to include the whole compiler in the runtime.
	// I don't expect this command will benefit anyone else, and would probably be more
	// confusing than helpful, so I'm going to leave it undocumented.

	// Code blocks cannot be nested.
	// Check we are in the main code block.
	if (m_currentCodeBlockIndex != 0)
	{
		SetError("Must end previous code block with 'endcodeblock' before beginning a new one");
		return false;
	}

	// Also cannot start a code block within a function or flow control
	if (m_inFunction)
	{
		SetError("Cannot begin a code block inside a user function");
		return false;
	}

	if (!m_flowControl.empty())
	{
		SetError("Cannot begin a code block inside a flow control structure");
		return false;
	}

	// Skip BEGINCODEBLOCK
	if (!GetToken())
		return false;

	// Code block "filename" follows 
	vmValue dummy;
	std::string filename;
	vmBasicValType valType = VTP_STRING;
	if (!EvaluateConstantExpression(valType, dummy, filename))
		return false;

	// To match runtime compilation code block will be terminated with OP_RETURN.
	// Therefore we can't just let execution carry on into the code block from the 
	// previous instruction. Instead we generate a jump around.
	AddInstruction(OP_JUMP, VTP_INT, 0);				// Offset will be filled in when code block ends

	// Allocate new code block
	m_currentCodeBlockIndex = m_vm.NewCodeBlock();
	CurrentCodeBlock().SetFilename(filename);

	return true;
}

bool TomBasicCompiler::CompileEndCodeBlock()
{
	// Must be inside code block
	if (m_currentCodeBlockIndex == 0)
	{
		SetError("'endcodeblock' without 'begincodeblock'");
		return false;
	}

	// Also cannot end a code block within a function or flow control
	if (m_inFunction)
	{
		SetError("Cannot end a code block inside a user function");
		return false;
	}

	if (!m_flowControl.empty())
	{
		SetError("Cannot end a code block inside a flow control structure");
		return false;
	}


	// Skip ENDCODEBLOCK
	if (!GetToken())
		return false;

	// Add OP_RETURN to end
	AddInstruction(OP_RETURN, VTP_INT, vmValue());

	// Set "jump around offset"
	int jumpOpCode = CurrentCodeBlock().programOffset - 1;
	m_vm.Instruction(jumpOpCode).m_value = vmValue((int)m_vm.InstructionCount());

	// Resume compiling main code block
	m_currentCodeBlockIndex = 0;

	return true;
}

bool TomBasicCompiler::CompileStructure () {

    // Skip STRUC
    std::string keyword = m_token.m_text;               // Record whether "struc" or "type" was used to declare type  
    if (!GetToken ())
        return false;

    // Check that we are not already inside a function
    if (m_inFunction) {
        SetError("Cannot define a structure inside a function or subroutine");
        return false;
    }

    // Check that we are not inside a control structure
    if (!CheckUnclosedFlowControl())
        return false;

    // Expect structure name
    if (m_token.m_type != CTT_TEXT) {
        SetError ("Expected structure name");
        return false;
    }
    std::string name = m_symbolPrefix + m_token.m_text;
    if (!CheckName (name))
        return false;

	// Check if already declared
    if (m_vm.DataTypes().StrucStored(name)) 
	{	
		m_vm.DataTypes().MakeStrucCurrent(name);

		// If already declared we can define it, but only if it has not already been defined
		if (m_vm.DataTypes().CurrentStruc().m_isDefined) {
			SetError((std::string) "'" + name + "' has already been used as a structure name");
			return false;
		}

		// Define existing structure
		m_vm.DataTypes().DefineExistingStruc();
    }
	else
	{
		// Create and define new structure
		m_vm.DataTypes().NewStruc(name);
	}
    if (!GetToken ())                                   // Skip structure name
        return false;

    if (!SkipSeparators ())                             // Require : or new line
        return false;

    // Expect at least one field
    if (m_token.m_text == "endstruc" || m_token.m_text == "end") {
        SetError ("Expected DIM or field name");
        return false;
    }

    // Populate with fields
    while (m_token.m_text != "endstruc" && m_token.m_text != "end") {

        // dim statement is optional
/*        if (m_token.m_text != "dim") {
            SetError ("Expected 'dim' or 'endstruc'");
            return false;
        }*/
        if (!CompileDim (true, false))
            return false;

        if (!SkipSeparators ())
            return false;
    }

    if (m_token.m_text == "end") {

        // Skip END
        if (!GetToken ())
            return false;

        // Check END keyword matches declaration keyword
        if (m_token.m_text != keyword) {
            SetError ("Expected '" + keyword + "'");
            return false;
        }

        // Skip STRUC/TYPE
        if (!GetToken ())
            return false;
    }
    else {

        // Make sure "struc" was declared
        if (keyword != "struc") {
            SetError ("Expected 'END '" + keyword + "'");
            return false;
        }

        // Skip ENDSTRUC
        if (!GetToken ())
            return false;
    }

	// Mark structure as defined
	m_vm.DataTypes().CurrentStruc().m_isDefined = true;

    return true;
}

bool TomBasicCompiler::CompileDim(bool forStruc, bool forFuncParam)
{
	vmUserFuncPrototype dummy;
	return CompileDim(forStruc, forFuncParam, dummy);
}

bool TomBasicCompiler::CompileDim (bool forStruc, bool forFuncParam, vmUserFuncPrototype& prototype) {

    // Skip optional DIM
    if (m_token.m_text == "dim")
        if (!GetToken ())
            return false;

    // Expect at least one field in dim
    if (AtSeparatorOrSpecial ()) {
        SetError ("Expected variable declaration");
        return false;
    }

    // Parse fields in dim
    bool needComma = false;             // First element doesn't need a comma
    while (!AtSeparatorOrSpecial () && (!forFuncParam || m_token.m_text != ")")) {

        // Handle commas
        if (needComma) {
            if (m_token.m_text != ",") {
                SetError ("Expected ','");
                return false;
            }
            if (!GetToken ())
                return false;
        }
        needComma = true;               // Remaining elements do need commas

        // Extract field type
        std::string name;
        vmValType type;
        if (!CompileDimField (name, type, forStruc, forFuncParam))
            return false;
        if (!CheckName (name))
            return false;

        if (forStruc) {

            // Validate field name and type
            vmStructure& struc = m_vm.DataTypes ().CurrentStruc ();
            if (m_vm.DataTypes ().FieldStored (struc, name)) {
                SetError ((std::string) "Field '" + name + "' has already been DIMmed in structure '" + struc.m_name + "'");
                return false;
            }

			// Embedded structures must be fully defined (pointers to declared but undefined structures are okay though)
			if (type.m_pointerLevel == 0 && type.m_basicType >= 0) 
			{
				const auto& s = m_vm.DataTypes().Structures()[type.m_basicType];
				if (!s.m_isDefined)
				{
					SetError("Cannot include not yet implemented structure '" + s.m_name + "'");
					return false;
				}
			}

            // Add field to structure
            m_vm.DataTypes ().NewField (name, type);
        }
        else if (forFuncParam) {

            // Find current function

            // Check parameter of the same name has not already been added
			int varIndex = prototype.GetLocalVar(name);
            if (varIndex >= 0) {
                SetError((std::string)"There is already a function parameter called '" + name + "'");
                return false;
            }

			// Check parameter data type has been defined
			if (type.m_pointerLevel == 0 && !CheckTypeIsDefined(type))
				return false;

            // Add parameter type to function definition
			prototype.NewParam(name, type);
        }
        else {

			// Check parameter data type has been defined
			if (!CheckTypeIsDefined(type))
				return false;

			// Regular DIM.
            // Prefix DIMmed variable names
            name = m_symbolPrefix + name;

            if (m_inFunction) {

                // Local variable

                // Check if variable has already been DIMmed locally. (This is
                // allowed, but only if DIMmed to the same type.)
                int varIndex = UserPrototype().GetLocalVar(name);
                if (varIndex >= 0) {
                    if (!(UserPrototype().localVarTypes[varIndex] == type)) {
                        SetError((std::string) "Local variable '" + name + "' has already been allocated as a different type.");
                        return false;
                    }

                    // Variable already created (earlier), so fall through to
                    // allocation code generation
                }
                else
                    // Create new variable
                    varIndex = UserPrototype().NewLocalVar(name, type);

                // Generate code to allocate local variable data
                // Note:    Opcode contains the index of the variable. Variable
                //          type and size data stored in the user function defn.
                AddInstruction(OP_DECLARE_LOCAL, VTP_INT, vmValue(varIndex));

                // Data containing strings will need to be "destroyed" when the stack unwinds.
                if (m_vm.DataTypes().ContainsString(type))
                    AddInstruction(OP_REG_DESTRUCTOR, VTP_INT, vmValue((vmInt)m_vm.StoreType(type)));

                // Optional "= value"?
                if (m_token.m_text == "=") {

                    // Add opcode to load variable address
                    AddInstruction (OP_LOAD_LOCAL_VAR, VTP_INT, vmValue(varIndex));

                    // Set register type
                    m_regType = UserPrototype().localVarTypes[varIndex];
                    m_regType.m_pointerLevel++;

                    // Compile single deref.
                    // Unlike standard variable assignment, we don't automatically
                    // deref pointers here. (Otherwise it would be impossible to
                    // set the pointer within the DIM statement).
                    if (!CompileDeref())
                        return false;

                    // Compile the rest of the assignment
                    if (!InternalCompileAssignment())
                        return false;   
                }
            }
            else {

                // Check if variable has already been DIMmed. (This is allowed, but
                // only if DIMed to the same type.)
                int varIndex = m_vm.Variables ().GetVar (name);
                if (varIndex >= 0) {
                    if (!(m_vm.Variables ().Variables () [varIndex].m_type == type)) {
                        SetError ((std::string) "Variable '" + name + "' has already been allocated as a different type.");
                        return false;
                    }

                    // Variable already created (earlier), so fall through to
                    // allocation code generation.
                }
                else
                    // Create new variable
                    varIndex = m_vm.Variables ().NewVar (name, type);

                // Generate code to allocate variable data
                // Note:    Opcode contains the index of the variable. Variable type
                //          and size data is stored in the variable entry.
                AddInstruction (OP_DECLARE, VTP_INT, vmValue (varIndex));

                // Optional "= value"?
                if (m_token.m_text == "=") {

                    // Add opcode to load variable address
                    AddInstruction (OP_LOAD_VAR, VTP_INT, vmValue(varIndex));

                    // Set register type
                    m_regType = m_vm.Variables().Variables()[varIndex].m_type;
                    m_regType.m_pointerLevel++;

                    // Compile single deref.
                    // Unlike standard variable assignment, we don't automatically
                    // deref pointers here. (Otherwise it would be impossible to
                    // set the pointer within the DIM statement).
                    if (!CompileDeref())
                        return false;

                    // Compile the rest of the assignment
                    if (!InternalCompileAssignment())
                        return false;
                }
            }

            // If this was an array and not a pointer, then its array indices
            // will have been pushed to the stack.
            // The DECLARE operator automatically removes them however
            if (type.PhysicalPointerLevel() == 0)
                for (int i = 0; i < type.m_arrayLevel; i++)
                    m_operandStack.pop_back ();
        }
    }
    return true;
}

bool TomBasicCompiler::CompileTokenName(std::string& name, compTokenType& tokenType, bool allowSuffix) {
    name = "";
    tokenType = m_token.m_type;
    name = m_token.m_text;

    if (tokenType != CTT_TEXT &&
            tokenType != CTT_USER_FUNCTION &&
            tokenType != CTT_RUNTIME_FUNCTION) {
        SetError((std::string)"Expected name");
        return false;
    }

    if (!allowSuffix) {
        char last = name[name.length() - 1];
        if (last == '#' || last == '%' || last == '$' || last == '!') {
            SetError((std::string)"Subroutine names cannot end with: " + last);
            return false;
        }
    }

    if (!GetToken ())
        return false;

    return true;
}

bool TomBasicCompiler::CompileDataType(std::string& name, vmValType& type, compTokenType& tokenType) {

    type = VTP_UNDEFINED;
    name = "";

    // Look for structure type
    if (m_token.m_type == CTT_TEXT) {
        std::string structureName = m_symbolPrefix + m_token.m_text;
        int i = m_vm.DataTypes ().GetStruc (structureName);
        if (i >= 0) {
            type.m_basicType = (vmBasicValType) i;
            if (!GetToken ())           // Skip token type keyword
                return false;
        }
    }

    // Look for preceeding & (indicates pointer)
    if (m_token.m_text == "&") {
        type.m_pointerLevel++;
        if (!GetToken ())
            return false;
    }

    // Look for variable name
    if (!CompileTokenName(name, tokenType, true))
        return false;

/*    if (m_token.m_type != CTT_TEXT) {
        SetError ("Expected variable name");
        return false;
    }

    if (m_token.m_type != CTT_TEXT) {
        SetError ("Expected variable name");
        return false;
    }
    name = m_token.m_text;
    if (!GetToken ())
        return false;*/

    // Determine variable type
    assert (name.length () > 0);
    char last = name [name.length () - 1];
    if (type.m_basicType == VTP_UNDEFINED) {
        if (last == '$')                        type.m_basicType = VTP_STRING;
        else if (last == '#' || last == '!')    type.m_basicType = VTP_REAL;
        else if (last == '%')                   type.m_basicType = VTP_INT;
    }
    else {
        if (last == '$' || last == '#' || last == '%' || last == '!') {
            SetError ((std::string) "\"" + name + "\" is a structure variable, and cannot end with #, $, % or !");
            return false;
        }
    }

    return true;
}

bool TomBasicCompiler::CompileAs(std::string& name, vmValType& type) {
    if (type.m_basicType != VTP_UNDEFINED) {
        SetError ("'" + name + "'s type has already been defined. Cannot use 'as' here.");
        return false;
    }

    // Skip "as"
    if (!GetToken ())
        return false;

    // Expect "single", "double", "integer", "string" or a structure type
    if (m_token.m_type != CTT_TEXT && m_token.m_type != CTT_KEYWORD) {
        SetError ("Expected 'single', 'double', 'integer', 'string' or type name");
        return false;
    }
    if (m_token.m_text == "integer")
        type.m_basicType = VTP_INT;
    else    if (m_token.m_text == "single"
            ||  m_token.m_text == "double")

        // Note: Basic4GL supports only one type of floating point number.
        // We will accept both keywords, but simply allocate a real (= single
        // precision) floating point number each time.
        type.m_basicType = VTP_REAL;
    else if (m_token.m_text == "string")
        type.m_basicType = VTP_STRING;
	else if (m_token.m_text == "function" || m_token.m_text == "sub")
	{
		// Function or sub
		return CompileUserFunction(UFT_POINTER, type);
	}
    else {

        // Look for recognised structure name
        std::string structureName = m_symbolPrefix + m_token.m_text;
        int i = m_vm.DataTypes ().GetStruc (structureName);
        if (i < 0) {
            SetError ("Expected 'single', 'double', 'integer', 'string' or type name");
            return false;
        }
        type.m_basicType = (vmBasicValType) i;
    }

    // Skip type name
    if (!GetToken())
        return false;

    return true;
}

bool TomBasicCompiler::CompileDimField (std::string& name, vmValType& type, bool forStruc, bool forFuncParam) {

    // Compile data type
    compTokenType tokenType;
    if (!CompileDataType(name, type, tokenType))
        return false;

    // Name token must be text
    if (tokenType != CTT_TEXT) {
        SetError ("Expected variable name");
        return false;
    }

/*    type = VTP_UNDEFINED;
    name = "";

    // Look for structure type
    if (m_token.m_type == CTT_TEXT) {
        std::string structureName = m_symbolPrefix + m_token.m_text;
        int i = m_vm.DataTypes ().GetStruc (structureName);
        if (i >= 0) {
            type.m_basicType = (vmBasicValType) i;
            if (!GetToken ())           // Skip token type keyword
                return false;
        }
    }

    // Look for preceeding & (indicates pointer)
    if (m_token.m_text == "&") {
        type.m_pointerLevel++;
        if (!GetToken ())
            return false;
    }

    // Look for variable name
    if (m_token.m_type != CTT_TEXT) {
        SetError ("Expected variable name");
        return false;
    }
    name = m_token.m_text;
    if (!GetToken ())
        return false;

    // Determine variable type
    assert (name.length () > 0);
    char last = name [name.length () - 1];
    if (type.m_basicType == VTP_UNDEFINED) {
        type.m_basicType = VTP_INT;
        if (last == '$')        type.m_basicType = VTP_STRING;
        else if (last == '#')   type.m_basicType = VTP_REAL;
        else if (last == '%')   type.m_basicType = VTP_INT;
    }
    else {
        if (last == '$' || last == '#' || last == '%') {
            SetError ((std::string) "\"" + name + "\" is a structure variable, and cannot end with #, $ or %");
            return false;
        }
    }*/

    // Look for array dimensions
    if (m_token.m_text == "(") {

        bool foundComma = false;
        while (m_token.m_text == "(" || foundComma) {

            // Room for one more dimension?
            if (type.m_arrayLevel >= VM_MAXDIMENSIONS) {
                SetError ((std::string) "Arrays cannot have more than " + IntToString (VM_MAXDIMENSIONS) + " dimensions.");
                return false;
            }
            if (!GetToken ())           // Skip "("
                return false;

            // Validate dimensions.
            // Firstly, pointers don't have dimensions declared with them.
            if (type.m_pointerLevel > 0) {
                if (m_token.m_text != ")") {
                    SetError ("Use '()' to declare a pointer to an array");
                    return false;
                }
                type.m_arrayLevel++;
            }
            // Structure field types must have constant array size that we can
            // evaluate at compile time (i.e right now).
            else if (forStruc) {

                // Evaluate constant expression
                vmBasicValType expressionType = VTP_INT;
                vmValue value;
                std::string stringValue;
                if (!EvaluateConstantExpression (expressionType, value, stringValue))
                    return false;

                // Store array dimension
                type << (value.IntVal() + 1);
            }
            else if (forFuncParam) {
                // Array sizes for function parameters aren't declared.
                // (Syntax is "dim myArray()")
                type.m_arrayLevel++;
            }
            // Regular DIMmed array dimensions are sized at run time.
            // Here we generate code to calculate the dimension size and push it to
            // stack.
            else {
                if (!(CompileExpression () && CompileConvert (VTP_INT) && CompilePush ()))
                    return false;
                type.m_arrayLevel++;
            }

            // Expect closing ')', or a separating comma
            foundComma = false;
            if (m_token.m_text == ")") {
                if (!GetToken ())
                    return false;
            }
            else if (m_token.m_text == ",")
                foundComma = true;
            else {
                SetError ("Expected ')' or ','");
                return false;
            }
        }
    }

    // "as" keyword (QBasic/FreeBasic compatibility)
    if (m_token.m_text == "as") {
        if (!CompileAs(name, type))
            return false;
    }

    // If data type still not specified, default to integer
    if (type.m_basicType == VTP_UNDEFINED)
            type.m_basicType = GetDefaultDataType();

    return true;
}

bool TomBasicCompiler::CompileLoadVar() {

    // Look for "take address"
    bool takeAddress = false;
    if (m_token.m_text == "&") {
        if (!GetToken ())
            return false;
		
		// Special case, take address of function
		if (m_token.m_type == CTT_USER_FUNCTION)
		{
			if (!CompileUserFunctionAddress())
				return false;

			// There's not much point doing data lookups on a function pointer.
			// The only possible "lookup" is to call the pointer, which begets the
			// question: why take the address in the first place.
			// But it's syntactically consistent and there's no reason to disallow it.
			return CompileDataLookup(false);
		}

		takeAddress = true;
	}

    // Look for variable name
    if (m_token.m_type != CTT_TEXT) {
        SetError ("Expected variable name or user function name");
        return false;
    }

    // Prefix variable names
    std::string varName = m_symbolPrefix + m_token.m_text;

    // Find variable
    bool found = false;

    // Check local variable first
    if (m_inFunction) {

        // Look for variable
        int varIndex = UserPrototype().GetLocalVar(varName);

        // Set register type
        if (varIndex >= 0) {

            // Generate code to load variable
            AddInstruction (OP_LOAD_LOCAL_VAR, VTP_INT, vmValue(varIndex));

            // Set register type
            m_regType = UserPrototype().localVarTypes[varIndex];
            m_regType.m_pointerLevel++;

            found = true;
        }
    }

    // Then try global
    if (!found) {

        // Look for variable
        int varIndex = m_vm.Variables().GetVar(varName);

        if (varIndex >= 0)  {

            // Generate code to load variable
            AddInstruction (OP_LOAD_VAR, VTP_INT, vmValue(varIndex));

            // Set register type
            m_regType = m_vm.Variables().Variables()[varIndex].m_type;
            m_regType.m_pointerLevel++;

            found = true;
        }
    }

    if (!found) {
        SetError((std::string) "Unknown variable: " + m_token.m_text + ". Variables must be declared first with DIM");
        return false;
    }

    // Skip past variable name
    if (!GetToken ())
        return false;

    // Dereference to reach data
    if (!CompileDerefs ())
        return false;

    // Compile data lookups (e.g. ".fieldname", array indices, take address e.t.c)
    return CompileDataLookup (takeAddress);
}

bool TomBasicCompiler::CompileDerefs () {

    // Generate code to dereference pointer
    if (!CompileDeref ())
        return false;

    // In Basic4GL syntax, pointers are implicitly dereferenced (similar to C++'s
    // "reference" type.)
    if (m_regType.VirtualPointerLevel () > 0)
        if (!CompileDeref ())
            return false;

    return true;
}

bool TomBasicCompiler::CompileDeref () {

    // Generate code to dereference pointer in reg. (i.e reg = [reg]).
    assert (m_vm.DataTypes ().TypeValid (m_regType));

    // Not a pointer?
    if (m_regType.VirtualPointerLevel () <= 0) {
        assert (false);                                                                 // This should never happen
        SetError ("INTERNAL COMPILER ERROR: Attempted to dereference a non-pointer");
        return false;
    }

    // If reg is pointing to a structure or an array, we don't dereference
    // (as we can't fit an array or structure into a 4 byte register!).
    // Instead we leave it as a pointer, but update the type to indicate
    // to the compiler that we are using a pointer internally to represent
    // a variable.
    assert (!m_regType.m_byRef);
    if (    m_regType.PhysicalPointerLevel () == 1  // Must be pointer to actual data (not pointer to pointer e.t.c)
        && (    m_regType.m_arrayLevel > 0          // Array
            ||  m_regType.m_basicType >= 0)) {      // or structure
        m_regType.m_byRef = true;
        return true;
    }

    /////////////////////////
    // Generate dereference

    // Generate deref instruction
    m_regType.m_pointerLevel--;
    AddInstruction (OP_DEREF, m_regType.StoredType (), vmValue ());  // Load variable

    return true;
} 

bool TomBasicCompiler::CompileDataLookup(bool takeAddress)
{
    // Compile various data operations that can be performed on data object.
    // These operations include:
    //  * Array indexing:           data (index)
    //  * Field lookup:             data.field
    //  * Taking address:           &data
    // Or any combination of the above.

    bool done = false;
    while (!done) {
        if (m_token.m_text == ".") {

            // Lookup subfield
            // Register must contain a structure type
            if (    m_regType.VirtualPointerLevel () != 0
                ||  m_regType.m_arrayLevel != 0
                ||  m_regType.m_basicType < 0) {
                SetError ("Unexpected '.'");
                return false;
            }
            assert (m_vm.DataTypes ().TypeValid (m_regType));

            // Skip "."
            if (!GetToken ())
                return false;

            // Read field name
            if (m_token.m_type != CTT_TEXT) {
                SetError ("Expected field name");
                return false;
            }
            std::string fieldName = m_token.m_text;
            if (!GetToken ())
                return false;

            // Validate field
            vmStructure& s = m_vm.DataTypes ().Structures () [m_regType.m_basicType];
            int fieldIndex = m_vm.DataTypes ().GetField (s, fieldName);
            if (fieldIndex < 0) {
                SetError ((std::string) "'" + fieldName + "' is not a field of structure '" + s.m_name + "'");
                return false;
            }
						
			vmStructureField& field = m_vm.DataTypes().Fields()[fieldIndex];

			// Check field type has been defined
			if (!CheckTypeIsDefined(field.m_type))
				return false;

            // Generate code to calculate address of field
            // Reg is initially pointing to address of structure.
            AddInstruction (OP_ADD_CONST, VTP_INT, vmValue (field.m_dataOffset));
			
            // Reg now contains pointer to field
            m_regType = field.m_type;
            m_regType.m_pointerLevel++;

            // Dereference to reach data
            if (!CompileDerefs ())
                return false;
        }
		else if (m_token.m_text == "(" && 
			m_regType.m_basicType == VTP_FUNC_PTR && 
			m_regType.m_pointerLevel == 0 &&
			m_regType.m_arrayLevel == 0) {

			// Function pointer variable call
			if (!CompileUserFunctionCall(false, false, true))
				return false;
		}
		else if (m_token.m_text == "(") {

			// Array index			
	        // Register must contain an array
	        if (m_regType.VirtualPointerLevel() != 0
		        || m_regType.m_arrayLevel == 0) {
		        SetError("Unexpected '('");
		        return false;
	        }

	        do {
		        if (m_regType.m_arrayLevel == 0) {
			        SetError("Unexpected ','");
			        return false;
		        }

		        // Index into array
		        if (!GetToken())                   // Skip "(" or ","
			        return false;

		        // Generate code to push array address
		        if (!CompilePush())
			        return false;

		        // Evaluate array index, and convert to an integer.
		        if (!CompileExpression())
			        return false;
		        if (!CompileConvert(VTP_INT)) {
			        SetError("Array index must be a number. " + m_vm.DataTypes().DescribeVariable("", m_regType) + " is not a number");
			        return false;
		        }

		        // Generate code to pop array address into reg2
		        if (!CompilePop())
			        return false;

		        // Generate code to index into array.
		        // Input:  reg  = Array index
		        //         reg2 = Array address
		        // Output: reg  = Pointer to array element
		        AddInstruction(OP_ARRAY_INDEX, VTP_INT, vmValue());

		        // reg now points to an element
		        m_regType = m_reg2Type;
		        m_regType.m_byRef = false;
		        m_regType.m_pointerLevel = 1;
		        m_regType.m_arrayLevel--;

		        // Dereference to get to element
		        if (!CompileDerefs())
			        return false;

	        } while (m_token.m_text == ",");

	        // Expect closing bracket
            if (m_token.m_text != ")") {
                SetError ("Expected ')'");
                return false;
            }
            if (!GetToken ())
                return false;
        }
        else
            done = true;
    }

    // Compile take address (if necessary)
    if (takeAddress)
        if (!CompileTakeAddress ())
            return false;

    return true;
}

bool TomBasicCompiler::CompileExpression (bool mustBeConstant) {

    // Compile expression.
    // Generates code that once executed will leave the result of the expression
    // in Reg.

    // Must start with either:
    //      A constant (numeric or string)
    //      A variable reference

    // Push "stop evaluation" operand to stack. (To protect any existing operators
    // on the stack.
    m_operatorStack.push_back (compStackedOperator(compOperator (OT_STOP, OP_NOP, 0, -200000)));    // Stop evaluation operator

    if (!CompileExpressionLoad (mustBeConstant))
        return false;

    compOperatorMap::iterator o;
    while ((m_token.m_text == ")" && OperatorTOS ().m_op.m_type != OT_STOP) || (o = m_binaryOperators.find (m_token.m_text)) != m_binaryOperators.end ()) {

        // Special case, right bracket
        if (m_token.m_text == ")") {

            // Evaluate all operators down to left bracket
            while (OperatorTOS ().m_op.m_type != OT_STOP && OperatorTOS ().m_op.m_type != OT_LBRACKET)
                if (!CompileOperation ())
                    return false;

            // If operator stack is empty, then the expression terminates before
            // the closing bracket
            if (OperatorTOS ().m_op.m_type == OT_STOP) {
                m_operatorStack.pop_back ();            // Remove stopper
                return true;
            }

            // Remove left bracket
            m_operatorStack.pop_back ();

            // Move on
            if (!GetToken ())
                return false;

            // Result may be an array or a structure to which a data lookup can
            // be applied.
            if (!CompileDataLookup (false))
                return false;
        }

        // Otherwise must be regular binary operator
        else {

            // Compare current operator with top of stack operator
            while (OperatorTOS ().m_op.m_type != OT_STOP && OperatorTOS ().m_op.m_binding >= (*o).second.m_binding)
                if (!CompileOperation ())
                    return false;

            // 14-Apr-06: Lazy evaluation.
            // Add jumps around the second part of AND or OR operations
            int lazyJumpAddr = -1;
            if ((*o).second.m_type == OT_LAZYBOOLOPERATOR) {
                if ((*o).second.m_opCode == OP_OP_AND) {
                    lazyJumpAddr = m_vm.InstructionCount();
                    AddInstruction(OP_JUMP_FALSE, VTP_INT, vmValue(0));
                }
                else if ((*o).second.m_opCode == OP_OP_OR) {
                    lazyJumpAddr = m_vm.InstructionCount();
                    AddInstruction(OP_JUMP_TRUE, VTP_INT, vmValue(0));
                }
            }

            // Save operator to stack
            m_operatorStack.push_back (compStackedOperator((*o).second, lazyJumpAddr));

            // Push first operand
            if (!CompilePush ())
                return false;

            // Load second operand
            if (!GetToken ())
                return false;
            if (!CompileExpressionLoad (mustBeConstant))
                return false;
        }
    }

    // Perform remaining operations
    while (OperatorTOS ().m_op.m_type != OT_STOP)
        if (!CompileOperation ())
            return false;

    // Remove stopper
    m_operatorStack.pop_back ();

    return true;
}

bool TomBasicCompiler::CompileOperation () {

    // Compile topmost operation on operator stack
    assert (!m_operatorStack.empty ());

    // Remove operator from stack
    compStackedOperator o = OperatorTOS ();
    m_operatorStack.pop_back ();

    // Must not be a left bracket
    if (o.m_op.m_type == OT_LBRACKET) {
        SetError ("Expected ')'");
        return false;
    }

	// Binary or unary operation?
    if (o.m_op.m_params == 1) {

		// Try overloaded operator functions first
		if (o.m_op.m_type == OT_OPERATOR && CompileOverloadedOperator(o.m_op.m_opCode, FT_OPERATOR_UNARY))
			return true;

        //// Try plug in language extension first
        //if (CompileExtendedUnOperation (o.m_op.m_opCode))
        //    return true;

        // Can only operate on basic types.
        // (This will change once vector and matrix routines have been implemented).
        if (!m_regType.IsBasic ()) {
            SetError ("Operator cannot be applied to this data type");
            return false;
        }

        // Special case, boolean operator.
        // Must convert to boolean first
        if (o.m_op.m_type == OT_BOOLOPERATOR || o.m_op.m_type == OT_LAZYBOOLOPERATOR)
            CompileConvert (VTP_INT);

        // Perform unary operation
        AddInstruction (o.m_op.m_opCode, m_regType.m_basicType, vmValue ());

        // Special case, boolean operator
        // Result will be an integer
        if (o.m_op.m_type == OT_RETURNBOOLOPERATOR)
            m_regType = VTP_INT;
    }
    else if (o.m_op.m_params == 2) {

        // Generate code to pop first operand from stack into Reg2
        if (!CompilePop ())
            return false;

		// Try overloaded operator functions first
		if (o.m_op.m_type == OT_OPERATOR && CompileOverloadedOperator(o.m_op.m_opCode, FT_OPERATOR_BINARY))
			return true;

        //// Try plug in language extension first
        //if (CompileExtendedBinOperation (o.m_op.m_opCode))
        //    return true;

        // Ensure operands are equal type. Generate code to convert one if necessary.

        vmBasicValType opCodeType;                  // Data type to store in OP_CODE
        if (m_regType.IsNull () || m_reg2Type.IsNull ()) {

            // Can compare NULL to any pointer type. However, operator must be '=' or '<>'
            if (o.m_op.m_opCode != OP_OP_EQUAL && o.m_op.m_opCode != OP_OP_NOT_EQUAL) {
                SetError ("Operator cannot be applied to this data type");
                return false;
            }

            // Convert NULL pointer type to non NULL pointer type
            // Note: If both pointers a NULL, CompileConvert will simply do nothing
            if (m_regType.IsNull ())
                if (!CompileConvert (m_reg2Type))
                    return false;

            if (m_reg2Type.IsNull ())
                if (!CompileConvert2 (m_regType))
                    return false;

            opCodeType = VTP_INT;               // Integer comparison is used internally
        }
		else if (m_regType.IsFuncPtr() && m_reg2Type.IsFuncPtr())
		{
			// Can compare function pointers to other function pointers with compatible prototypes.
			// Operator must be '=' or '<>'.
			if (o.m_op.m_opCode != OP_OP_EQUAL && o.m_op.m_opCode != OP_OP_NOT_EQUAL) {
				SetError("Operator cannot be applied to this data type");
				return false;
			}

			// Make sure prototypes are compatible
			const auto& p1 = m_vm.UserFunctionPrototypes()[m_regType.m_prototypeIndex];
			const auto& p2 = m_vm.UserFunctionPrototypes()[m_reg2Type.m_prototypeIndex];
			if (!p1.IsCompatibleWith(p2))
			{
				SetError("Function pointer types are not compatible");
				return false;
			}

			opCodeType = VTP_INT;				// Integer comparison used internally
		}
        else if (m_regType.VirtualPointerLevel () > 0 || m_reg2Type.VirtualPointerLevel () > 0) {

            // Can compare 2 pointers. However operator must be '=' or '<>' and
            // pointer types must be exactly the same
            if (o.m_op.m_opCode != OP_OP_EQUAL && o.m_op.m_opCode != OP_OP_NOT_EQUAL) {
                SetError ("Operator cannot be applied to this data type");
                return false;
            }
            if (!m_regType.ExactEquals (m_reg2Type)) {
                SetError ("Cannot compare pointers to different types");
                return false;
            }

            opCodeType = VTP_INT;               // Integer comparison is used internally
        }
        else {

            // Otherwise all operators can be applied to basic data types
            if (!m_regType.IsBasic () || !m_reg2Type.IsBasic ()) {
                SetError ("Operator cannot be applied to this data type");
                return false;
            }

            // Convert operands to highest type
            vmBasicValType highest = m_regType.m_basicType;
            if (m_reg2Type.m_basicType > highest)
                highest = m_reg2Type.m_basicType;
            if (o.m_op.m_type == OT_BOOLOPERATOR || o.m_op.m_type == OT_LAZYBOOLOPERATOR)
                highest = VTP_INT;
            if ((m_syntax == LS_TRADITIONAL || m_syntax == LS_TRADITIONAL_SUFFIX) && o.m_op.m_opCode == OP_OP_DIV)     // 14-Aug-05 Tom: In traditional mode, division is always between floating pt numbers
                highest = VTP_REAL;

            if (!CompileConvert(highest))
                return false;
            if (!CompileConvert2(highest))
                return false;
            opCodeType = highest;
        }

        // Generate operation code
        AddInstruction (o.m_op.m_opCode, opCodeType, vmValue ());

        // Special case, boolean operator
        // Result will be an integer
        if (o.m_op.m_type == OT_RETURNBOOLOPERATOR)
            m_regType = VTP_INT;
    }
    else
        assert (false);

    // Fix up lazy jumps
    if (o.m_lazyJumpAddr >= 0)
        m_vm.Instruction(o.m_lazyJumpAddr).m_value = (int) m_vm.InstructionCount();

    return true;
}

bool TomBasicCompiler::CompileOverloadedOperator(vmOpCode opCode, compFunctionType type)
{
	// Look for operator overload
	std::string overloadFn = m_operatorOverloads[opCode];
	if (overloadFn == "") return false;
	
	// Save state before calling CompileFunctionCall (in case call fails)
	auto count = m_vm.InstructionCount();
	vmValType saveReg1Type = m_regType;
	vmValType saveReg2Type = m_reg2Type;
	int numOperators = m_operatorStack.size();
	int numOperands = m_operandStack.size();

	if (CompileFunctionCall(overloadFn, type, true))
		return true;

	// Restore state 
	ClearError();
	m_vm.RollbackProgram(count);
	m_regType = saveReg1Type;
	m_reg2Type = saveReg2Type;
	while (m_operatorStack.size() > numOperators) m_operatorStack.pop_back();
	while (m_operandStack.size() > numOperands) m_operandStack.pop_back();
	return false;
}

bool TomBasicCompiler::CompileExpressionLoad (bool mustBeConstant) {

    // Like CompileLoad, but will also accept and stack preceeding unary operators

    // Push any unary operators found
    while (true) {

        // Special case, left bracket
        if (m_token.m_text == "(")
            m_operatorStack.push_back (compOperator (OT_LBRACKET, OP_NOP, 0, -10000));         // Brackets bind looser than anything else

        // Otherwise look for recognised unary operator
        else {
            compOperatorMap::iterator o = m_unaryOperators.find (m_token.m_text);
            if (o != m_unaryOperators.end ())                   // Operator found
                m_operatorStack.push_back((*o).second);         // => Stack it
            else {                                              // Not an operator
                if (mustBeConstant)
                    return CompileLoadConst ();
                else
                    return CompileLoad ();                      // => Proceed on to load variable/constant
            }
        }

        if (!GetToken ())
            return false;
    }
}

bool TomBasicCompiler::CompileLoad () {

    // Compile load var or constant, or function result
    if (m_token.m_type == CTT_CONSTANT || m_token.m_text == "null")
        return CompileLoadConst ();
    else if (m_token.m_type == CTT_TEXT || m_token.m_text == "&")
        return CompileLoadVar ();
    else if (m_token.m_type == CTT_FUNCTION)
        return CompileFunction (true);
    else if (m_token.m_type == CTT_USER_FUNCTION)
        return CompileUserFunctionCall(true, false, false);
    else if (m_token.m_type == CTT_RUNTIME_FUNCTION)
        return CompileUserFunctionCall(true, true, false);
	else if (m_token.m_type == CTT_KEYWORD && m_token.m_text == "arraymax")
		return CompileArrayMax();

    SetError ("Expected constant, variable or function");
    return false;
}

bool TomBasicCompiler::CompileNull() {
    AddInstruction (OP_LOAD_CONST, VTP_INT, vmValue (0));       // Load 0 into register
    m_regType = vmValType (VTP_NULL, 0, 1, false);              // Type is pointer to VTP_NULL
    return GetToken ();
}

bool TomBasicCompiler::CompileLoadConst () {

    // Special case, "NULL" reserved word
    if (m_token.m_text == "null")
        return CompileNull();

    // Compile load constant
    if (m_token.m_type == CTT_CONSTANT) {

        // Special case, string constants
        if (m_token.m_valType == VTP_STRING) {

            // Allocate new string constant
            std::string text = m_token.m_text.substr (1, m_token.m_text.length () - 1);     // Remove S prefix
            int index = m_vm.StoreStringConstant (text);

            // store load instruction
            AddInstruction (OP_LOAD_CONST, VTP_STRING, vmValue (index));
            m_regType = VTP_STRING;
        }
        else if (m_token.m_valType == VTP_REAL) {
            AddInstruction (OP_LOAD_CONST, VTP_REAL, vmValue (StringToReal (m_token.m_text)));
            m_regType = VTP_REAL;
        }
        else if (m_token.m_valType == VTP_INT) {
            AddInstruction (OP_LOAD_CONST, VTP_INT, vmValue(StringToInt(m_token.m_text)));
            m_regType = VTP_INT;
        }
        else {
            SetError ("Unknown data type");
            return false;
        }

        return GetToken ();
    }

    SetError ("Expected constant");
    return false;
}

bool TomBasicCompiler::CompilePush () {

    // Store pushed value type
    m_operandStack.push_back (m_regType);

    // Generate push code
    AddInstruction (OP_PUSH, m_regType.StoredType (), vmValue ());

    return true;
}

bool TomBasicCompiler::CompilePop () {

    if (m_operandStack.empty ()) {
        SetError ("Expression error");
        return false;
    }

    // Retrieve pushed value type
    m_reg2Type = m_operandStack [m_operandStack.size () - 1];
    m_operandStack.pop_back ();

    // Generate pop code
    AddInstruction (OP_POP, m_reg2Type.StoredType (), vmValue ());

    return true;
}

bool TomBasicCompiler::CompileConvert (vmBasicValType type) {

    // Convert reg to given type
    if (m_regType == type)              // Already same type
        return true;

	// Can convert function pointer, untyped function pointer or null to a function pointer.
	// Note: Calling code is responsible for ensuring functions have compatible prototypes
	// (and emitting the appropriate runtime check if necessary)
	if (type == VTP_FUNC_PTR && 
		(	m_regType.PhysicalPointerLevel() == 0 && m_regType.m_basicType == VTP_FUNC_PTR 
		||  m_regType.PhysicalPointerLevel() == 0 && m_regType.m_basicType == VTP_UNTYPED_FUNC_PTR
		||	m_regType.PhysicalPointerLevel() == 1 && m_regType.IsNull()))
	{
		return true;
	}

    // Determine opcode
    vmOpCode code = OP_NOP;
    if (m_regType == VTP_INT) {
        if (type == VTP_REAL)           code = OP_CONV_INT_REAL;
        else if (type == VTP_STRING)    code = OP_CONV_INT_STRING;
    }
    else if (m_regType == VTP_REAL) {
        if (type == VTP_INT)            code = OP_CONV_REAL_INT;
        else if (type == VTP_STRING)    code = OP_CONV_REAL_STRING;
    }

    // Store instruction
    if (code != OP_NOP) {
        AddInstruction (code, VTP_INT, vmValue ());
        m_regType = type;
        return true;
    }

    SetError ("Incorrect data type");
    return false;
}

bool TomBasicCompiler::CompileConvert2 (vmBasicValType type) {

    // Convert reg2 to given type
    if (m_reg2Type == type)             // Already same type
        return true;

    // Determine opcode
    vmOpCode code = OP_NOP;
    if (m_reg2Type == VTP_INT) {
        if (type == VTP_REAL)           code = OP_CONV_INT_REAL2;
        else if (type == VTP_STRING)    code = OP_CONV_INT_STRING2;
    }
    else if (m_reg2Type == VTP_REAL) {
        if (type == VTP_INT)            code = OP_CONV_REAL_INT2;
        else if (type == VTP_STRING)    code = OP_CONV_REAL_STRING2;
    }

    // Store instruction
    if (code != OP_NOP) {
        AddInstruction (code, VTP_INT, vmValue ());
        m_reg2Type = type;
        return true;
    }

    SetError ("Incorrect data type");
    return false;
}

bool TomBasicCompiler::CompileTakeAddress () {

    // Take address of data in reg.
    // We do this my moving the previously generate deref from the end of the program.
    // (If the last instruction is not a deref, then there is a problem.)

    // Special case: Implicit pointer
    if (m_regType.m_byRef) {
        m_regType.m_byRef = false;              // Convert to explicit pointer
        return true;
    }

    // Check last instruction was a deref
    if (m_vm.InstructionCount () <= 0 || m_vm.Instruction (m_vm.InstructionCount () - 1).m_opCode != OP_DEREF) {
        SetError ("Cannot take address of this data");
        return false;
    }

    // Remove it
    m_vm.RemoveLastInstruction ();
    m_regType.m_pointerLevel++;

    return true;
}

bool TomBasicCompiler::CompileAssignment () {

    // Generate code to load target variable
    if (!CompileLoadVar ())
        return false;

	// Is this an assignment?
	if (m_token.m_text == "=") {

		// Compile code to assign value to variable
		if (!InternalCompileAssignment())
			return false;
	}

    return true;
}

bool TomBasicCompiler::InternalCompileAssignment() {

    // Expect =
    if (m_token.m_text != "=") {
        SetError ("Expected '='");
        return false;
    }

    // Convert load target variable into take address of target variable
    if (!CompileTakeAddress ()) {
        SetError ("Left side cannot be assigned to");
        return false;
    }

    // Skip =
    if (!GetToken ())
        return false;

    // Push target address
    if (!CompilePush ())
        return false;

    // Generate code to evaluate expression
    if (!CompileExpression ())
        return false;

    // Pop target address into reg2
    if (!CompilePop ())
        return false;

    // Simple type case: reg2 points to basic type
    if (    m_reg2Type.m_pointerLevel == 1
        &&  m_reg2Type.m_arrayLevel   == 0
        &&  m_reg2Type.m_basicType    < 0) {

        // Attempt to convert value in reg to same type
		vmValType prevRegType = m_regType;
		if (!CompileConvert(m_reg2Type.m_basicType)) {
            SetError ("Types do not match");
            return false;
        }

		// Extra checking required for function pointers. 
		if (m_reg2Type.m_basicType == VTP_FUNC_PTR) 
		{
			// Function pointer assignment requires function prototypes be compatible
			if (m_regType.m_basicType == VTP_FUNC_PTR)
			{
				const auto& p1 = m_vm.UserFunctionPrototypes()[m_reg2Type.m_prototypeIndex];
				const auto& p2 = m_vm.UserFunctionPrototypes()[prevRegType.m_prototypeIndex];
				if (!p1.IsCompatibleWith(p2))
				{
					SetError("Types do not match");
					return false;
				}
			}
			
			// Assigning an untyped function pointer requires a run-time type check
			if (m_regType.m_basicType == VTP_UNTYPED_FUNC_PTR)
			{
				AddInstruction(OP_CHECK_FUNC_PTR, VTP_INT, m_reg2Type.m_prototypeIndex);
			}
		}

        // Save reg into [reg2]
        AddInstruction (OP_SAVE, m_reg2Type.m_basicType, vmValue ());
    }

    // Pointer case. m_reg2 must point to a pointer and m_reg1 point to a value.
    else if (m_reg2Type.VirtualPointerLevel () == 2 && m_regType.VirtualPointerLevel () == 1) {

        // Must both point to same type, OR m_reg1 must point to NULL
        if (    m_regType.IsNull ()
            || (    m_regType.m_arrayLevel == m_reg2Type.m_arrayLevel
                &&  m_regType.m_basicType  == m_reg2Type.m_basicType)) {

            // Validate pointer scope before saving to variable
            AddInstruction(OP_CHECK_PTR, VTP_INT, vmValue());

            // Save address to pointer
            AddInstruction(OP_SAVE, VTP_INT, vmValue());
        }
        else {
            SetError ("Types do not match");
            return false;
        }
    }

    // Copy object case
    else if (   m_reg2Type.VirtualPointerLevel () == 1
            &&  m_regType.VirtualPointerLevel () == 0
            &&  m_regType.PhysicalPointerLevel () == 1) {

        // Check that both are the same type
        if (    m_regType.m_arrayLevel == m_reg2Type.m_arrayLevel
            &&  m_regType.m_basicType  == m_reg2Type.m_basicType) {

            // Add op-code to check pointers if necessary
            vmValType dataType = m_regType;
            dataType.m_pointerLevel--;
            dataType.m_byRef = false;
            if (m_vm.DataTypes().ContainsPointer(dataType))
                AddInstruction(OP_CHECK_PTRS, VTP_INT, vmValue((vmInt) m_vm.StoreType(dataType)));

            AddInstruction (OP_COPY, VTP_INT, vmValue ((vmInt) m_vm.StoreType (m_regType)));
        }
        else {
            SetError ("Types do not match");
            return false;
        }
    }
    else {
        SetError ("Types do not match");
        return false;
    }

    return true;
}

bool TomBasicCompiler::CompileGoto (vmOpCode jumpType) {
    assert (    jumpType == OP_JUMP
            ||  jumpType == OP_JUMP_TRUE
            ||  jumpType == OP_JUMP_FALSE
            ||  jumpType == OP_CALL);

    // Cannot use goto inside a function or sub (can use GOSUB though)
    if (m_inFunction && jumpType != OP_CALL) {
        SetError("Cannot use 'goto' inside a function or subroutine");
        return false;
    }

    // Validate label
    if (m_token.m_type != CTT_TEXT) {
        SetError ("Expected label name");
        return false;
    }

    // Record jump, so that we can fix up the offset in the second compile pass.
    std::string labelName = m_symbolPrefix + m_token.m_text;
    m_jumps.push_back (compJump (m_vm.InstructionCount (), labelName));

    // Add jump instruction
    AddInstruction (jumpType, VTP_INT, vmValue (0));

    // Move on
    return GetToken ();
}

bool TomBasicCompiler::CompileIf (bool elseif) {

    // Skip "if"
    int line = m_parser.Line (), col = m_parser.Col ();
    if (!GetToken ())
        return false;

    // Generate code to evaluate expression
    if (!CompileExpression ())
        return false;

    // Generate code to convert to integer
    if (!CompileConvert (VTP_INT))
        return false;

    // Free any temporary data expression may have created
    if (!CompileFreeTempData ())
        return false;

    // Special case!
    // If next instruction is a "goto", then we can ommit the "then"
    if (m_token.m_text != "goto") {

        // Otherwise expect "then"
        if (m_token.m_text != "then") {
            SetError ("Expected 'then'");
            return false;
        }
        if (!GetToken ())
            return false;
    }

    // Determine whether this "if" has an automatic "endif" inserted at the end of the line
    bool autoEndif = (m_syntax == LS_TRADITIONAL || m_syntax == LS_TRADITIONAL_SUFFIX)  // Only applies to traditional syntax
        && !(m_token.m_type == CTT_EOL || m_token.m_type == CTT_EOF);                   // "then" must not be the last token on the line

    // Create flow control structure
    m_flowControl.push_back (compFlowControl (FCT_IF, m_vm.InstructionCount (), 0, line, col, elseif, "", !autoEndif));

    // Create conditional jump
    AddInstruction (OP_JUMP_FALSE, VTP_INT, vmValue (0));

    m_needColon = false;                // Don't need colon between this and next instruction
    return true;
}

bool TomBasicCompiler::CompileElse (bool elseif) {

    // Find "if" on top of flow control stack
    if (!FlowControlTopIs (FCT_IF)) {
        SetError ("'else' without 'if'");
        return false;
    }
    compFlowControl top = FlowControlTOS ();
    m_flowControl.pop_back ();

    // Skip "else"
    // (But not if it's really an "elseif". CompileIf will skip over it then.)
    int line = m_parser.Line (), col = m_parser.Col ();
    if (!elseif) {
        if (!GetToken ())
            return false;
    }

    // Push else to flow control stack
    m_flowControl.push_back (compFlowControl (FCT_ELSE, m_vm.InstructionCount (), 0, line, col, top.m_impliedEndif, "", top.m_blockIf));

    // Generate code to jump around else block
    AddInstruction (OP_JUMP, VTP_INT, vmValue (0));

    // Fixup jump around IF block
    assert ((unsigned)top.m_jumpOut < m_vm.InstructionCount ());
    m_vm.Instruction (top.m_jumpOut).m_value.IntVal () = m_vm.InstructionCount ();

    m_needColon = false;                // Don't need colon between this and next instruction
    return true;
}

bool TomBasicCompiler::CompileEndIf (bool automatic) {

    // Find if or else on top of flow control stack
    if (!(FlowControlTopIs (FCT_IF) || FlowControlTopIs (FCT_ELSE))) {
        SetError ("'endif' without 'if'");
        return false;
    }
    compFlowControl top = FlowControlTOS ();
    m_flowControl.pop_back ();

    // Skip "endif"
    if (!top.m_impliedEndif && !automatic) {
        if (!GetToken ())
            return false;
    }

    // Fixup jump around IF or ELSE block
    assert ((unsigned)top.m_jumpOut < m_vm.InstructionCount ());
    m_vm.Instruction (top.m_jumpOut).m_value.IntVal () = m_vm.InstructionCount ();

    // If there's an implied endif then add it
    if (top.m_impliedEndif)
        return CompileEndIf (automatic);
    else
        return true;
}

bool TomBasicCompiler::CompileFor () {

    // Skip "for"
    int line = m_parser.Line (), col = m_parser.Col ();
    if (!GetToken ())
        return false;

    // Extract loop variable name
    compToken nextToken = m_parser.PeekToken (false);
    if (!CheckParser ())
        return false;
    if (nextToken.m_text == "(") {
        SetError ("Cannot use array variable in 'for' - 'next' structure");
        return false;
    }
    std::string loopVarUnprefixed = m_token.m_text;
    std::string loopVar = m_symbolPrefix + loopVarUnprefixed;

    // Verify variable is numeric
    bool found = false;
    vmBasicValType loopVarType;

    // Check local variable first
    if (m_inFunction) {

        // Look for variable
        int varIndex = UserPrototype().GetLocalVar(loopVar);

        // Set register type
        if (varIndex >= 0) {
            found = true;

            // Check type is INT or REAL
            vmValType& type = UserPrototype().localVarTypes[varIndex];
            if (!(type == VTP_INT || type == VTP_REAL)) {
                SetError ((std::string) "Loop variable must be an Integer or Real");
                return false;
            }
            loopVarType = type.m_basicType;
        }
    }

    // Check global variable
    if (!found) {
        int varIndex = m_vm.Variables().GetVar(loopVar);
        if (varIndex >= 0) {
            found = true;

            // Check type is INT or REAL
            vmValType& type = m_vm.Variables().Variables()[varIndex].m_type;
            if (!(type == VTP_INT || type == VTP_REAL)) {
                SetError ((std::string) "Loop variable must be an Integer or Real");
                return false;
            }
            loopVarType = type.m_basicType;
        }
    }
    if (!found) {
        SetError ((std::string) "Unknown variable: " + m_token.m_text + ". Must be declared with DIM");
        return false;
    }

    // Compile assignment
    int varLine = m_parser.Line (), varCol = m_parser.Col ();
    compToken varToken = m_token;
    if (!CompileAssignment ())
        return false;

    // Save loop back position
    int loopPos = m_vm.InstructionCount ();

    // Expect "to"
    if (m_token.m_text != "to") {
        SetError ("Expected 'to'");
        return false;
    }
    if (!GetToken ())
        return false;

    // Compile load variable and push
    compParserPos savedPos = SavePos ();                            // Save parser position
    m_parser.SetPos (varLine, varCol);                              // Point to variable name
    m_token = varToken;

    if (!CompileLoadVar ())                                         // Load variable
        return false;
    if (!CompilePush ())                                            // And push
        return false;

    RestorePos (savedPos);                                          // Restore parser position

    // Compile "to" expression
    if (!CompileExpression ())
        return false;
    if (!CompileConvert (loopVarType))
        return false;

    // Evaluate step. (Must be a constant expression)
    vmBasicValType stepType = loopVarType;
    vmValue stepValue;

    if (m_token.m_text == "step") {

        // Skip step instruction
        if (!GetToken ())
            return false;

        // Compile step constant (expression)
        std::string stringValue;
        if (!EvaluateConstantExpression(stepType, stepValue, stringValue))
            return false;
    }
    else {

        // No explicit step.
        // Use 1 as default
        if (stepType == VTP_INT)
            stepValue = vmValue (1);
        else
            stepValue = vmValue ((vmReal) 1.0f);
    }

    // Choose comparison operator (based on direction of step)
    compOperator comparison;
    if (stepType == VTP_INT) {
             if (stepValue.IntVal () > 0)   comparison = m_binaryOperators ["<="];
        else if (stepValue.IntVal () < 0)   comparison = m_binaryOperators [">="];
        else                                comparison = m_binaryOperators ["<>"];
    }
    else {
        assert (stepType == VTP_REAL);
             if (stepValue.RealVal () > 0)  comparison = m_binaryOperators ["<="];
        else if (stepValue.RealVal () < 0)  comparison = m_binaryOperators [">="];
        else                                comparison = m_binaryOperators ["<>"];
    }

    // Compile comparison expression
    m_operatorStack.push_back (comparison);
    if (!CompileOperation ())
        return false;

    // Generate step expression
    std::string step = loopVarUnprefixed + " = " + loopVarUnprefixed + " + " + (stepType == VTP_INT ? IntToString (stepValue.IntVal ()) : RealToString (stepValue.RealVal ()));

    // Create flow control structure
    m_flowControl.push_back (compFlowControl (FCT_FOR, m_vm.InstructionCount (), loopPos, line, col, false, step));

    // Create conditional jump
    AddInstruction (OP_JUMP_FALSE, VTP_INT, vmValue (0));

    return true;
}

bool TomBasicCompiler::CompileNext () {

    // Find for on top of flow control stack
    if (!FlowControlTopIs (FCT_FOR)) {
        SetError ("'next' without 'for'");
        return false;
    }
    compFlowControl top = FlowControlTOS ();
    m_flowControl.pop_back ();

    // Skip "next"
    int nextLine = m_token.m_line, nextCol = m_token.m_col;
    if (!GetToken ())
        return false;

    // Generate instruction to increment loop variable
    m_parser.SetSpecial (top.m_data, nextLine, nextCol);    // Special mode. Compile this string instead of source code.
                                                            // We pass in the line and the column of the "next" instruction so that generated code will be
                                                            // associated with this offset in the source code. (This keeps the debugger happy) 
    compToken saveToken = m_token;
    if (!GetToken ())
        return false;
    if (!CompileAssignment ())
        return false;
    m_parser.SetNormal ();
    m_token = saveToken;

    // Generate jump back instruction
    AddInstruction (OP_JUMP, VTP_INT, vmValue (top.m_jumpLoop));

    // Fixup jump around FOR block
    assert ((unsigned)top.m_jumpOut < m_vm.InstructionCount ());
    m_vm.Instruction (top.m_jumpOut).m_value.IntVal () = m_vm.InstructionCount ();
    return true;
}

bool TomBasicCompiler::CompileWhile () {

    // Save loop position
    int loopPos = m_vm.InstructionCount ();

    // Skip "while"
    int line = m_parser.Line (), col = m_parser.Col ();
    if (!GetToken ())
        return false;

    // Generate code to evaluate expression
    if (!CompileExpression ())
        return false;

    // Generate code to convert to integer
    if (!CompileConvert (VTP_INT))
        return false;

    // Free any temporary data expression may have created
    if (!CompileFreeTempData ())
        return false;

    // Create flow control structure
    m_flowControl.push_back (compFlowControl (FCT_WHILE, m_vm.InstructionCount (), loopPos, line, col));

    // Create conditional jump
    AddInstruction (OP_JUMP_FALSE, VTP_INT, vmValue (0));
    return true;
}

bool TomBasicCompiler::CompileWend () {

    // Find while on top of flow control stack
    if (!FlowControlTopIs (FCT_WHILE)) {
        SetError ("'wend' without 'while'");
        return false;
    }
    compFlowControl top = FlowControlTOS ();
    m_flowControl.pop_back ();

    // Skip "wend"
    if (!GetToken ())
        return false;

    // Generate jump back
    AddInstruction (OP_JUMP, VTP_INT, vmValue (top.m_jumpLoop));

    // Fixup jump around WHILE block
    assert ((unsigned)top.m_jumpOut < m_vm.InstructionCount ());
    m_vm.Instruction (top.m_jumpOut).m_value.IntVal () = m_vm.InstructionCount ();
    return true;
}

bool TomBasicCompiler::CheckName (std::string name) {

    // Check that name is a suitable variable, structure or structure field name.
    if (m_constants.find (name) != m_constants.end ()
    ||  m_programConstants.find (name) != m_programConstants.end ()) {
        SetError ("'" + name + "' is a constant, and cannot be used here");
        return false;
    }
    if (m_reservedWords.find (name) != m_reservedWords.end ()) {
        SetError ("'" + name + "' is a reserved word, and cannot be used here");
        return false;
    }
    return true;
}

void TomBasicCompiler::AddFunction (    std::string         name,
                                        vmFunction          func,
                                        compParamTypeList   params,
                                        bool                isFunction,
                                        bool                brackets,
                                        vmValType           returnType,
                                        bool                timeshare,
                                        bool                freeTempData,
                                        compParamValidationCallback paramValidationCallback,
										bool				conditionalTimeshare) {

    // Register wrapper function to virtual machine
    int vmIndex = m_vm.AddFunction (func);

    // Register function spec to compiler
    int specIndex = m_functions.size ();
    m_functions.push_back (
            compFuncSpec (
                params,
                isFunction,
                brackets,
                returnType,
                timeshare,
				conditionalTimeshare,
                vmIndex,
                freeTempData,
                paramValidationCallback));

    // Add function name -> function spec mapping
    m_functionIndex.insert (std::make_pair(LowerCase (name), specIndex));
}

bool TomBasicCompiler::CompileFunction(bool needResult) {

	// Get function name
	std::string name = m_token.m_text;
	if (!GetToken())
		return false;

	return CompileFunctionCall(name, FT_NORMAL, needResult);
}

template<typename T> void Filter(T* items, int& count, std::function<bool(T const&)> predicate)
{
	// Filter out all items that don't match the predicate
	int src, dst;
	dst = 0;
	for (src = 0; src < count; src++)
		if (predicate(items[src]))
			items[dst++] = items[src];
	count = dst;
}

template<typename T> bool HasAny(T const* items, int count, std::function<bool(T const&)> predicate)
{
	for (int i = 0; i < count; i++)
		if (predicate(items[i]))
			return true;
	return false;
}

bool TomBasicCompiler::CompileFunctionCall(std::string name, compFunctionType type, bool needResult)
{
    // Find function specifications.
    // (Note: There may be more than one with the same name.
    // We collect the possible candidates in an array, and prune out the ones
    // whose paramater types are incompatible as we go..)
    compExtFuncSpec functions [TC_MAXOVERLOADEDFUNCTIONS];
    int functionCount = 0;

    // Find builtin functions
    compFuncIndex::iterator it;
    for (   it = m_functionIndex.find (name);
            it != m_functionIndex.end () && (*it).first == name && functionCount < TC_MAXOVERLOADEDFUNCTIONS;
            it++) {
        compFuncSpec& spec = m_functions [(*it).second];                        // Get specification

        // Check whether function returns a value (if we need one)
        if (!needResult || spec.m_isFunction) {
            functions[functionCount].m_spec = &spec;
            functions[functionCount].m_builtin = true;
            functionCount++;
        }
    }

    // Find plugin DLL functions
    m_plugins.FindFunctions(name, functions, functionCount, TC_MAXOVERLOADEDFUNCTIONS);

	// No functions?
	if (functionCount == 0)
	{
		SetError(name + " is not a recognised function name");
		return false;
	}

	// Filter out non functions if a result is needed.
	if (needResult)
	{
		Filter<compExtFuncSpec>(functions, functionCount, [=](compExtFuncSpec const& fn)
		{
			return fn.m_spec->m_isFunction;
		});
		if (functionCount == 0)
		{
			SetError(name + " does not return a value");
			return false;
		}
	}
	
	// Brackets are required if any remaining overload requires brackets.
	// (We need to decide up front whether a '(' is the function brackets or part of the first parameter expression.
	// It gets too complicated if we try to handle some overloads that require brackets and some that don't.)
	bool brackets = HasAny<compExtFuncSpec>(functions, functionCount, [this](compExtFuncSpec const& fn)
	{
		// Explicit no-brackets function
		if (!fn.m_spec->m_brackets) 
			return false;

		// Traditional syntax: Don't require brackets for subs or no-param functions
		if ((m_syntax == LS_TRADITIONAL || m_syntax == LS_TRADITIONAL_SUFFIX) && (!fn.m_spec->m_isFunction || fn.m_spec->m_paramTypes.Params().size() == 0))
			return false;

		return true;
	});

	bool isNoParamFunction = functionCount == 1 && functions[0].m_spec->m_isFunction && functions[0].m_spec->m_paramTypes.Params().size() == 0;

    // Expect opening bracket
    if (type == FT_NORMAL && brackets) {
        if (m_token.m_text != "(") {
            SetError ("Expected '('");
            return false;
        }
        // Skip it
        if (!GetToken ())
            return false;
    }

    // Generate code to push parameters
    bool first = true;
    unsigned int count = 0;
    unsigned int pushCount = 0;      // Usually pushCount = count (the parameter count). However "any type" parameters also have their data type pushed with them, in which case pushCount > count.
    while (functionCount > 0 && 
		(	(type == FT_NORMAL && m_token.m_text != ")" && !AtSeparatorOrSpecial () && !isNoParamFunction)		// Exit criteria for normal functions
		 || (type == FT_OPERATOR_UNARY && count < 1)															// Exit criteria for unary functions
		 || (type == FT_OPERATOR_BINARY && count < 2))) {														// Exit criteria for binary functions

        // Trim functions with fewer parameters than we have found
        int src, dst;
        dst = 0;
        for (src = 0; src < functionCount; src++)
            if (functions[src].m_spec->m_paramTypes.Params().size() > count)
                functions[dst++] = functions [src];
        functionCount = dst;

        // None left?
        if (functionCount == 0) {
			if (type == FT_NORMAL) {
				if (brackets)   SetError("Expected ')'");
				else            SetError("Expected ':' or end of line");
			}
            return false;
        }

        if (type == FT_NORMAL && !first) {
            // Expect comma
            if (m_token.m_text != ",") {
                SetError ("Expected ','");
                return false;
            }
            // Skip it
            if (!GetToken ())
                return false;
        }
        first = false;

        // Generate code to evaluate parameter
		switch (type)
		{
		case FT_NORMAL:
			if (!CompileExpression())
				return false;
			break;
		case FT_OPERATOR_UNARY:
			// Register already holds value
			break;
		case FT_OPERATOR_BINARY:
			// Initially reg contains second parameter and reg2 contains first.
			// Swap them each iteration to get the correct parameter into reg.
			if (!CompileSwap())
				return false;
			break;
		}

        // Find first valid function which matches at this parameter
        int matchIndex = -1;
        bool isAnyType = false;
        int i;
        for (i = 0; i < functionCount && matchIndex < 0; i++) {
            vmValType& type = functions[i].m_spec->m_paramTypes.Params()[count];

            // Check for undefined type parameter
            if (type == VTP_UNDEFINED) {

                // Function definition indicates whether parameter type is valid
                // via a compiler time callback function.
                if (functions[i].m_spec->m_paramValidationCallback(count, m_regType)) {

                    // Found "any type" match
                    matchIndex = i;
                    isAnyType = true;
                }
                else
                    SetError("Incorrect data type");
            }
            else {

                // Specific type requested.
                // Check parameter can be converted to that type
                if (CompileConvert(type)) {

                    // Found specific type match
                    matchIndex = i;
                    isAnyType = false;
                }
            }
        }

//            if (CompileConvert (functions[i].m_spec->m_paramTypes.Params () [count]))     // Can convert register to parameter type
//                matchIndex = i;                                                     // Matching function found

        if (matchIndex >= 0) {

            // Clear any errors that non-matching instances might have set.
            ClearError ();

            vmValType& type = functions[matchIndex].m_spec->m_paramTypes.Params () [count];

            // Filter out all functions whose "count" parameter doesn't match "type".
			Filter<compExtFuncSpec>(functions, functionCount, [=](compExtFuncSpec const& fn)
			{
				return isAnyType
					// If the first function to match accepts an "any type"
					// parameter, then all other overloads must be an "any type"
					// parameter.
					? fn.m_spec->m_paramValidationCallback(count, m_regType)

					// Likewise if the first function to match requires a specific
					// parameter type, then all other overloads must require that
					// same type.
					: fn.m_spec->m_paramTypes.Params()[count] == type;
			});

            ////int src, dst;
            ////dst = 0;
            ////for (src = 0; src < functionCount; src++) {

            ////    if (isAnyType) {
            ////        // If the first function to match accepts an "any type"
            ////        // parameter, then all other overloads must be an "any type"
            ////        // parameter.
            ////        if (functions[src].m_spec->m_paramValidationCallback(count, m_regType))
            ////            functions[dst++] = functions[src];
            ////    }
            ////    else {
            ////        // Likewise if the first function to match requires a specific
            ////        // parameter type, then all other overloads must require that
            ////        // same type.
            ////        if (functions [src].m_spec->m_paramTypes.Params () [count] == type)
            ////            functions [dst++] = functions [src];
            ////    }
            ////}
            ////functionCount = dst;
            assert (functionCount > 0);     // (Should at least have the function that originally matched the register)

            // Generate code to push parameter to stack
            CompilePush();
            pushCount++;

            // If parameter is an "any type" then generate code to push the
            // parameter type to the stack.
            if (isAnyType) {
                AddInstruction(OP_LOAD_CONST, VTP_INT, vmValue((vmInt)m_vm.StoreType(m_regType)));
                m_regType = VTP_INT;
                CompilePush();
                pushCount++;
            }
        }
        else
            return false;                   // No function matched. (Return last compile convert error).

        count++;                            // Count parameters pushed
    }

    // Find the first function instance that accepts this number of parameters
    int matchIndex = -1;
    int i;
    for (i = 0; i < functionCount && matchIndex < 0; i++)
        if (functions [i].m_spec->m_paramTypes.Params ().size () == count)
            matchIndex = i;
    if (matchIndex < 0) {
        if (count == 0) SetError ("Expected function parameter");
        else            SetError ("Expected ','");
        return false;
    }
    compExtFuncSpec& spec = functions[matchIndex];

    // Expect closing bracket
    if (type == FT_NORMAL && brackets) {
        if (m_token.m_text != ")") {
            SetError ("Expected ')'");
            return false;
        }
        // Skip it
        if (!GetToken ())
            return false;
    }

    // Generate code to call function
	AddFunctionCallInstruction(spec);

    // If function has return type, it will have changed the type in the register
    if (spec.m_spec->m_isFunction) {
        m_regType = spec.m_spec->m_returnType;

        // If data is too large to fit in the register, it will be returned
        // in the "temp" area. If the data contains strings, they will need to
        // be "destroyed" when temp data is unwound.
        if (!m_regType.CanStoreInRegister() && m_vm.DataTypes().ContainsString(m_regType))
            AddInstruction(OP_REG_DESTRUCTOR, VTP_INT, vmValue((vmInt)m_vm.StoreType(m_regType)));

        if (!CompileDataLookup (false))
            return false;
    }

    // Note whether function has generated temporary data
    m_freeTempData = m_freeTempData | spec.m_spec->m_freeTempData;

    // Generate code to clean up stack
    for (int i2 = 0; i2 < pushCount; i2++)
        if (!CompilePop ())
            return false;

    // Generate explicit timesharing break (if necessary)
	AddTimeshareOpCode(spec);

    return true;
}

bool TomBasicCompiler::CompileSwap()
{
	// Generate swap op code
	AddInstruction(OP_SWAP, VTP_UNDEFINED, vmValue());

	// Swap register types
	auto temp = m_regType;
	m_regType = m_reg2Type;
	m_reg2Type = temp;

	return true;
}

bool TomBasicCompiler::CompileConvert (vmValType type) {

    // Can convert NULL to a different pointer type, or function pointer
    if (m_regType.IsNull ()) {
        if (type.VirtualPointerLevel () <= 0 && type.m_basicType != VTP_FUNC_PTR) {
            SetError ("Cannot convert NULL to " + m_vm.DataTypes ().DescribeVariable ("", type));
            return false;
        }

        // No generated code necessary, just substitute in type
        m_regType = type;
        return true;
    }

    // Can convert values to references. (This is used when evaluating function
    // parameters.)
    if (    type.m_pointerLevel         == 1 && type.m_byRef    // type is a reference
        &&  m_regType.m_pointerLevel    == 0                    // regType is a value
        &&  m_regType.m_basicType       == type.m_basicType     // Same type of data
        &&  m_regType.m_arrayLevel      == type.m_arrayLevel) {

        // Convert register to pointer
        if (CompileTakeAddress ()) {

            // Convert pointer to reference
            m_regType.m_byRef = true;
            return true;
        }
        else
            return false;
    }

    // Can convert to basic types.
    // For non basic types, all we can do is verify that the register contains
    // the type that we expect, and raise a compiler error otherwise.
	if (type.IsBasic()) {
		vmValType prevRegType = m_regType;
		if (!CompileConvert(type.m_basicType))
			return false;

		// For function pointers, also need to verify that the prototypes are compatible
		if (type.m_basicType == VTP_FUNC_PTR && prevRegType.m_basicType == VTP_FUNC_PTR)
		{
			const auto& p1 = m_vm.UserFunctionPrototypes()[type.m_prototypeIndex];
			const auto& p2 = m_vm.UserFunctionPrototypes()[prevRegType.m_prototypeIndex];
			if (p1.IsCompatibleWith(p2))
				return true;
		}
		else
			return true;
	}
    else if (m_regType.ExactEquals (type))  return true;        // Note: Exact equals is required as == will say that pointers are equal to references.
                                                                // (Internally this is true, but we want to enforce that programs use the correct type.)

    SetError ("Cannot convert to " + m_vm.DataTypes ().DescribeVariable ("", type));
    return false;
}

bool TomBasicCompiler::CompileConvert2 (vmValType type) {

	// Can convert NULL to a different pointer type
	if (m_reg2Type.IsNull()) {
		if (type.VirtualPointerLevel() <= 0 && type.m_basicType != VTP_FUNC_PTR) {
			SetError("Cannot convert NULL to " + m_vm.DataTypes().DescribeVariable("", type));
			return false;
		}

		// No generated code necessary, just substitute in type
		m_reg2Type = type;
		return true;
	}

	// Can convert to basic types.
	// For non basic types, all we can do is verify that the register contains
	// the type that we expect, and raise a compiler error otherwise.
	if (type.IsBasic()) {
		vmValType prevRegType = m_reg2Type;
		if (!CompileConvert2(type.m_basicType))
			return false;

		// For function pointers, also need to verify that the prototypes are compatible
		if (type.m_basicType == VTP_FUNC_PTR && prevRegType.m_basicType == VTP_FUNC_PTR)
		{
			const auto& p1 = m_vm.UserFunctionPrototypes()[type.m_prototypeIndex];
			const auto& p2 = m_vm.UserFunctionPrototypes()[prevRegType.m_prototypeIndex];
			if (p1.IsCompatibleWith(p2))
				return true;
		}
		else
			return true;
	}
	else if (m_reg2Type.ExactEquals(type))   return true;      // Note: Exact equals is required as == will say that pointers are equal to references.
	// (Internally this is true, but we want to enforce that programs use the correct type.)

	SetError("Cannot convert to " + m_vm.DataTypes().DescribeVariable("", type));
	return false;
}

bool TomBasicCompiler::CompileConstant () {

    // Skip CONST
    if (!GetToken ())
        return false;

    // Expect at least one field in dim
    if (AtSeparatorOrSpecial ()) {
        SetError ("Expected constant declaration");
        return false;
    }

    // Parse fields in dim
    bool needComma = false;             // First element doesn't need a comma
    while (!AtSeparatorOrSpecial ()) {

        // Handle commas
        if (needComma) {
            if (m_token.m_text != ",") {
                SetError ("Expected ','");
                return false;
            }
            if (!GetToken ())
                return false;
        }
        needComma = true;               // Remaining elements do need commas

        // Read constant name
        if (!m_token.m_type == CTT_TEXT) {
            SetError ("Expected constant name");
            return false;
        }
        std::string name = m_token.m_text;
        if (m_programConstants.find (name) != m_programConstants.end ()) {
            SetError ("'" + name + "' has already been declared as a constant.");
            return false;
        }
        if (!CheckName (name))
            return false;
        if (!GetToken ())
            return false;

        // Determine constant type from last character of constant name
        vmBasicValType type = VTP_UNDEFINED;
        if (name.length () > 0) {
            char last = name [name.length () - 1];
            if (last == '$')                        type = VTP_STRING;
            else if (last == '#' || last == '!')    type = VTP_REAL;
            else if (last == '%')                   type = VTP_INT;
        }

        if (m_token.m_text == "as") {
            if (type != VTP_UNDEFINED) {
                SetError ("'" + name + "'s type has already been defined. Cannot use 'as' here.");
                return false;
            }
            if (!GetToken ())
                return false;
            if (m_token.m_text == "integer")
                type = VTP_INT;
            else    if (m_token.m_text == "single"
                    ||  m_token.m_text == "double")

                // Note: Basic4GL supports only one type of floating point number.
                // We will accept both keywords, but simply allocate a real (= single
                // precision) floating point number each time.
                type = VTP_REAL;
            else if (m_token.m_text == "string")
                type = VTP_STRING;
            else {
                SetError("Expected 'integer', 'single', 'double', 'string'");
                return false;
            }
            if (!GetToken ())
                return false;
        }

        // Default type to integer if not defined
        if (type == VTP_UNDEFINED)
            type = GetDefaultDataType();

        // Expect =
        if (m_token.m_text != "=") {
            SetError ("Expected '='");
            return false;
        }
        if (!GetToken ())
            return false;

        // Compile constant expression
        vmValue value;
        std::string stringValue;
        if (!EvaluateConstantExpression (type, value, stringValue))
            return false;
        switch (type) {
        case VTP_INT:       m_programConstants [name] = compConstant (value.IntVal ());     break;
        case VTP_REAL:      m_programConstants [name] = compConstant (value.RealVal ());    break;
        case VTP_STRING:    m_programConstants [name] = compConstant ((std::string) ("S" + stringValue));         break;
        };

/*        // Expect constant value
        if (m_token.m_type != CTT_CONSTANT) {
            SetError ("Expected constant value");
            return false;
        }

        // Validate constant type matches that expected
        if (type == VTP_STRING && m_token.m_valType != VTP_STRING) {
            SetError ("String constant expected");
            return false;
        }
        else if (type != VTP_STRING && m_token.m_valType == VTP_STRING) {
            SetError ("Number constant expected");
            return false;
        }

        // Register constant
        switch (m_token.m_valType) {
        case VTP_INT:       m_programConstants [name] = compConstant (StringToInt  (m_token.m_text));   break;
        case VTP_REAL:      m_programConstants [name] = compConstant (StringToReal (m_token.m_text));   break;
        case VTP_STRING:    m_programConstants [name] = compConstant (m_token.m_text);                  break;
        };

        // Skip constant value
        if (!GetToken ())
            return false;*/
    }
    return true;
}

bool TomBasicCompiler::CompileFreeTempData    () {

    // Add instruction to free temp data (if necessary)
    if (m_freeTempData)
        AddInstruction (OP_FREE_TEMP, VTP_INT, vmValue ());
    m_freeTempData = false;

    return true;
}

std::string TomBasicCompiler::FunctionName (int index) {
    for (   compFuncIndex::iterator i = m_functionIndex.begin ();
            i != m_functionIndex.end ();
            i++)
        if ((*i).second == index)
            return (*i).first;
    return "???";
}

bool TomBasicCompiler::CompileAlloc () {

    // Skip "alloc"
    if (!GetToken ())
        return false;

    // Expect &pointer variable
    if (m_token.m_text == "&") {
        SetError ("First argument must be a pointer");
        return false;
    }

    // Load pointer
    if (!(CompileLoadVar () && CompileTakeAddress ()))
        return false;

    // Store pointer type
    vmValType   ptrType     = m_regType,
                dataType    = m_regType;
    dataType.m_byRef = false;
    dataType.m_pointerLevel--;

    // Get pointer address
    if (!CompileTakeAddress ()) {
        SetError ("First argument must be a pointer");
        return false;
    }

    // Push destination address to stack
    if (!CompilePush ())
        return false;

    // Generate code to push array dimensions (if any) to the stack.
    int i;
    for (i = 0; i < dataType.m_arrayLevel; i++) {

        // Expect ,
        if (m_token.m_text != ",") {
            SetError ("Expected ','");
            return false;
        }
        if (!GetToken ())
            return false;

        // Generate code to evaluate array index, and convert to an integer.
        if (!CompileExpression ())
            return false;
        if (!CompileConvert (VTP_INT)) {
            SetError ("Array index must be a number. " + m_vm.DataTypes ().DescribeVariable ("", m_regType) + " is not a number");
            return false;
        }

        // Push array index to stack
        if (!CompilePush ())
            return false;
    }

    // Add alloc instruction
    AddInstruction (OP_ALLOC, VTP_INT, vmValue ((vmInt) m_vm.StoreType (dataType)));

    // Instruction automatically removes all array indices that were pushed to
    // the stack.
    for (i = 0; i < dataType.m_arrayLevel; i++)
        m_operandStack.pop_back ();

    // Instruction also automatically leaves the register pointing to the new
    // data.
    m_regType = ptrType;
    m_regType.m_byRef = false;

    // Generate code to pop destination address
    if (!CompilePop ())
        return false;

    // Generate code to save address to pointer
    AddInstruction (OP_SAVE, VTP_INT, vmValue ());

    return true;
}

compParserPos TomBasicCompiler::SavePos () {

    // Save the current parser position, so we can return to it later.
    compParserPos pos;
    pos.m_line  = m_parser.Line ();
    pos.m_col   = m_parser.Col ();
    pos.m_token = m_token;
    return pos;
}

void TomBasicCompiler::RestorePos (compParserPos& pos) {

    // Restore parser position
    m_parser.SetPos (pos.m_line, pos.m_col);
    m_token = pos.m_token;
}

std::string TomBasicCompiler::DescribeStackCall (unsigned int returnAddr) {

    // Return a string describing the gosub call
    if (returnAddr == 0 || returnAddr >= m_vm.InstructionCount ())
        return "???";

    // Look at instruction immediately before return address.
    // This should be the gosub
    if (m_vm.Instruction (returnAddr - 1).m_opCode != OP_CALL)
        return "???";

    // Get target address
    unsigned int target = m_vm.Instruction (returnAddr - 1).m_value.IntVal ();

    // Lookup label name
    compLabelIndex::iterator i = m_labelIndex.find (target);
    if (i == m_labelIndex.end ())
        return "???";

    // Return label name
    return (*i).second;
}

bool TomBasicCompiler::TempCompileExpression (
    std::string expression,
    vmValType& valType,
    bool inFunction,
    int currentFunction) {

    // Load expression into parser
    m_parser.SourceCode ().clear ();
    m_parser.SourceCode ().push_back (expression);
    m_parser.Reset ();
    m_lastLine = 0;
    m_lastCol  = 0;

    // Reset compiler state
    ClearState ();
    m_inFunction = inFunction;
    m_currentFunction = currentFunction;
	m_currentCodeBlockIndex = 0;

    // Clear error state
    ClearError ();
    m_parser.ClearError ();

    // Read first token
    if (!GetToken (true))
        return false;

    // Compile code
    if (!CompileExpression ())
        return false;

    if (m_token.m_type != CTT_EOL) {
        SetError ("Extra characters after expression");
        return false;
    }

    // Terminate program
    AddInstruction (OP_END, VTP_INT, vmValue ());

    // Return expression result type
    valType = m_regType;
    return true;
}

bool TomBasicCompiler::CompileData () {

    // Skip "data"
    if (!GetToken (false, true))            // Use "DATA" mode read
        return false;

    // Compile data elements
    bool needComma = false;
    do {

        // Handle commas
        if (needComma) {
            if (m_token.m_text != ",") {
                SetError ("Expected ','");
                return false;
            }
            if (!GetToken (false, true))
                return false;
        }
        needComma = true;               // Remaining elements do need commas

        // Consecutive commas?
        if (m_token.m_text == "," || AtSeparatorOrSpecial ()) {

            // Store a blank string
            m_vm.StoreProgramData (VTP_STRING, vmValue (0));
        }
        else {

            // Extract value
            vmValue v;
            if (m_token.m_valType == VTP_STRING) {

                // Allocate new string constant
                std::string text = m_token.m_text.substr (1, m_token.m_text.length () - 1);     // Remove S prefix
                v.IntVal () = m_vm.StoreStringConstant (text);
            }
            else if (m_token.m_valType == VTP_INT)
                v.IntVal () = StringToInt (m_token.m_text);
            else
                v.RealVal () = StringToReal (m_token.m_text);

            // Store data in VM
            m_vm.StoreProgramData (m_token.m_valType, v);

            // Next token
            if (!GetToken ())
                return false;
        }
    } while (!AtSeparatorOrSpecial ());
    return true;
}

bool TomBasicCompiler::CompileDataRead () {

    // Skip "read"
    if (!GetToken ())
        return false;

    // Expect at one variable name
    if (AtSeparatorOrSpecial ()) {
        SetError ("Expected variable name");
        return false;
    }

    // Parse fields in dim
    bool needComma = false;             // First element doesn't need a comma
    while (!AtSeparatorOrSpecial ()) {

        // Handle commas
        if (needComma) {
            if (m_token.m_text != ",") {
                SetError ("Expected ','");
                return false;
            }
            if (!GetToken ())
                return false;
        }
        needComma = true;               // Remaining elements do need commas

        // Generate code to load target variable address
        if (!CompileLoadVar ())
            return false;

        // Must be a basic type.
        vmValType type = m_regType;
        if (!type.IsBasic ()) {
            SetError ("Can only READ built in types (int, real or string)");
            return false;
        }

        // Convert load target variable into take address of target variable
        if (!CompileTakeAddress ()) {
            SetError ("Value cannot be READ into");
            return false;
        }

        if (!CompilePush ())
            return false;

        // Generate READ op-code
        AddInstruction (OP_DATA_READ, type.m_basicType, vmValue ());

        if (!CompilePop ())
            return false;

        // Save reg into [reg2]
        AddInstruction (OP_SAVE, m_reg2Type.m_basicType, vmValue ());
    }
    return true;
}

bool TomBasicCompiler::CompileDataReset () {

    // Skip "reset"
    if (!GetToken ())
        return false;

    // If label specified, use offset stored in label
    if (!AtSeparatorOrSpecial ()) {

        // Validate label
        if (m_token.m_type != CTT_TEXT) {
            SetError ("Expected label name");
            return false;
        }

        // Record reset, so that we can fix up the offset in the second compile pass.
        std::string labelName = m_symbolPrefix + m_token.m_text;
        m_resets.push_back (compJump (m_vm.InstructionCount (), labelName));

        // Skip label name
        if (!GetToken ())
            return false;
    }

    // Add jump instruction
    AddInstruction (OP_DATA_RESET, VTP_INT, vmValue (0));

    return true;
}

bool TomBasicCompiler::CompileArrayMax()
{
	// Skip "arraymax"
	if (!GetToken())
		return false;

	// Function takes a parameter and returns a value. Thus it requires brackets in all syntax variations.
	if (m_token.m_text != "(")
	{
		SetError("Expected '('");
		return false;
	}
	if (!GetToken())
		return false;

	// Compile single parameter
	if (!CompileExpression())
		return false;

	// Expect this to be an array
	if (m_regType.m_arrayLevel == 0)
	{
		SetError("Expected an array");
		return false;
	}

	// Closing ")"
	if (m_token.m_text != ")")
	{
		SetError("Expected ')'");
		return false;
	}
	if (!GetToken())
		return false;

	// Push array as function parameter
	if (!CompilePush())
		return false;

	// Look up arraymax function
	compExtFuncSpec spec = FindFunctionExt("arraymax", true, VTP_UNDEFINED);		// Must have a parameter, but don't check it's type
	if (spec.m_spec == nullptr)
		return false;
	if (!spec.m_spec->m_isFunction || !(spec.m_spec->m_returnType == VTP_INT))
	{
		SetError("'arraymax' function should return an integer.");
		return false;
	}

	// Generate call
	AddFunctionCallInstruction(spec);	
	if (!CompilePop())
		return false;

	// Return type is integer
	m_regType = VTP_INT;

	return true;
}

int TomBasicCompiler::GetFnPointerPrototypeIndex(const vmUserFuncPrototype& prototype)
{
	// TODO: Optimise with hash lookup?

	// Look for existing matching prototype
	auto& prototypes = m_vm.UserFunctionPrototypes();
	for (int i = 0; i < prototypes.size(); i++)
		if (prototypes[i].IsCompatibleWith(prototype))
			return i;

	// Or add one if none found
	prototypes.push_back(prototype);
	return prototypes.size() - 1;
}


bool TomBasicCompiler::CompileUserFunctionAddress()
{
	// Current token must be the user function name
	assert(m_token.m_type == CTT_USER_FUNCTION);

	// Read function name
	std::string name = m_token.m_text;
	if (!GetToken())
		return false;

	// Lookup prototype
	int index = GetUserFunctionIndex(name);

	// Add op code to load function index + 1 into register.
	// Note: We add 1, so that we can reserve 0 as a "null" value.
	// (This makes the code simpler, as we don't have to have any special initialisation logic, 
	// or null checking logic when assigning function pointers).
	AddInstruction(OP_LOAD_CONST, VTP_FUNC_PTR, vmValue(index + 1));

	// Set register type
	m_regType = vmValType(VTP_FUNC_PTR);
	m_regType.m_prototypeIndex = m_vm.UserFunctions()[index].prototypeIndex;

	return true;
}

bool TomBasicCompiler::CheckTypeIsDefined(const vmValType& type)
{
	// Check that field data type has been fully defined
	if (type.m_basicType >= 0)
	{
		const auto& struc = m_vm.DataTypes().Structures()[type.m_basicType];
		if (!struc.m_isDefined)
		{
			SetError("Structure type '" + struc.m_name + "' has not been defined yet");
			return false;
		}
	}

	return true;
}

bool TomBasicCompiler::EvaluateConstantExpression (vmBasicValType& type, vmValue& result, std::string& stringResult) {

    // Note: If type is passed in as VTP_UNDEFINED, then any constant expression
    // will be accepted, and type will be set to the type that was found.
    // Otherwise it will only accept expressions that can be cast to the specified type.
    // If the expression is a string, its value will be returned in stringResult.
    // Otherwise its value will be returned in result.

    // Mark the current size of the program. This is where the expression will start
    int expressionStart = m_vm.InstructionCount ();

    // Compile expression, specifying that it must be constant
    if (!CompileExpression (true))
        return false;

    // Convert to required type
    if (type != VTP_UNDEFINED)
        if (!CompileConvert (type))
            return false;

    // Add "end program" opcode, so we can safely evaluate it
    AddInstruction (OP_END, VTP_INT, vmValue ());

    // Setup virtual machine to execute expression
    // Note: Expressions can't branch or loop, and it's very difficult to write
    // one that evaluates to a large number of op-codes. Therefore we won't worry
    // about processing windows messages or checking for pause state etc.
    m_vm.ClearError ();
    m_vm.GotoInstruction (expressionStart);
    try {
        do {
            m_vm.Continue (1000);
        } while (!m_vm.Error () && !m_vm.Done ());
    }
    catch (...) {
        SetError ("Error evaluating constant expression");
        return false;
    }
    if (m_vm.Error ()) {
        SetError ("Error evaluating constant expression");
        return false;
    }

    // Now we have the result type of the constant expression,
    // AND the virtual machine has its value stored in the register.

    // Roll back all the expression op-codes
    m_vm.GotoInstruction (0);               
    m_vm.RollbackProgram (expressionStart);

    // Set return values
    type = m_regType.m_basicType;
    if (type == VTP_STRING) stringResult    = m_vm.RegString ();
    else                    result          = m_vm.Reg ();

    return true;
}

bool TomBasicCompiler::CompileConstantExpression (vmBasicValType type) {

    // Evaluate constant expression
    vmValue value;
    std::string stringValue = "";
    if (!EvaluateConstantExpression (type, value, stringValue))
        return false;

    // Generate "load constant" instruction
    if (type == VTP_STRING) {

        // Create string constant entry if necessary
        int index = m_vm.StoreStringConstant (stringValue);
        AddInstruction (OP_LOAD_CONST, VTP_STRING, vmValue (index));
    }
    else
        AddInstruction (OP_LOAD_CONST, type, value);
        
    return true;
}

compExtFuncSpec TomBasicCompiler::FindFunction(std::string name, vmBasicValType paramType) {
	return FindFunctionExt(name, paramType != VTP_UNDEFINED, paramType);
}

compExtFuncSpec TomBasicCompiler::FindFunctionExt(std::string name, bool hasParam, vmBasicValType paramType) {

	// Search for plugin function with matching name & param type
	compExtFuncSpec functions[TC_MAXOVERLOADEDFUNCTIONS];
	int count = 0;
	m_plugins.FindFunctions(name, functions, count, TC_MAXOVERLOADEDFUNCTIONS);
	for (int i = 0; i < count; i++) {
		vmValTypeList params = functions[i].m_spec->m_paramTypes.Params();
		if ((params.size() == 0 && !hasParam) ||
			(params.size() == 1 && hasParam && (paramType == VTP_UNDEFINED || params[0] == paramType)))
			return functions[i];
	}

	// Search for builtin function with matching name & param type
	compExtFuncSpec result;
	result.m_builtin = true;
	for (compFuncIndex::iterator it = m_functionIndex.find(name);
		it != m_functionIndex.end() && (*it).first == name;
		it++) {
		compFuncSpec& spec = m_functions[(*it).second];
		vmValTypeList params = spec.m_paramTypes.Params();
		if ((params.size() == 0 && !hasParam) ||
			(params.size() == 1 && hasParam && (paramType == VTP_UNDEFINED || params[0] == paramType)))
		{
			result.m_spec = &spec;
			return result;
		}
	}

	// None found
	SetError((std::string) "'" + name + "' function not found");
	result.m_spec = NULL;
	return result;
}

bool TomBasicCompiler::CompilePrint (bool forceNewline) {

    // The print function has a special syntax, and must be compiled separtely

    // Skip "print"
    if (!GetToken())
        return false;

    bool foundSemiColon = false;
    int operandCount = 0;
    while (!AtSeparatorOrSpecial()) {

        // Look for semicolon
        if (m_token.m_text == ";") {

            // Record it, and move on to next
            foundSemiColon = true;
            if (!GetToken())
                return false;
        }
        else {
            foundSemiColon = false;

            // If this is not the first operand, then there will be a string
            // sitting in the register. Need to push it first.
            if (operandCount > 0)
                if (!CompilePush())
                    return false;

            // Evaluate expression & convert it to string
            if (!(CompileExpression() && CompileConvert(VTP_STRING)))
                return false;

            operandCount++;
        }
    }

    // Add all operands together
    while (operandCount > 1) {
        if (!CompilePop())
            return false;
        AddInstruction(OP_OP_PLUS, VTP_STRING, vmValue());
        m_regType = VTP_STRING;

        operandCount--;
    }

    // Push string as function parameter
    if (operandCount == 1)
        if (!CompilePush())
            return false;

    // Find print/printr function
    bool newLine = forceNewline ||
        ((m_syntax == LS_TRADITIONAL || m_syntax == LS_TRADITIONAL_PRINT || m_syntax == LS_TRADITIONAL_SUFFIX) && !foundSemiColon);

    if (!newLine && operandCount == 0)                      // Nothing to print?
        return true;                                        // Do nothing!

	compExtFuncSpec spec = FindFunction(newLine ? "printr" : "print", operandCount == 1 ? VTP_STRING : VTP_UNDEFINED);
	if (spec.m_spec == NULL)
		return false;
	
    // Generate code to call it
	AddFunctionCallInstruction(spec);
	AddTimeshareOpCode(spec);

    // Generate code to clean up stack
    if (operandCount == 1)
        if (!CompilePop ())
            return false;

    return true;
}

bool TomBasicCompiler::CompileInput () {

    // Input also has a special syntax.
    // This still isn't a complete input implementation, as it doesn't support
    // inputting multiple values on one line.
    // But it's a lot better than before.

    // Skip "input"
    if (!GetToken())
        return false;

    // Check for prompt
    if (m_token.m_type == CTT_CONSTANT && m_token.m_valType == VTP_STRING) {

        // Allocate new string constant
        std::string text = m_token.m_text.substr (1, m_token.m_text.length () - 1);     // Remove S prefix

        if (!GetToken())
            return false;          

        // Expect , or ;
        if (m_token.m_text == ";")
            text = text + "? ";
        else if (m_token.m_text != ",") {
            SetError("Expected ',' or ';'");
            return false;
        }
        if (!GetToken())
            return false;

        // Create new string constant
        int index = m_vm.StoreStringConstant (text);

        // Generate code to print it (load, push, call "print" function)
        AddInstruction (OP_LOAD_CONST, VTP_STRING, vmValue (index));
        m_regType = VTP_STRING;
        if (!CompilePush())
            return false;

        // Generate code to call "print" function
        compExtFuncSpec printSpec = FindFunction("print", VTP_STRING);
		if (printSpec.m_spec == NULL)
            return false;
		AddFunctionCallInstruction(printSpec);
		AddTimeshareOpCode(printSpec);

        // Generate code to clean up stack
        if (!CompilePop ())
            return false;
    }

    // Generate code to effectively perform
    //      variable = Input$()
    // or
    //      variable = val(Input$())
    //
    // (Depending on whether variable is a string)

    // Generate code to load target variable
    if (!CompileLoadVar ())
        return false;

    // Must be a simple variable
    if (!m_regType.IsBasic()) {
        SetError("Input variable must be a basic string, integer or real type");
        return false;
    }
    vmBasicValType variableType = m_regType.m_basicType;

    // Generate code to push its address to stack
    if (!(CompileTakeAddress() && CompilePush()))
        return false;

    // Generate code to call "input$()" function
    compExtFuncSpec inputSpec = FindFunction("input$", VTP_UNDEFINED);
	if (inputSpec.m_spec == NULL)
        return false;
	AddFunctionCallInstruction(inputSpec);
    AddInstruction (OP_TIMESHARE, VTP_INT, vmValue ());         // Timesharing break is necessary
    m_regType = VTP_STRING;

    // If the variable is not a string, then we need to convert it to the target
    // type. We do this by inserting an implicit call to the val() function.
    if (variableType != VTP_STRING) {

        // Push register back as input to val function
        if (!CompilePush())
            return false;

        // Generate code to call "val()" function
		compExtFuncSpec valSpec = FindFunction("val", VTP_STRING);
		if (valSpec.m_spec == NULL)
            return false;
		AddFunctionCallInstruction(valSpec);
		AddTimeshareOpCode(valSpec);
		m_regType = VTP_REAL;

        // Clean up stack
        if (!CompilePop())
            return false;
    }

    // Generate code to pop target address into reg2
    if (!CompilePop ())
        return false;

    if (!CompileConvert (m_reg2Type.m_basicType)) {
        SetError ("Types do not match");        // Technically this should never actually happen
        return false;
    }

    // Generate code to save value
    AddInstruction (OP_SAVE, m_reg2Type.m_basicType, vmValue ());

    return true;
}

bool TomBasicCompiler::CompileLanguage () {

    // Compile language directive
    // Skip "language"
    if (!GetToken())
        return false;

    // Expect syntax type
            if (m_token.m_text == "traditional")        m_syntax = LS_TRADITIONAL;
    else    if (m_token.m_text == "basic4gl")           m_syntax = LS_BASIC4GL;
    else    if (m_token.m_text == "traditional_print")  m_syntax = LS_TRADITIONAL_PRINT;
    else    if (m_token.m_text == "traditional_suffix") m_syntax = LS_TRADITIONAL_SUFFIX;
    else {
        SetError ("Expected 'traditional', 'basic4gl', 'traditional_print' or 'traditional_suffix'");
        return false;
    }

    // Skip syntax token
    if (!GetToken())
        return false;

    return true;
}

bool TomBasicCompiler::CompileDo() {

    // Save loop position
    int loopPos = m_vm.InstructionCount ();

    // Skip "do"
    int line = m_parser.Line (), col = m_parser.Col ();
    if (!GetToken ())
        return false;

    // Look for "while" or "until"
    if (m_token.m_text == "while" || m_token.m_text == "until") {

        // Is this a negative condition?
        bool negative = m_token.m_text == "until";

        // Skip "while" or "until"
        if (!GetToken())
            return false;

        // Generate code to evaluate expression
        if (!CompileExpression ())
            return false;

        // Generate code to convert to integer
        if (!CompileConvert (VTP_INT))
            return false;

        // Free any temporary data expression may have created
        if (!CompileFreeTempData ())
            return false;

        // Create flow control structure
        m_flowControl.push_back (compFlowControl (FCT_DO_PRE, m_vm.InstructionCount (), loopPos, line, col));

        // Create conditional jump
        AddInstruction (negative ? OP_JUMP_TRUE : OP_JUMP_FALSE, VTP_INT, vmValue (0));

        // Done
        return true;
    }
    else {

        // Post condition DO.
        // Create flow control structure
        m_flowControl.push_back (compFlowControl (FCT_DO_POST, m_vm.InstructionCount (), loopPos, line, col));
        return true;
    }
}

bool TomBasicCompiler::CompileLoop() {

    if (!(FlowControlTopIs(FCT_DO_PRE) || FlowControlTopIs(FCT_DO_POST))) {
        SetError("'loop' without 'do'");
        return false;
    }

    // Find DO details
    compFlowControl top = FlowControlTOS ();
    m_flowControl.pop_back ();

    // Skip "DO"
    if (!GetToken())
        return false;

    // Look for "while" or "until"
    if (m_token.m_text == "while" || m_token.m_text == "until") {

        // This must be a post condition "do"
        if (top.m_type != FCT_DO_POST) {
            SetError("'until' or 'while' condition has already been specified for this 'do'");
            return false;
        }

        // Is this a negative condition?
        bool negative = m_token.m_text == "until";

        // Skip "while" or "until"
        if (!GetToken())
            return false;

        // Generate code to evaluate expression
        if (!CompileExpression ())
            return false;

        // Generate code to convert to integer
        if (!CompileConvert (VTP_INT))
            return false;

        // Free any temporary data expression may have created
        if (!CompileFreeTempData ())
            return false;

        // Create conditional jump back to "do"
        AddInstruction (negative ? OP_JUMP_FALSE : OP_JUMP_TRUE, VTP_INT, vmValue (top.m_jumpLoop));

        // Done
        return true;
    }
    else {

        // Jump unconditionally back to "do"
        AddInstruction (OP_JUMP, VTP_INT, vmValue(top.m_jumpLoop));

        // If this is a precondition "do", fixup the jump around the "do" block
        if (top.m_type == FCT_DO_PRE) {
            assert ((unsigned)top.m_jumpOut < m_vm.InstructionCount ());
            m_vm.Instruction (top.m_jumpOut).m_value.IntVal () = m_vm.InstructionCount ();
        }

        // Done
        return true;
    }
}

bool TomBasicCompiler::CompileFwdDecl()
{
	// Skip "declare"
	if (!GetToken())
		return false;

	// Function / subroutine
	if (m_token.m_text == "function" || m_token.m_text == "sub")
		return CompileUserFunction(UFT_FWDDECLARATION);

	// Structure
	if (m_token.m_text == "struc" || m_token.m_text == "type")
		return CompileStructureFwdDecl();

	SetError("Expected 'function', 'sub', 'struc' or 'type'");
	return false;
}

bool TomBasicCompiler::CompileUserFunctionFwdDecl() {

    // Skip "declare"
    if (!GetToken())
        return false;

    // Look for "sub" or "function"
    return CompileUserFunction(UFT_FWDDECLARATION);
}

bool TomBasicCompiler::CompileUserFunctionRuntimeDecl() {

    // Skip "runtime"
    if (!GetToken())
        return false;

    return CompileUserFunction(UFT_RUNTIMEDECLARATION);
}

bool TomBasicCompiler::CompileUserFunction(compUserFunctionType funcType)
{
	vmValType dummy;
	return CompileUserFunction(funcType, dummy);
}

bool TomBasicCompiler::CompileUserFunction(compUserFunctionType funcType, vmValType& fnPtrType) {

	// Note: fnPtrType is used to return corresponding function pointer type, if funcType = UFT_POINTER

    // Function or sub?
    bool hasReturnVal;
    if (m_token.m_text == "function")
        hasReturnVal = true;
    else if (m_token.m_text == "sub")
        hasReturnVal = false;
    else {
        SetError("Expected 'sub' or 'function'");
        return false;
    }

    // Check that we are not already inside a function
    if (funcType != UFT_POINTER && m_inFunction) {
        SetError("Cannot define a function or subroutine inside another function or subroutine");
        return false;
    }

    // Check that we are not inside a control structure
	if (funcType != UFT_POINTER && !CheckUnclosedFlowControl())
        return false;

    // Mark start of function in source code
    m_functionStart.m_sourceLine = m_parser.Line();
    m_functionStart.m_sourceCol = m_parser.Col();

    // Skip "func"
    if (!GetToken())
        return false;

    // Compile data type
    vmValType type = VTP_UNDEFINED;
    std::string name = "";
	if (funcType != UFT_POINTER)
	{
		// Note: Pointers to function types don't have a name.
		// Which also means they don't have anything to suffix with a data type.
		// (Return type can be declared after the function definition, with "as").

		// Otherwise parse name and data type as normal
		compTokenType tokenType;
		if (hasReturnVal) {
			if (!CompileDataType(name, type, tokenType))
				return false;
		}
		else {
			if (!CompileTokenName(name, tokenType, false))
				return false;
		}

		// Validate function name
		if (tokenType != CTT_TEXT && tokenType != CTT_USER_FUNCTION && tokenType != CTT_RUNTIME_FUNCTION) {
			if (tokenType == CTT_FUNCTION)
				SetError((std::string)"'" + name + "' has already been used as a built-in function/subroutine name");
			else
				SetError("Expected a function/subroutine name");
			return false;
		}

		// Must not be a variable name
		if (m_vm.Variables().GetVar(name) >= 0) {
			SetError((std::string)"'" + name + "' has already been used as a variable name");
			return false;
		}

		// Must not be a structure name
		if (m_vm.DataTypes().GetStruc(name) >= 0) {
			SetError((std::string)"'" + name + "' has already been used as a structure name");
			return false;
		}
	}

    // Allocate a new user function
	vmUserFuncPrototype prototype;

    // Expect "("
    if (m_token.m_text != "(") {
        SetError ("Expected '('");
        return false;
    }
    if (!GetToken())
        return false;

    // Look for function parameters
    if (m_token.m_text != ")") {
        if (!CompileDim(false, true, prototype))
            return false;
    }

    // Expect ")"
    if (m_token.m_text != ")") {
        SetError ("Expected ')'");
        return false;
    }
    if (!GetToken())
        return false;

    // Calculate final return value
    if (hasReturnVal) {

        // Any trailing () denote an array
        while (m_token.m_text == "(") {

            // Room for one more dimension?
            if (type.m_arrayLevel >= VM_MAXDIMENSIONS) {
                SetError ((std::string) "Arrays cannot have more than " + IntToString (VM_MAXDIMENSIONS) + " dimensions.");
                return false;
            }

            // Add dimension
            type.m_arrayLevel++;

            if (!GetToken ())           // Skip "("
                return false;

            // Expect ")"
            if (m_token.m_text != ")") {
                SetError("')' expected");
                return false;
            }
            if (!GetToken())
                return false;
        }

        // "as" keyword (QBasic/FreeBasic compatibility)
        if (m_token.m_text == "as") {
            if (!CompileAs(name, type))
                return false;
        }

        // Default basic type to int if undefined
        if (type.m_basicType == VTP_UNDEFINED)
            type.m_basicType = GetDefaultDataType();

        // Store function return value type
        prototype.hasReturnVal = true;
		prototype.returnValType = type;
    }
    else
		prototype.hasReturnVal = false;

    // Store function, and get its index (in m_currentFunction)
    std::vector<vmUserFunc>& functions = m_vm.UserFunctions();
    std::vector<vmUserFuncPrototype>& prototypes = m_vm.UserFunctionPrototypes();

    if (funcType == UFT_FWDDECLARATION) {
        // Forward declaration.

        // Function name must not already have been used
        if (IsLocalUserFunction(name)) {
            SetError((std::string)"'" + name + "' has already been used as a function/subroutine name");
            return false;
        }

        // Function name must not have been used for a runtime function
        if (IsRuntimeFunction(name)) {
            SetError((std::string)"'" + name + "' has already been used as a runtime function/subroutine name");
            return false;
        }

        // Allocate new function
		prototypes.push_back(prototype);
        functions.push_back(vmUserFunc(prototypes.size() - 1, false));
        m_currentFunction = functions.size() - 1;

        // Map name to function
		CurrentCodeBlock().userFunctions[name] = m_currentFunction;
        if (!IsGlobalUserFunction(name))
            m_globalUserFunctionIndex[name] = m_currentFunction;

        // Build reverse index (for debugger)
        m_userFunctionReverseIndex[m_currentFunction] = name;
    }
    else if (funcType == UFT_RUNTIMEDECLARATION) {

        // Function name must not already have been used
        if (IsLocalUserFunction(name)) {
            SetError((std::string)"'" + name + "' has already been used as a function/subroutine name");
            return false;
        }

        // Function name must not have been used for a runtime function
        if (IsRuntimeFunction(name)) {
            SetError((std::string)"'" + name + "' has already been used as a runtime function/subroutine name");
            return false;
        }

        // Store prototype
		prototypes.push_back(prototype);

        // Store runtime function
        m_runtimeFunctions.push_back(compRuntimeFunction(prototypes.size() - 1));

        // Map name to runtime function
        m_runtimeFunctionIndex[name] = m_runtimeFunctions.size() - 1;
    }
    else if (funcType == UFT_IMPLEMENTATION) {

        // Function implementation

        // Create jump-past-function op-code
        m_functionJumpOver = m_vm.InstructionCount();
        AddInstruction(OP_JUMP, VTP_INT, vmValue(0));   // Jump target will be fixed up when "endfunction" is compiled

        if (IsRuntimeFunction(name)) {

            // Implementation of runtime function
            int index = m_runtimeFunctionIndex[name];
            vmRuntimeFunction& runtimeFunction = CurrentCodeBlock().GetRuntimeFunction(index);

            // Check if already implemented
            if (runtimeFunction.functionIndex >= 0) {
                SetError((std::string)"Runtime function/sub '" + name + "' has already been implemented");
                return false;
            }

            // Function must match runtime prototype
			if (!prototype.Matches(prototypes[m_runtimeFunctions[index].prototypeIndex])) {
                SetError((std::string)"Function/sub does not match its RUNTIME declaration");
                return false;  
            }

            // Allocate new function
			prototypes.push_back(prototype);
            functions.push_back(vmUserFunc(prototypes.size() - 1, true, m_vm.InstructionCount()));
            m_currentFunction = functions.size() - 1;

            // Map runtime function to implementation
            runtimeFunction.functionIndex = m_currentFunction;
        }
        else {
            if (IsLocalUserFunction(name)) {

                // Function already DECLAREd.
                m_currentFunction = CurrentCodeBlock().userFunctions[LowerCase(name)];

                // Must not be already implemented
                if (functions[m_currentFunction].implemented) {
                    SetError((std::string)"'" + name + "' has already been used as a function/subroutine name");
                    return false;
                }

                // Function prototypes must match
				if (!prototype.Matches(prototypes[functions[m_currentFunction].prototypeIndex])) {
                    SetError((std::string)"Function/subroutine does not match how it was DECLAREd");
                    return false;
                }

                // Save updated function spec
                // Function starts at next offset
                functions[m_currentFunction].implemented = true;
                functions[m_currentFunction].programOffset = m_vm.InstructionCount();
            }
            else {

                // Completely new function

                // Allocate a new prototype
				prototypes.push_back(prototype);

                // Allocate a new function
                functions.push_back(vmUserFunc(prototypes.size() - 1, true, m_vm.InstructionCount()));
                m_currentFunction = functions.size() - 1;
            }
            
            // Map name to function
			CurrentCodeBlock().userFunctions[name] = m_currentFunction;
            if (!IsGlobalUserFunction(name))
                m_globalUserFunctionIndex[name] = m_currentFunction;
        }

        // Build reverse index (for debugger)
        m_userFunctionReverseIndex[m_currentFunction] = name;

        // Compile the body of the function
        m_inFunction = true;
    }
	else if (funcType == UFT_POINTER)
	{
		// Find matching prototype, or register a new one
		auto index = GetFnPointerPrototypeIndex(prototype);

		// Update function pointer type.
		// Note that some fields (e.g. array level) are already set by caller
		fnPtrType.m_basicType = VTP_FUNC_PTR;
		fnPtrType.m_prototypeIndex = index;
	}
    return true;
}

bool TomBasicCompiler::CompileEndUserFunction(bool hasReturnVal) {

    // Must be inside a function
    if (!m_inFunction) {
        if (hasReturnVal)   SetError("'endfunction' without 'function'");
        else                SetError("'endsub' without 'sub'");
        return false;
    }

    // Match end sub/function against sub/function type
    if (UserPrototype().hasReturnVal != hasReturnVal) {
        if (hasReturnVal)   SetError("'endfunction' without 'function'");
        else                SetError("'endsub' without 'sub'");
        return false;
    }

    // Check for unclosed flow controls
    if (!CheckUnclosedFlowControl())
        return false;

    // Skip 'endfunction'
    if (!GetToken())
        return false;

    // If end of function is reached without a return value, need to trigger
    // a runtime error.
    if (UserPrototype().hasReturnVal)
        AddInstruction(OP_NO_VALUE_RETURNED, VTP_INT, vmValue(0));
    else
        // Add return-from-user-function instruction
        AddInstruction(OP_RETURN_USER_FUNC, VTP_INT, vmValue(0));

    // Fix up jump-past-function op-code
    assert((unsigned)m_functionJumpOver < m_vm.InstructionCount ());
    m_vm.Instruction(m_functionJumpOver).m_value.IntVal() = m_vm.InstructionCount();

    // Let compiler know we have left the function
    m_inFunction = false;

    // TODO: If function is supposed to return a value, add an op-code that triggers
    // a run-time error (meaning that the end of the function was reached without
    // finding a "return" command).

    return true;
}

bool TomBasicCompiler::CompileStructureFwdDecl()
{
	// Expect "struc" or "type"
	if (m_token.m_text != "struc" && m_token.m_text != "type")
	{
		SetError("Expected 'struc' or 'type'");
		return false;
	}

	// Skip token
	if (!GetToken())
		return false;

	// Check that we are not already inside a function
	if (m_inFunction) {
		SetError("Cannot declare a structure inside a function or subroutine");
		return false;
	}

	// Check that we are not inside a control structure
	if (!CheckUnclosedFlowControl())
		return false;

	// Expect structure name
	if (m_token.m_type != CTT_TEXT) {
		SetError("Expected structure name");
		return false;
	}
	std::string name = m_symbolPrefix + m_token.m_text;
	if (!CheckName(name))
		return false;
	if (m_vm.DataTypes().StrucStored(name)) {         // Must be unused
		SetError((std::string) "'" + name + "' has already been used as a structure name");
		return false;
	}
	if (!GetToken())                                   // Skip structure name
		return false;

	// Create structure. Will be flagged as "undefined"
	m_vm.DataTypes().NewStruc(name);
	
	return true;
}

bool TomBasicCompiler::CompileUserFunctionCall(bool mustReturnValue, bool isRuntimeFunc, bool isFunctionPtr) {
    assert(
		isFunctionPtr ||
        (!isRuntimeFunc && m_token.m_type == CTT_USER_FUNCTION) ||
        ( isRuntimeFunc && m_token.m_type == CTT_RUNTIME_FUNCTION) );
    assert(
		isFunctionPtr ||
        (!isRuntimeFunc && IsUserFunction(m_token.m_text)) ||
        ( isRuntimeFunc && IsRuntimeFunction(m_token.m_text)) );

	std::string name = "";
	int index = -1;
	int prototypeIndex;

	if (isFunctionPtr)
	{
		// Look at function pointer type to get prototype index
		assert(m_regType.m_basicType == VTP_FUNC_PTR);
		assert(m_regType.m_pointerLevel == 0);
		name = "[Function pointer]";
		prototypeIndex = m_regType.m_prototypeIndex;
	}
	else {

		// Read function name
		name = m_token.m_text;
		if (!GetToken())
			return false;

		// Lookup prototype
		if (isRuntimeFunc) {
			index = m_runtimeFunctionIndex[name];
			prototypeIndex = m_runtimeFunctions[index].prototypeIndex;
		}
		else {
			index = GetUserFunctionIndex(name);
			prototypeIndex = m_vm.UserFunctions()[index].prototypeIndex;
		}
	}

    vmUserFuncPrototype& prototype = m_vm.UserFunctionPrototypes()[prototypeIndex];

    if (mustReturnValue && !prototype.hasReturnVal) {
        SetError((std::string) "'" + name + "' does not return a value");
        return false;
    }

    // Add op-code to prepare function stack frame.
    // Stack frame remains inactive while evaluating its parameters.
	if (isFunctionPtr)
		AddInstruction(OP_CREATE_FUNC_PTR_FRAME, VTP_INT, vmValue());			// Index will be in reg2
    else if (isRuntimeFunc)
        AddInstruction(OP_CREATE_RUNTIME_FRAME, VTP_INT, vmValue(index));
    else
        AddInstruction(OP_CREATE_USER_FRAME, VTP_INT, vmValue(index));

    // Determine whether the function needs brackets
    bool brackets = true;
    if (!isFunctionPtr && (m_syntax == LS_TRADITIONAL || m_syntax == LS_TRADITIONAL_SUFFIX))
        brackets = prototype.hasReturnVal && prototype.paramCount > 0;

    // Expect "("
    if (brackets) {
        if (m_token.m_text != "(") {
            SetError ("Expected '('");
            return false;
        }
        if (!GetToken())
            return false;
    }

    // Evaluate function parameters
    bool needComma = false;
    for (int i = 0; i < prototype.paramCount; i++) {
        if (needComma) {
            if (m_token.m_text != ",") {
                SetError("Expected ','");
                return false;
            }
            if (!GetToken())
                return false;
        }
        needComma = true;

        if (!CompileUserFuncParam(prototype, i))
            return false;
    }

    // Expect ")"
    if (brackets) {
        if (m_token.m_text != ")") {
            SetError ("Expected ')'");
            return false;
        }
        if (!GetToken())
            return false;
    }

    // Add op-code to call function.
    // Type: Unused.
    // Value: index of function specification.
    AddInstruction(OP_CALL_USER_FUNC, VTP_INT, vmValue());

	if (prototype.hasReturnVal) {

		// Data containing strings will need to be "destroyed" when the stack unwinds.
		if (!prototype.returnValType.CanStoreInRegister() &&
			m_vm.DataTypes().ContainsString(prototype.returnValType))
			AddInstruction(OP_REG_DESTRUCTOR, VTP_INT, vmValue((vmInt)m_vm.StoreType(prototype.returnValType)));

		// Set register type to value returned from function (if applies)
		m_regType = prototype.returnValType.RegisterType();
		if (!CompileDataLookup(false))
			return false;

		// If function returns a value larger than the register, temp data will
		// need to be freed.
		if (!prototype.returnValType.CanStoreInRegister()) {
			m_freeTempData = true;
		}
	}
	else
		m_regType = VTP_VOID;				// Indicates no return type

    return true;
}

bool TomBasicCompiler::CompileUserFuncParam(vmUserFuncPrototype& prototype, int i) {

    // Generate code to store result as a function parameter
    vmValType& type = prototype.localVarTypes[i];

    // Basic type case
    if (type.IsBasic()) {

        // Generate code to compile function parameter
        if (!CompileExpression(false))
            return false;

        // Attempt to convert value in reg to same type
        if (!CompileConvert(type)) {			// Note: Use full type, so that prototype # is taken into account for function pointer types.
            SetError ("Types do not match");
            return false;
        }

        // Save reg into parameter
        AddInstruction (OP_SAVE_PARAM, type.m_basicType, vmValue(i));
    }

    // Pointer case. Parameter must be a pointer and m_reg must point to a value accessible through a variable.
    else if (type.VirtualPointerLevel() == 1) {

        // Special case: We accept "null" to pointer parameters
        if (m_token.m_text == "null") {
            if (!CompileNull())
                return false;
            AddInstruction(OP_SAVE_PARAM, VTP_INT, vmValue(i));
        }
        else {

			// Compile expression
			if (!CompileExpression(false))
				return false;

			// Implicitly take address if necessary
			if (m_regType.VirtualPointerLevel() == 0)
				if (!CompileTakeAddress())
					return false;

            // Register should now match the expected type
            if (    m_regType.m_pointerLevel == type.m_pointerLevel &&
                    m_regType.m_arrayLevel == type.m_arrayLevel &&
                    m_regType.m_basicType == type.m_basicType)
                AddInstruction(OP_SAVE_PARAM, VTP_INT, vmValue(i));

            else {
                SetError("Types do not match");
                return false;
            }
        }
    }

    // Not basic, and not a pointer.
    // Must be a large object (structure or array)
    else {
        assert(type.m_pointerLevel == 0);

        // Generate code to compile function parameter
        if (!CompileExpression(false))
            return false;

        if (    m_regType.m_pointerLevel == 1 &&
                m_regType.m_byRef &&
                m_regType.m_arrayLevel == type.m_arrayLevel &&
                m_regType.m_basicType == type.m_basicType) {
            AddInstruction(OP_COPY_USER_STACK, VTP_INT, vmValue((vmInt)m_vm.StoreType(type)));
            AddInstruction(OP_SAVE_PARAM_PTR, VTP_INT, vmValue(i));
        }
        else {
            SetError("Types do not match");
            return false;
        }
    }

    // Data containing strings will need to be "destroyed" when the stack unwinds.
    if (m_vm.DataTypes().ContainsString(type))
        AddInstruction(OP_REG_DESTRUCTOR, VTP_INT,  vmValue((vmInt)m_vm.StoreType(type)));

    return true;
}

bool TomBasicCompiler::CompileReturn() {
    if (!GetToken ())
        return false;

    if (m_inFunction) {
        if (UserPrototype().hasReturnVal) {
            vmValType type = UserPrototype().returnValType;

            // Generate code to compile and return value
            if (!CompileExpression())
                return false;
            if (!CompileConvert(type.RegisterType()))
                return false;

            // Basic values and pointers can be returned in the register
            if (!type.CanStoreInRegister()) {

                // Add instruction to move that data into temp data
                AddInstruction(OP_MOVE_TEMP, VTP_INT, vmValue((vmInt)m_vm.StoreType(type)));

                // Add return-from-function OP-code
                // Note: The 0 in the instruction value indicates that temp
                // data should NOT be freed on return (as we have just moved
                // the return value there.)
                AddInstruction(OP_RETURN_USER_FUNC, VTP_INT, vmValue(0));
            }
            else
                // Add return-from-function OP-code
                // Note: The 1 in the instruction value indicates that temp
                // data should be freed on return.
                AddInstruction(OP_RETURN_USER_FUNC, VTP_INT, vmValue(1));
        }
        else
            AddInstruction(OP_RETURN_USER_FUNC, VTP_INT, vmValue(1));
    }
    else {
        // Add "return from Gosub" op-code
        AddInstruction (OP_RETURN, VTP_INT, vmValue ());
    }

    return true;
}

std::string TomBasicCompiler::GetUserFunctionName(int index) {
    std::map<int,std::string>::iterator i = m_userFunctionReverseIndex.find(index);
    return i == m_userFunctionReverseIndex.end() ? (std::string)"???" : i->second;
}

bool TomBasicCompiler::InternalCompileBindCode() {

    // Evaluate code handle
    if (!CompileExpression())
        return false;

    if (!CompileConvert(VTP_INT))
        return false;

    // Add bindcode op-code
    AddInstruction(OP_BINDCODE, VTP_INT, vmValue());

    return true;
}

bool TomBasicCompiler::CompileBindCode() {

    // Skip "bindcode" keyword
    if (!GetToken())
        return false;

    // Combine bind code
    return InternalCompileBindCode();
}

bool TomBasicCompiler::CompileExec() {

    // Skip "exec" keyword
    if (!GetToken())
        return false;

    // Check if explicit code block specified
    if (!AtSeparatorOrSpecial())
        if (!InternalCompileBindCode())
            return false;

    // Add exec op-code
    AddInstruction(OP_EXEC, VTP_INT, vmValue());

    return true;
}

compRollbackPoint TomBasicCompiler::GetRollbackPoint() {
    compRollbackPoint r;

    // Get virtual machine rollback info
    r.vmRollback = m_vm.GetRollbackPoint();

    // Get compiler rollback info
    r.runtimeFunctionCount = m_runtimeFunctions.size();

    return r;
}

void TomBasicCompiler::Rollback(compRollbackPoint rollbackPoint) {

    // Rollback virtual machine
    m_vm.Rollback(rollbackPoint.vmRollback);

    // Rollback compiler

    // Remove new labels
    // (We can detect these as any labels with an offset past the instruction
    // count stored in the rollback).
    compLabelMap::iterator i = m_labels.begin();
    while (i != m_labels.end()) {
        if (i->second.m_offset >= rollbackPoint.vmRollback.instructionCount) {

            // This should compile, but doesn't: i = m_labels.erase(i);
            // The following is a workaround.
            compLabelMap::iterator temp = i;
            i++;
            m_labels.erase(temp);
        }
        else
            i++;
    }

    // Remove global function name->index records
    // (Can detect them as any global function with an invalid function index)
    std::map<std::string,int>::iterator j = m_globalUserFunctionIndex.begin();
    while (j != m_globalUserFunctionIndex.end()) {
        if (j->second >= rollbackPoint.vmRollback.functionCount) {
            std::map<std::string,int>::iterator temp = j;
            j++;
            m_globalUserFunctionIndex.erase(temp);
        }
        else
            j++;
    }

    // Remove function index->name records (used for debugging)
    std::map<int,std::string>::iterator k = m_userFunctionReverseIndex.begin();
    while (k != m_userFunctionReverseIndex.end()) {
        if (k->first >= rollbackPoint.vmRollback.functionCount) {
            std::map<int,std::string>::iterator temp = k;
            k++;
            m_userFunctionReverseIndex.erase(temp);
        }
        else
            k++;
    }

    // Remove runtime functions
    m_runtimeFunctions.resize(rollbackPoint.runtimeFunctionCount);

    std::map<std::string,int>::iterator l = m_runtimeFunctionIndex.begin();
    while (l != m_runtimeFunctionIndex.end()) {
        if (l->second >= rollbackPoint.runtimeFunctionCount) {
            std::map<std::string,int>::iterator temp = l;
            l++;
            m_runtimeFunctionIndex.erase(temp);
        }
        else
            l++;
    }
}

#ifdef VM_STATE_STREAMING
void TomBasicCompiler::StreamOut(std::ostream& stream) {

    // Stream out VM state
    m_vm.StreamOut(stream);
    
#ifdef STREAM_NAMES
    // Stream out constants
    std::string name;
    for (
        compConstantMap::iterator iConst = m_programConstants.begin();
        iConst != m_programConstants.end();
        iConst++) {
        std::string name = iConst->first;
        WriteString(stream, name);
        iConst->second.StreamOut(stream);
    }
    name = "";
    WriteString(stream, name);

    // Stream out labels
    for (
        compLabelMap::iterator iLabel = m_labels.begin();
        iLabel != m_labels.end();
        iLabel++) {
        std::string name = iLabel->first;
        WriteString(stream, name);
        iLabel->second.StreamOut(stream);
    }
    name = "";
    WriteString(stream, name);

    // Stream out user function/subroutine names
    for (   std::map<std::string,int>::iterator i = m_globalUserFunctionIndex.begin();
            i != m_globalUserFunctionIndex.end();
            i++) {
        std::string name = i->first;
        WriteString(stream, name);
        WriteLong(stream, i->second);
    }
    name = "";
    WriteString(stream, name);

    // Stream out runtime functions
    // Note that strictly speaking these aren't "names", but because they are
    // required when name information is present, and not required when it is
    // absent, we are bundling them into the same #ifdef
    WriteLong(stream, m_runtimeFunctions.size());
    for (int i = 0; i < m_runtimeFunctions.size(); i++)
        m_runtimeFunctions[i].StreamOut(stream);

    // Stream out runtime function names
    for(    std::map<std::string,int>::iterator i = m_runtimeFunctionIndex.begin();
            i != m_runtimeFunctionIndex.end();
            i++) {
        std::string name = i->first;
        WriteString(stream, name);
        WriteLong(stream, i->second);
    }
    name = "";
    WriteString(stream, name);

#endif
}

bool TomBasicCompiler::StreamIn(std::istream& stream) {

    // Unload any plugins
    m_plugins.Clear();

    // Clear current program (if any)
    New();

    // Stream in VM state
    if (!m_vm.StreamIn(stream)) {
        SetError(m_vm.GetError());
        return false;
    }

#ifdef STREAM_NAMES
    // Stream in constant names
    std::string name = ReadString(stream);
    while (name != "") {

        // Read constant details
        compConstant constant;
        constant.StreamIn(stream);

        // Store constant
        m_programConstants[name] = constant;

        // Next constant
        name = ReadString(stream);
    }

    // Stream in label names
    name = ReadString(stream);
    while (name != "") {

        // Read label details
        compLabel label;
        label.StreamIn(stream);

        // Store label
        m_labels[name] = label;
        m_labelIndex[label.m_offset] = name;
        
        // Next label
        name = ReadString(stream);
    }

    // Stream in user function/subroutine names
    name = ReadString(stream);
    while (name != "") {

        // Read function details
        int index = ReadLong(stream);

        // Store function index
        m_globalUserFunctionIndex[name] = index;

        // Next function
        name = ReadString(stream);
    }

    // Stream in runtime functions
    // Note that strictly speaking these aren't "names", but because they are
    // required when name information is present, and not required when it is
    // absent, we are bundling them into the same #ifdef
    int count = ReadLong(stream);
    m_runtimeFunctions.resize(count);
    for (int i = 0; i < count; i++)
        m_runtimeFunctions[i].StreamIn(stream);

    name = ReadString(stream);
    while (name != "") {

        // Read runtime function details
        int index = ReadLong(stream);

        // Store runtime function index
        m_runtimeFunctionIndex[name] = index;

        // Next function
        name = ReadString(stream);
    }
#endif

    return true;
}
#endif



