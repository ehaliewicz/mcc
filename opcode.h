#ifndef OPCODE_H
#define OPCODE_H


typedef enum {
  INT,
  CHAR
} runtime_type;


#define OPCODES						\
  X(LIT, 1) X(DROP, 0)					\
  X(POPENV, 1) X(PUSHENV, 1)				\
  X(CALL, 1) X(RET, 0)					\
  X(MKENV, 1) X(EXTEND_ENV, 0)				\
  X(POP_EXTEND_ENV, 0)					\
  X(CHKTYPE_POPENV, 2) X(DROPENV, 0)			\
  X(ADD, 0) X(SUB, 0)   X(NEG, 0)			\
  X(MUL, 0) X(DIV, 0)   X(MOD, 0)			\
  X(IOR, 0) X(XOR, 0)   X(AND, 0)			\
  X(NOT, 0) X(BRA, 1)   X(BEQ, 1)			\
  X(CMP, 0) X(TST, 0) X(BGT, 1)				\
  X(PRINT, 0) X(PUTC, 0)				\
  X(READ, 0) X(READC, 0)				\
  X(HALT, 0)





typedef enum {
#define X(nm, op) nm,
  OPCODES
#undef X
} opcode;

char* opcode_names[] = {
#define X(nm, op) #nm,
  OPCODES
#undef X
};

int has_operand[] = {
#define X(nm, op) op,
  OPCODES
#undef X
};


#endif
