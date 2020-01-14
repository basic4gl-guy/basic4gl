//---------------------------------------------------------------------------
/*  Created 7-Sep-2003: Thomas Mulgrew

    Code format and storage.
*/
                                                                         

#pragma hdrstop

#include "vmCode.h"

//---------------------------------------------------------------------------

#ifndef _MSC_VER
#pragma package(smart_init)
#endif
std::string vmOpCodeName (vmOpCode code) {
    switch (code) {
    case OP_NOP:                return  "NOP";
    case OP_END:                return  "END";
    case OP_LOAD_CONST:         return  "LOAD_CONST";
    case OP_LOAD_VAR:           return  "LOAD_VAR";
    case OP_LOAD_LOCAL_VAR:     return  "LOAD_LOCAL_VAR";
    case OP_DEREF:              return  "DEREF";
    case OP_ADD_CONST:          return  "ADD_CONST";
    case OP_ARRAY_INDEX:        return  "ARRAY_INDEX";
    case OP_PUSH:               return  "PUSH";
    case OP_POP:                return  "POP";
    case OP_SAVE:               return  "SAVE";
    case OP_COPY:               return  "COPY";
    case OP_DECLARE:            return  "DECLARE";
    case OP_DECLARE_LOCAL:      return  "DECLARE_LOCAL";
    case OP_TIMESHARE:          return  "TIMESHARE";
    case OP_FREE_TEMP:          return  "FREE_TEMP";
    case OP_ALLOC:              return  "ALLOC";
    case OP_DATA_READ:          return  "DATA_READ";
    case OP_DATA_RESET:         return  "DATA_RESET";
    case OP_SAVE_PARAM:         return  "SAVE_PARAM";
    case OP_SAVE_PARAM_PTR:     return  "SAVE_PARAM_PTR";
    case OP_COPY_USER_STACK:    return  "COPY_USER_STACK";
    case OP_MOVE_TEMP:          return  "MOVE_TEMP";
    case OP_CHECK_PTR:          return  "CHECK_PTR";
    case OP_CHECK_PTRS:         return  "CHECK_PTRS";
    case OP_REG_DESTRUCTOR:     return  "REG_DESTRUCTOR";
	case OP_SWAP:				return  "SWAP";
	case OP_COND_TIMESHARE:		return	"COND_TIMESHARE";
    case OP_JUMP:               return  "JUMP";
    case OP_JUMP_TRUE:          return  "JUMP_TRUE";
    case OP_JUMP_FALSE:         return  "JUMP_FALSE";
    case OP_CALL_FUNC:          return  "CALL_FUNC";
    case OP_CALL:               return  "CALL";
    case OP_RETURN:             return  "RETURN";
    case OP_CALL_DLL:           return  "CALL_DLL";
    case OP_CREATE_USER_FRAME:  return  "CREATE_USER_FRAME";
    case OP_CALL_USER_FUNC:     return  "CALL_USER_FUNC";
    case OP_RETURN_USER_FUNC:   return  "RETURN_USER_FUNC";
    case OP_NO_VALUE_RETURNED:  return  "NO_VALUE_RETURNED";
    case OP_BINDCODE:           return  "BINDCODE";
    case OP_EXEC:               return  "EXEC";
    case OP_CREATE_RUNTIME_FRAME: return "CREATE_RUNTIME_FRAME";
	case OP_CREATE_FUNC_PTR_FRAME:	return "CREATE_FUNC_PTR_FRAME";
	case OP_CHECK_FUNC_PTR:		return	"CHECK_FUNC_PTR";
    case OP_END_CALLBACK:       return  "END_CALLBACK";
    case OP_OP_NEG:             return  "OP_NEG";
    case OP_OP_PLUS:            return  "OP_PLUS";
    case OP_OP_MINUS:           return  "OP_MINUS";
    case OP_OP_TIMES:           return  "OP_TIMES";
    case OP_OP_DIV:             return  "OP_DIV";
    case OP_OP_MOD:             return  "OP_MOD";
    case OP_OP_NOT:             return  "OP_NOT";
    case OP_OP_EQUAL:           return  "OP_EQUAL";
    case OP_OP_NOT_EQUAL:       return  "OP_NOT_EQUAL";
    case OP_OP_GREATER:         return  "OP_GREATER";
    case OP_OP_GREATER_EQUAL:   return  "OP_GREATER_EQUAL";
    case OP_OP_LESS:            return  "OP_LESS";
    case OP_OP_LESS_EQUAL:      return  "OP_LESS_EQUAL";
    case OP_CONV_INT_REAL:      return  "CONV_INT_REAL";
    case OP_CONV_INT_STRING:    return  "CONV_INT_STRING";
    case OP_CONV_REAL_STRING:   return  "CONV_REAL_STRING";
    case OP_CONV_INT_REAL2:     return  "CONV_INT_REAL2";
    case OP_CONV_INT_STRING2:   return  "CONV_INT_STRING2";
    case OP_CONV_REAL_STRING2:  return  "CONV_REAL_STRING2";
    case OP_OP_OR:              return  "OP_OR";
    case OP_OP_AND:             return  "OP_AND";
    case OP_OP_XOR:             return  "OP_XOR";
    case OP_CONV_REAL_INT:      return  "CONV_REAL_INT";
    case OP_CONV_REAL_INT2:     return  "CONV_REAL_INT2";
    case OP_RUN:                return  "OP_RUN";
    case OP_BREAKPT:            return  "OP_BREAKPT";
    default:                    return  "???";
    };
}

////////////////////////////////////////////////////////////////////////////////
// vmInstruction
#ifdef VM_STATE_STREAMING
void vmInstruction::StreamOut (std::ostream& stream) const {
    WriteByte (stream, m_opCode);
    WriteByte (stream, m_type);
    m_value.StreamOut (stream);
#ifdef STREAM_NAMES
    WriteLong (stream, m_sourceLine);
    WriteShort (stream, m_sourceChar);
#endif
}

void vmInstruction::StreamIn (std::istream& stream) {
    m_opCode        = ReadByte (stream);
    m_type          = ReadByte (stream);
    m_value.StreamIn (stream);
#ifdef STREAM_NAMES
    m_sourceLine    = ReadLong (stream);
    m_sourceChar    = ReadShort (stream);
#endif
}
#endif

