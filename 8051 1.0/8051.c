#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>

#define IROM_SIZE 0x10000
#define XROM_SIZE 0x10000
#define IRAM_SIZE 0x100
#define XRAM_SIZE 0x10000
#define CLRMOD    1

/* ─────────────────────────────────────────────
   TABLES
───────────────────────────────────────────── */
const unsigned char cycles[256] = {
//0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x00
  2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x10
  2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20
  2, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x30
  2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40
  2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x50
  2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60
  2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x70
  2, 2, 2, 2, 4, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x80
  2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x90
  2, 2, 1, 1, 4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xA0
  2, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0xB0
  2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xC0
  2, 2, 1, 1, 1, 2, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, // 0xD0
  2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xE0
  2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  // 0xF0
};

const unsigned char instSize[256] = {
//0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
  1, 2, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x00
  3, 2, 3, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x10
  3, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x20
  3, 2, 1, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x30
  2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x40
  2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x50
  2, 2, 2, 3, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x60
  2, 2, 2, 1, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x70
  2, 2, 2, 1, 1, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0x80
  3, 2, 2, 1, 2, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0x90
  2, 2, 2, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, // 0xA0
  2, 2, 2, 1, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, // 0xB0
  2, 2, 2, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xC0
  2, 2, 2, 1, 1, 3, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, // 0xD0
  1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, // 0xE0
  1, 2, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1  // 0xF0
};

/* ─────────────────────────────────────────────
   CPU STATE
───────────────────────────────────────────── */
struct CPU {
    unsigned char IROM[IROM_SIZE];
    unsigned char XROM[XROM_SIZE];
    unsigned char IRAM[IRAM_SIZE];
    unsigned char XRAM[XRAM_SIZE];
} CPU;

/* ─────────────────────────────────────────────
   HELPERS
───────────────────────────────────────────── */
void clr(void) { if(CLRMOD) system("cls"); }
void delay(void) { return; }

/* PSW bit macros — all read/write through IRAM[0xD0] */
#define PSW        (CPU.IRAM[0xD0])
#define CY         ((PSW >> 7) & 1)
#define AC         ((PSW >> 6) & 1)
#define OV         ((PSW >> 2) & 1)
#define SET_CY(v)  (PSW = (PSW & 0x7F) | ((v) ? 0x80 : 0))
#define SET_AC(v)  (PSW = (PSW & 0xBF) | ((v) ? 0x40 : 0))
#define SET_OV(v)  (PSW = (PSW & 0xFB) | ((v) ? 0x04 : 0))
#define SET_P()    do { uint8_t _p = CPU.IRAM[0xE0]; \
                        _p ^= _p>>4; _p ^= _p>>2; _p ^= _p>>1; \
                        PSW = (PSW & 0xFE) | (_p & 1); } while(0)

/* Active register bank: RS1:RS0 = PSW[4:3] */
#define Rn(n)      (CPU.IRAM[((PSW >> 3) & 0x03) * 8 + (n)])

/* DPTR */
#define DPTR       ((uint16_t)(CPU.IRAM[0x83] << 8 | CPU.IRAM[0x82]))
#define SET_DPTR(v) do { CPU.IRAM[0x83]=((v)>>8)&0xFF; \
                         CPU.IRAM[0x82]=(v)&0xFF; } while(0)

/* Stack */
#define SP         (CPU.IRAM[0x81])
#define PUSH8(v)   do { SP++; CPU.IRAM[SP]=(uint8_t)(v); } while(0)
#define POP8(dst)  do { (dst)=CPU.IRAM[SP]; SP--; } while(0)

/* Bit-addressable: bit addr → byte in IRAM, mask */
#define BIT_BYTE(b)  (CPU.IRAM[((b)<0x80) ? (0x20+(b)/8) : ((b)&0xF8)])
#define BIT_MASK(b)  (1u << ((b) & 7))
#define GET_BIT(b)   ((BIT_BYTE(b) >> ((b)&7)) & 1)
#define SET_BIT(b,v) do { if(v) BIT_BYTE(b)|=BIT_MASK(b); \
                          else  BIT_BYTE(b)&=~BIT_MASK(b); } while(0)

/* ─────────────────────────────────────────────
   DECODED INSTRUCTION
───────────────────────────────────────────── */
typedef enum {
    OP_NOP,
    OP_MOV,         /* *dst = src_val                          */
    OP_MOVX_RD,     /* A = XRAM[addr16]                        */
    OP_MOVX_WR,     /* XRAM[addr16] = A                        */
    OP_MOVC,        /* A = IROM/XROM[base + A]                 */
    OP_ADD,
    OP_ADDC,
    OP_SUBB,
    OP_INC_PTR,     /* (*dst)++  — no flags except DPTR        */
    OP_DEC_PTR,
    OP_INC_DPTR,
    OP_MUL,
    OP_DIV,
    OP_DA,
    OP_ANL,
    OP_ORL,
    OP_XRL,
    OP_ANL_C_BIT,   /* C &= bit (or /bit)                      */
    OP_ORL_C_BIT,   /* C |= bit (or /bit)                      */
    OP_CLR_BIT,
    OP_SETB_BIT,
    OP_CPL_BIT,
    OP_MOV_BIT_C,   /* bit = C                                  */
    OP_MOV_C_BIT,   /* C = bit                                  */
    OP_RL,  OP_RLC,
    OP_RR,  OP_RRC,
    OP_SWAP,
    OP_XCH,
    OP_XCHD,
    OP_PUSH, OP_POP,
    OP_SJMP,
    OP_AJMP,
    OP_LJMP,
    OP_JMP_ADPTR,
    OP_JC,  OP_JNC,
    OP_JZ,  OP_JNZ,
    OP_JB,  OP_JNB, OP_JBC,
    OP_CJNE,
    OP_DJNZ,
    OP_ACALL,
    OP_LCALL,
    OP_RET,
    OP_RETI,
    OP_HALT
} Operation;

typedef struct {
    Operation op;
    uint8_t  *dst;       /* pointer into IRAM (destination)   */
    uint8_t  *src;       /* pointer into IRAM (source)        */
    uint8_t   imm8;      /* immediate / operand value         */
    uint16_t  imm16;     /* 16-bit immediate or address       */
    uint8_t   bit_addr;  /* bit address for bit ops           */
    uint8_t   inv;       /* 1 = inverted bit (/ prefix)       */
    int8_t    offset;    /* signed relative offset            */
    uint8_t   use_imm;   /* 1 = use imm8 instead of *src      */
    uint16_t  movc_base; /* IROM or XROM base for MOVC        */
    uint8_t   movc_xrom; /* 1 = MOVC reads from XROM          */
} DecodedInstr;

/* ─────────────────────────────────────────────
   FETCH
───────────────────────────────────────────── */
static void fetch(uint16_t *PC,
                  uint8_t  *op,
                  uint8_t  *b1,
                  uint8_t  *b2)
{
    *op = CPU.IROM[*PC];
    *b1 = CPU.IROM[*PC + 1];
    *b2 = CPU.IROM[*PC + 2];
    *PC += instSize[*op];   /* PC now points at NEXT instruction */
}

/* ─────────────────────────────────────────────
   DECODE
   PC is already advanced (= address of next instr)
───────────────────────────────────────────── */
static DecodedInstr decode(uint8_t op, uint8_t b1, uint8_t b2, uint16_t PC)
{
    DecodedInstr d;
    memset(&d, 0, sizeof(d));

    /* Convenience: pointer to accumulator */
    uint8_t *A = &CPU.IRAM[0xE0];

    switch(op) {

    /* ── NOP ──────────────────────────────── */
    case 0x00: d.op = OP_NOP; break;

    /* ── AJMP ─────────────────────────────── */
    case 0x01: case 0x21: case 0x41: case 0x61:
    case 0x81: case 0xA1: case 0xC1: case 0xE1:
        d.op    = OP_AJMP;
        /* upper 5 bits from already-advanced PC */
        d.imm16 = (PC & 0xF800)
                | ((op & 0xE0) << 3)
                | b1;
        break;

    /* ── LJMP ─────────────────────────────── */
    case 0x02:
        d.op    = OP_LJMP;
        d.imm16 = ((uint16_t)b1 << 8) | b2;
        break;

    /* ── RR A ─────────────────────────────── */
    case 0x03: d.op = OP_RR; d.dst = A; break;

    /* ── INC A ────────────────────────────── */
    case 0x04: d.op = OP_INC_PTR; d.dst = A; break;

    /* ── INC direct ───────────────────────── */
    case 0x05: d.op = OP_INC_PTR; d.dst = &CPU.IRAM[b1]; break;

    /* ── INC @R0/R1 ───────────────────────── */
    case 0x06: d.op = OP_INC_PTR; d.dst = &CPU.IRAM[Rn(0)]; break;
    case 0x07: d.op = OP_INC_PTR; d.dst = &CPU.IRAM[Rn(1)]; break;

    /* ── INC Rn ───────────────────────────── */
    case 0x08: case 0x09: case 0x0A: case 0x0B:
    case 0x0C: case 0x0D: case 0x0E: case 0x0F:
        d.op = OP_INC_PTR; d.dst = &Rn(op & 7); break;

    /* ── JBC bit, offset ──────────────────── */
    case 0x10:
        d.op       = OP_JBC;
        d.bit_addr = b1;
        d.offset   = (int8_t)b2;
        break;

    /* ── ACALL ────────────────────────────── */
    case 0x11: case 0x31: case 0x51: case 0x71:
    case 0x91: case 0xB1: case 0xD1: case 0xF1:
        d.op    = OP_ACALL;
        d.imm16 = (PC & 0xF800)
                | ((op & 0xE0) << 3)
                | b1;
        break;

    /* ── LCALL ────────────────────────────── */
    case 0x12:
        d.op    = OP_LCALL;
        d.imm16 = ((uint16_t)b1 << 8) | b2;
        break;

    /* ── RRC A ────────────────────────────── */
    case 0x13: d.op = OP_RRC; d.dst = A; break;

    /* ── DEC A ────────────────────────────── */
    case 0x14: d.op = OP_DEC_PTR; d.dst = A; break;

    /* ── DEC direct ───────────────────────── */
    case 0x15: d.op = OP_DEC_PTR; d.dst = &CPU.IRAM[b1]; break;

    /* ── DEC @R0/R1 ───────────────────────── */
    case 0x16: d.op = OP_DEC_PTR; d.dst = &CPU.IRAM[Rn(0)]; break;
    case 0x17: d.op = OP_DEC_PTR; d.dst = &CPU.IRAM[Rn(1)]; break;

    /* ── DEC Rn ───────────────────────────── */
    case 0x18: case 0x19: case 0x1A: case 0x1B:
    case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        d.op = OP_DEC_PTR; d.dst = &Rn(op & 7); break;

    /* ── JB bit, offset ───────────────────── */
    case 0x20:
        d.op       = OP_JB;
        d.bit_addr = b1;
        d.offset   = (int8_t)b2;
        break;

    /* ── RET ──────────────────────────────── */
    case 0x22: d.op = OP_RET; break;

    /* ── RL A ─────────────────────────────── */
    case 0x23: d.op = OP_RL; d.dst = A; break;

    /* ── ADD A, #imm ──────────────────────── */
    case 0x24:
        d.op = OP_ADD; d.dst = A;
        d.use_imm = 1; d.imm8 = b1; break;

    /* ── ADD A, direct ────────────────────── */
    case 0x25:
        d.op = OP_ADD; d.dst = A; d.src = &CPU.IRAM[b1]; break;

    /* ── ADD A, @R0/R1 ────────────────────── */
    case 0x26: d.op=OP_ADD; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x27: d.op=OP_ADD; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── ADD A, Rn ────────────────────────── */
    case 0x28: case 0x29: case 0x2A: case 0x2B:
    case 0x2C: case 0x2D: case 0x2E: case 0x2F:
        d.op=OP_ADD; d.dst=A; d.src=&Rn(op&7); break;

    /* ── JNB bit, offset ──────────────────── */
    case 0x30:
        d.op       = OP_JNB;
        d.bit_addr = b1;
        d.offset   = (int8_t)b2;
        break;

    /* ── RETI ─────────────────────────────── */
    case 0x32: d.op = OP_RETI; break;

    /* ── RLC A ────────────────────────────── */
    case 0x33: d.op = OP_RLC; d.dst = A; break;

    /* ── ADDC A, #imm ─────────────────────── */
    case 0x34:
        d.op=OP_ADDC; d.dst=A; d.use_imm=1; d.imm8=b1; break;

    /* ── ADDC A, direct ───────────────────── */
    case 0x35:
        d.op=OP_ADDC; d.dst=A; d.src=&CPU.IRAM[b1]; break;

    /* ── ADDC A, @R0/R1 ───────────────────── */
    case 0x36: d.op=OP_ADDC; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x37: d.op=OP_ADDC; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── ADDC A, Rn ───────────────────────── */
    case 0x38: case 0x39: case 0x3A: case 0x3B:
    case 0x3C: case 0x3D: case 0x3E: case 0x3F:
        d.op=OP_ADDC; d.dst=A; d.src=&Rn(op&7); break;

    /* ── JC offset ────────────────────────── */
    case 0x40: d.op=OP_JC;  d.offset=(int8_t)b1; break;

    /* ── AJMP (page 2) already covered above  */

    /* ── ORL direct, A ───────────────────── */
    case 0x42:
        d.op=OP_ORL; d.dst=&CPU.IRAM[b1]; d.src=A; break;

    /* ── ORL direct, #imm ────────────────── */
    case 0x43:
        d.op=OP_ORL; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;

    /* ── ORL A, #imm ─────────────────────── */
    case 0x44:
        d.op=OP_ORL; d.dst=A; d.use_imm=1; d.imm8=b1; break;

    /* ── ORL A, direct ───────────────────── */
    case 0x45:
        d.op=OP_ORL; d.dst=A; d.src=&CPU.IRAM[b1]; break;

    /* ── ORL A, @R0/R1 ───────────────────── */
    case 0x46: d.op=OP_ORL; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x47: d.op=OP_ORL; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── ORL A, Rn ───────────────────────── */
    case 0x48: case 0x49: case 0x4A: case 0x4B:
    case 0x4C: case 0x4D: case 0x4E: case 0x4F:
        d.op=OP_ORL; d.dst=A; d.src=&Rn(op&7); break;

    /* ── JNC offset ──────────────────────── */
    case 0x50: d.op=OP_JNC; d.offset=(int8_t)b1; break;

    /* ── ANL direct, A ───────────────────── */
    case 0x52:
        d.op=OP_ANL; d.dst=&CPU.IRAM[b1]; d.src=A; break;

    /* ── ANL direct, #imm ────────────────── */
    case 0x53:
        d.op=OP_ANL; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;

    /* ── ANL A, #imm ─────────────────────── */
    case 0x54:
        d.op=OP_ANL; d.dst=A; d.use_imm=1; d.imm8=b1; break;

    /* ── ANL A, direct ───────────────────── */
    case 0x55:
        d.op=OP_ANL; d.dst=A; d.src=&CPU.IRAM[b1]; break;

    /* ── ANL A, @R0/R1 ───────────────────── */
    case 0x56: d.op=OP_ANL; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x57: d.op=OP_ANL; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── ANL A, Rn ───────────────────────── */
    case 0x58: case 0x59: case 0x5A: case 0x5B:
    case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        d.op=OP_ANL; d.dst=A; d.src=&Rn(op&7); break;

    /* ── JZ offset ───────────────────────── */
    case 0x60: d.op=OP_JZ; d.offset=(int8_t)b1; break;

    /* ── XRL direct, A ───────────────────── */
    case 0x62:
        d.op=OP_XRL; d.dst=&CPU.IRAM[b1]; d.src=A; break;

    /* ── XRL direct, #imm ────────────────── */
    case 0x63:
        d.op=OP_XRL; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;

    /* ── XRL A, #imm ─────────────────────── */
    case 0x64:
        d.op=OP_XRL; d.dst=A; d.use_imm=1; d.imm8=b1; break;

    /* ── XRL A, direct ───────────────────── */
    case 0x65:
        d.op=OP_XRL; d.dst=A; d.src=&CPU.IRAM[b1]; break;

    /* ── XRL A, @R0/R1 ───────────────────── */
    case 0x66: d.op=OP_XRL; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x67: d.op=OP_XRL; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── XRL A, Rn ───────────────────────── */
    case 0x68: case 0x69: case 0x6A: case 0x6B:
    case 0x6C: case 0x6D: case 0x6E: case 0x6F:
        d.op=OP_XRL; d.dst=A; d.src=&Rn(op&7); break;

    /* ── JNZ offset ──────────────────────── */
    case 0x70: d.op=OP_JNZ; d.offset=(int8_t)b1; break;

    /* ── ORL C, bit ──────────────────────── */
    case 0x72:
        d.op=OP_ORL_C_BIT; d.bit_addr=b1; d.inv=0; break;

    /* ── JMP @A+DPTR ─────────────────────── */
    case 0x73: d.op=OP_JMP_ADPTR; break;

    /* ── MOV A, #imm ─────────────────────── */
    case 0x74:
        d.op=OP_MOV; d.dst=A; d.use_imm=1; d.imm8=b1; break;

    /* ── MOV direct, #imm ────────────────── */
    case 0x75:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;

    /* ── MOV @R0/R1, #imm ────────────────── */
    case 0x76:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(0)]; d.use_imm=1; d.imm8=b1; break;
    case 0x77:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(1)]; d.use_imm=1; d.imm8=b1; break;

    /* ── MOV Rn, #imm ────────────────────── */
    case 0x78: case 0x79: case 0x7A: case 0x7B:
    case 0x7C: case 0x7D: case 0x7E: case 0x7F:
        d.op=OP_MOV; d.dst=&Rn(op&7); d.use_imm=1; d.imm8=b1; break;

    /* ── SJMP offset  (0x80 0xFE = HALT) ─── */
    case 0x80:
        if(b1 == 0xFE) { d.op = OP_HALT; }
        else           { d.op = OP_SJMP; d.offset = (int8_t)b1; }
        break;

    /* ── ANL C, bit ──────────────────────── */
    case 0x82:
        d.op=OP_ANL_C_BIT; d.bit_addr=b1; d.inv=0; break;

    /* ── MOVC A, @A+PC (reads IROM) ─────── */
    case 0x83:
        d.op=OP_MOVC; d.movc_base=PC; d.movc_xrom=0; break;

    /* ── DIV AB ──────────────────────────── */
    case 0x84: d.op=OP_DIV; break;

    /* ── MOV direct(dst), direct(src) ────── */
    /* NOTE: encoding is [85][src][dst]        */
    case 0x85:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b2]; d.src=&CPU.IRAM[b1]; break;

    /* ── MOV direct, @R0/R1 ──────────────── */
    case 0x86:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x87:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── MOV direct, Rn ──────────────────── */
    case 0x88: case 0x89: case 0x8A: case 0x8B:
    case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=&Rn(op&7); break;

    /* ── MOV DPTR, #imm16 ────────────────── */
    case 0x90:
        d.op=OP_MOV; d.imm16=((uint16_t)b1<<8)|b2;
        /* handled specially in execute — dst==NULL means DPTR */
        d.dst=NULL; d.use_imm=1; break;

    /* ── MOV bit, C ──────────────────────── */
    case 0x92:
        d.op=OP_MOV_BIT_C; d.bit_addr=b1; break;

    /* ── MOVC A, @A+DPTR (reads XROM) ────── */
    case 0x93:
        d.op=OP_MOVC; d.movc_base=DPTR; d.movc_xrom=1; break;

    /* ── SUBB A, #imm ────────────────────── */
    case 0x94:
        d.op=OP_SUBB; d.dst=A; d.use_imm=1; d.imm8=b1; break;

    /* ── SUBB A, direct ──────────────────── */
    case 0x95:
        d.op=OP_SUBB; d.dst=A; d.src=&CPU.IRAM[b1]; break;

    /* ── SUBB A, @R0/R1 ──────────────────── */
    case 0x96: d.op=OP_SUBB; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x97: d.op=OP_SUBB; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── SUBB A, Rn ──────────────────────── */
    case 0x98: case 0x99: case 0x9A: case 0x9B:
    case 0x9C: case 0x9D: case 0x9E: case 0x9F:
        d.op=OP_SUBB; d.dst=A; d.src=&Rn(op&7); break;

    /* ── ORL C, /bit ─────────────────────── */
    case 0xA0:
        d.op=OP_ORL_C_BIT; d.bit_addr=b1; d.inv=1; break;

    /* ── MOV C, bit ──────────────────────── */
    case 0xA2:
        d.op=OP_MOV_C_BIT; d.bit_addr=b1; break;

    /* ── INC DPTR ────────────────────────── */
    case 0xA3: d.op=OP_INC_DPTR; break;

    /* ── MUL AB ──────────────────────────── */
    case 0xA4: d.op=OP_MUL; break;

    /* ── MOV @R0/R1, direct ──────────────── */
    case 0xA6:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(0)]; d.src=&CPU.IRAM[b1]; break;
    case 0xA7:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(1)]; d.src=&CPU.IRAM[b1]; break;

    /* ── MOV Rn, direct ──────────────────── */
    case 0xA8: case 0xA9: case 0xAA: case 0xAB:
    case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        d.op=OP_MOV; d.dst=&Rn(op&7); d.src=&CPU.IRAM[b1]; break;

    /* ── ANL C, /bit ─────────────────────── */
    case 0xB0:
        d.op=OP_ANL_C_BIT; d.bit_addr=b1; d.inv=1; break;

    /* ── CPL bit ─────────────────────────── */
    case 0xB2:
        d.op=OP_CPL_BIT; d.bit_addr=b1; break;

    /* ── CPL C ───────────────────────────── */
    case 0xB3:
        d.op=OP_CPL_BIT; d.bit_addr=0xFF; /* 0xFF = carry flag */ break;

    /* ── CJNE A, #imm, offset ────────────── */
    case 0xB4:
        d.op=OP_CJNE; d.dst=A;
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;

    /* ── CJNE A, direct, offset ──────────── */
    case 0xB5:
        d.op=OP_CJNE; d.dst=A;
        d.src=&CPU.IRAM[b1]; d.offset=(int8_t)b2; break;

    /* ── CJNE @R0/R1, #imm, offset ──────── */
    case 0xB6:
        d.op=OP_CJNE; d.dst=&CPU.IRAM[Rn(0)];
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;
    case 0xB7:
        d.op=OP_CJNE; d.dst=&CPU.IRAM[Rn(1)];
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;

    /* ── CJNE Rn, #imm, offset ───────────── */
    case 0xB8: case 0xB9: case 0xBA: case 0xBB:
    case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        d.op=OP_CJNE; d.dst=&Rn(op&7);
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;

    /* ── PUSH direct ─────────────────────── */
    case 0xC0:
        d.op=OP_PUSH; d.src=&CPU.IRAM[b1]; break;

    /* ── CLR bit ─────────────────────────── */
    case 0xC2:
        d.op=OP_CLR_BIT; d.bit_addr=b1; break;

    /* ── CLR C ───────────────────────────── */
    case 0xC3:
        d.op=OP_CLR_BIT; d.bit_addr=0xFF; break;

    /* ── SWAP A ──────────────────────────── */
    case 0xC4: d.op=OP_SWAP; d.dst=A; break;

    /* ── XCH A, direct ───────────────────── */
    case 0xC5:
        d.op=OP_XCH; d.dst=A; d.src=&CPU.IRAM[b1]; break;

    /* ── XCH A, @R0/R1 ───────────────────── */
    case 0xC6: d.op=OP_XCH; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0xC7: d.op=OP_XCH; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── XCH A, Rn ───────────────────────── */
    case 0xC8: case 0xC9: case 0xCA: case 0xCB:
    case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        d.op=OP_XCH; d.dst=A; d.src=&Rn(op&7); break;

    /* ── POP direct ──────────────────────── */
    case 0xD0:
        d.op=OP_POP; d.dst=&CPU.IRAM[b1]; break;

    /* ── SETB bit ────────────────────────── */
    case 0xD2:
        d.op=OP_SETB_BIT; d.bit_addr=b1; break;

    /* ── SETB C ──────────────────────────── */
    case 0xD3:
        d.op=OP_SETB_BIT; d.bit_addr=0xFF; break;

    /* ── DA A ────────────────────────────── */
    case 0xD4: d.op=OP_DA; d.dst=A; break;

    /* ── DJNZ direct, offset ─────────────── */
    case 0xD5:
        d.op=OP_DJNZ; d.dst=&CPU.IRAM[b1]; d.offset=(int8_t)b2; break;

    /* ── XCHD A, @R0/R1 ──────────────────── */
    case 0xD6: d.op=OP_XCHD; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0xD7: d.op=OP_XCHD; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── DJNZ Rn, offset ─────────────────── */
    case 0xD8: case 0xD9: case 0xDA: case 0xDB:
    case 0xDC: case 0xDD: case 0xDE: case 0xDF:
        d.op=OP_DJNZ; d.dst=&Rn(op&7); d.offset=(int8_t)b1; break;

    /* ── MOVX A, @DPTR ───────────────────── */
    case 0xE0:
        d.op=OP_MOVX_RD; d.imm16=DPTR; break;

    /* ── MOVX A, @R0/R1 ──────────────────── */
    case 0xE2: d.op=OP_MOVX_RD; d.imm16=Rn(0); break;
    case 0xE3: d.op=OP_MOVX_RD; d.imm16=Rn(1); break;

    /* ── CLR A ───────────────────────────── */
    case 0xE4:
        d.op=OP_MOV; d.dst=A; d.use_imm=1; d.imm8=0; break;

    /* ── MOV A, direct ───────────────────── */
    case 0xE5:
        d.op=OP_MOV; d.dst=A; d.src=&CPU.IRAM[b1]; break;

    /* ── MOV A, @R0/R1 ───────────────────── */
    case 0xE6: d.op=OP_MOV; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0xE7: d.op=OP_MOV; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    /* ── MOV A, Rn ───────────────────────── */
    case 0xE8: case 0xE9: case 0xEA: case 0xEB:
    case 0xEC: case 0xED: case 0xEE: case 0xEF:
        d.op=OP_MOV; d.dst=A; d.src=&Rn(op&7); break;

    /* ── MOVX @DPTR, A ───────────────────── */
    case 0xF0:
        d.op=OP_MOVX_WR; d.imm16=DPTR; break;

    /* ── MOVX @R0/R1, A ──────────────────── */
    case 0xF2: d.op=OP_MOVX_WR; d.imm16=Rn(0); break;
    case 0xF3: d.op=OP_MOVX_WR; d.imm16=Rn(1); break;

    /* ── CPL A ───────────────────────────── */
    case 0xF4:
        d.op=OP_MOV; d.dst=A; d.use_imm=1; d.imm8=~(*A); break;

    /* ── MOV direct, A ───────────────────── */
    case 0xF5:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=A; break;

    /* ── MOV @R0/R1, A ───────────────────── */
    case 0xF6: d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(0)]; d.src=A; break;
    case 0xF7: d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(1)]; d.src=A; break;

    /* ── MOV Rn, A ───────────────────────── */
    case 0xF8: case 0xF9: case 0xFA: case 0xFB:
    case 0xFC: case 0xFD: case 0xFE: case 0xFF:
        d.op=OP_MOV; d.dst=&Rn(op&7); d.src=A; break;

    /* ── 0xA5 reserved = HALT ────────────── */
    case 0xA5:
    default:
        d.op = OP_HALT; break;
    }
    return d;
}

/* ─────────────────────────────────────────────
   EXECUTE
   Returns 0 = continue, 1 = halt
───────────────────────────────────────────── */
static int execute(DecodedInstr *d, uint16_t *PC)
{
    uint8_t *A = &CPU.IRAM[0xE0];
    uint8_t  sv;        /* source value (resolved once) */

    /* Resolve source value */
    sv = d->use_imm ? d->imm8 : (d->src ? *d->src : 0);

    switch(d->op) {

    case OP_NOP: break;

    case OP_HALT: return 1;

    /* ── MOV ──────────────────────────────── */
    case OP_MOV:
        if(d->dst == NULL) {        /* MOV DPTR, #imm16 */
            SET_DPTR(d->imm16);
        } else {
            *d->dst = sv;
            if(d->dst == A) SET_P();
        }
        break;

    /* ── MOVX read ────────────────────────── */
    case OP_MOVX_RD:
        *A = CPU.XRAM[d->imm16];
        SET_P();
        break;

    /* ── MOVX write ───────────────────────── */
    case OP_MOVX_WR:
        CPU.XRAM[d->imm16] = *A;
        break;

    /* ── MOVC ─────────────────────────────── */
    case OP_MOVC:
        if(d->movc_xrom)
            *A = CPU.XROM[(uint16_t)(d->movc_base + *A)];
        else
            *A = CPU.IROM[(uint16_t)(d->movc_base + *A)];
        SET_P();
        break;

    /* ── ADD ──────────────────────────────── */
    case OP_ADD: {
        uint16_t r = *d->dst + sv;
        SET_AC((*d->dst & 0xF) + (sv & 0xF) > 0xF);
        SET_OV(((*d->dst ^ sv) & 0x80)==0 && ((*d->dst ^ r) & 0x80));
        SET_CY(r > 0xFF);
        *d->dst = r & 0xFF;
        SET_P();
        break; }

    /* ── ADDC ─────────────────────────────── */
    case OP_ADDC: {
        uint8_t  cy = CY;
        uint16_t r  = *d->dst + sv + cy;
        SET_AC((*d->dst & 0xF) + (sv & 0xF) + cy > 0xF);
        SET_OV(((*d->dst ^ sv) & 0x80)==0 && ((*d->dst ^ r) & 0x80));
        SET_CY(r > 0xFF);
        *d->dst = r & 0xFF;
        SET_P();
        break; }

    /* ── SUBB ─────────────────────────────── */
    case OP_SUBB: {
        uint8_t cy = CY;
        uint8_t a  = *d->dst;
        SET_AC((a & 0xF) < (sv & 0xF) + cy);
        SET_OV(((a ^ sv) & 0x80) && ((a ^ (a - sv - cy)) & 0x80));
        SET_CY(a < (uint16_t)sv + cy);
        *d->dst = a - sv - cy;
        SET_P();
        break; }

    /* ── INC (no flags except DPTR) ──────── */
    case OP_INC_PTR:
        (*d->dst)++;
        if(d->dst == A) SET_P();
        break;

    /* ── DEC ──────────────────────────────── */
    case OP_DEC_PTR:
        (*d->dst)--;
        if(d->dst == A) SET_P();
        break;

    /* ── INC DPTR ────────────────────────── */
    case OP_INC_DPTR:
        SET_DPTR(DPTR + 1);
        break;

    /* ── MUL AB ──────────────────────────── */
    case OP_MUL: {
        uint16_t r = (uint16_t)*A * CPU.IRAM[0xF0];
        *A             = r & 0xFF;
        CPU.IRAM[0xF0] = (r >> 8) & 0xFF;
        SET_CY(0);
        SET_OV(CPU.IRAM[0xF0] != 0);
        SET_P();
        break; }

    /* ── DIV AB ──────────────────────────── */
    case OP_DIV:
        if(CPU.IRAM[0xF0] == 0) {
            SET_OV(1); SET_CY(0);
        } else {
            uint8_t q = *A / CPU.IRAM[0xF0];
            uint8_t r = *A % CPU.IRAM[0xF0];
            *A             = q;
            CPU.IRAM[0xF0] = r;
            SET_OV(0); SET_CY(0);
        }
        SET_P();
        break;

    /* ── DA A ────────────────────────────── */
    case OP_DA: {
        uint8_t a = *A;
        if((a & 0x0F) > 9 || AC) a += 0x06;
        if(a > 0x9F || CY)       { a += 0x60; SET_CY(1); }
        *A = a;
        SET_P();
        break; }

    /* ── ANL / ORL / XRL ─────────────────── */
    case OP_ANL:
        *d->dst &= sv;
        if(d->dst == A) SET_P();
        break;
    case OP_ORL:
        *d->dst |= sv;
        if(d->dst == A) SET_P();
        break;
    case OP_XRL:
        *d->dst ^= sv;
        if(d->dst == A) SET_P();
        break;

    /* ── ANL C, bit / /bit ───────────────── */
    case OP_ANL_C_BIT: {
        uint8_t bv = GET_BIT(d->bit_addr);
        if(d->inv) bv = !bv;
        if(!bv) SET_CY(0);
        break; }

    /* ── ORL C, bit / /bit ───────────────── */
    case OP_ORL_C_BIT: {
        uint8_t bv = GET_BIT(d->bit_addr);
        if(d->inv) bv = !bv;
        if(bv) SET_CY(1);
        break; }

    /* ── CLR bit / C ─────────────────────── */
    case OP_CLR_BIT:
        if(d->bit_addr == 0xFF) SET_CY(0);
        else SET_BIT(d->bit_addr, 0);
        break;

    /* ── SETB bit / C ────────────────────── */
    case OP_SETB_BIT:
        if(d->bit_addr == 0xFF) SET_CY(1);
        else SET_BIT(d->bit_addr, 1);
        break;

    /* ── CPL bit / C ─────────────────────── */
    case OP_CPL_BIT:
        if(d->bit_addr == 0xFF) SET_CY(!CY);
        else SET_BIT(d->bit_addr, !GET_BIT(d->bit_addr));
        break;

    /* ── MOV bit, C ──────────────────────── */
    case OP_MOV_BIT_C:
        SET_BIT(d->bit_addr, CY);
        break;

    /* ── MOV C, bit ──────────────────────── */
    case OP_MOV_C_BIT:
        SET_CY(GET_BIT(d->bit_addr));
        break;

    /* ── RL A ────────────────────────────── */
    case OP_RL:
        *d->dst = (*d->dst << 1) | (*d->dst >> 7);
        SET_P();
        break;

    /* ── RLC A ───────────────────────────── */
    case OP_RLC: {
        uint8_t new_cy = (*d->dst >> 7) & 1;
        *d->dst = (*d->dst << 1) | CY;
        SET_CY(new_cy);
        SET_P();
        break; }

    /* ── RR A ────────────────────────────── */
    case OP_RR:
        *d->dst = (*d->dst >> 1) | (*d->dst << 7);
        SET_P();
        break;

    /* ── RRC A ───────────────────────────── */
    case OP_RRC: {
        uint8_t new_cy = *d->dst & 1;
        *d->dst = (*d->dst >> 1) | (CY << 7);
        SET_CY(new_cy);
        SET_P();
        break; }

    /* ── SWAP A ──────────────────────────── */
    case OP_SWAP:
        *d->dst = (*d->dst >> 4) | (*d->dst << 4);
        break;

    /* ── XCH A, src ──────────────────────── */
    case OP_XCH: {
        uint8_t t = *d->dst;
        *d->dst   = *d->src;
        *d->src   = t;
        SET_P();
        break; }

    /* ── XCHD A, @Ri ─────────────────────── */
    case OP_XCHD: {
        uint8_t t   = *d->dst & 0x0F;
        *d->dst     = (*d->dst & 0xF0) | (*d->src & 0x0F);
        *d->src     = (*d->src & 0xF0) | t;
        SET_P();
        break; }

    /* ── PUSH ────────────────────────────── */
    case OP_PUSH:
        PUSH8(*d->src);
        break;

    /* ── POP ─────────────────────────────── */
    case OP_POP:
        POP8(*d->dst);
        break;

    /* ── SJMP ────────────────────────────── */
    case OP_SJMP:
        *PC += (int16_t)d->offset;
        break;

    /* ── AJMP / LJMP ─────────────────────── */
    case OP_AJMP:
    case OP_LJMP:
        *PC = d->imm16;
        break;

    /* ── JMP @A+DPTR ─────────────────────── */
    case OP_JMP_ADPTR:
        *PC = DPTR + *A;
        break;

    /* ── Conditional jumps ───────────────── */
    case OP_JC:   if( CY)      *PC += (int16_t)d->offset; break;
    case OP_JNC:  if(!CY)      *PC += (int16_t)d->offset; break;
    case OP_JZ:   if(*A == 0)  *PC += (int16_t)d->offset; break;
    case OP_JNZ:  if(*A != 0)  *PC += (int16_t)d->offset; break;

    /* ── JB / JNB / JBC ─────────────────── */
    case OP_JB:
        if( GET_BIT(d->bit_addr)) *PC += (int16_t)d->offset;
        break;
    case OP_JNB:
        if(!GET_BIT(d->bit_addr)) *PC += (int16_t)d->offset;
        break;
    case OP_JBC:
        if(GET_BIT(d->bit_addr)) {
            SET_BIT(d->bit_addr, 0);
            *PC += (int16_t)d->offset;
        }
        break;

    /* ── CJNE ────────────────────────────── */
    case OP_CJNE:
        SET_CY(*d->dst < sv);
        if(*d->dst != sv) *PC += (int16_t)d->offset;
        break;

    /* ── DJNZ ────────────────────────────── */
    case OP_DJNZ:
        (*d->dst)--;
        if(*d->dst != 0) *PC += (int16_t)d->offset;
        break;

    /* ── ACALL / LCALL ───────────────────── */
    case OP_ACALL:
    case OP_LCALL:
        PUSH8(*PC & 0xFF);
        PUSH8((*PC >> 8) & 0xFF);
        *PC = d->imm16;
        break;

    /* ── RET / RETI ──────────────────────── */
    case OP_RET:
    case OP_RETI: {
        uint8_t hi, lo;
        POP8(hi); POP8(lo);
        *PC = ((uint16_t)hi << 8) | lo;
        break; }

    default: return 1;
    }
    return 0;
}

/* ─────────────────────────────────────────────
   REGISTER DUMP
───────────────────────────────────────────── */
static void reg_dump(uint16_t PC)
{
    printf("\n====== REGISTER DUMP ======\n");
    printf("PC  : %04XH\n", PC);
    printf("A   : %02XH    B   : %02XH\n",
           CPU.IRAM[0xE0], CPU.IRAM[0xF0]);
    printf("PSW : %02XH  (CY=%d AC=%d OV=%d P=%d)\n",
           PSW, CY, AC, OV, PSW & 1);
    printf("SP  : %02XH    DPTR: %04XH\n",
           CPU.IRAM[0x81], DPTR);
    uint8_t bank = (PSW >> 3) & 3;
    printf("Bank%d: ", bank);
    for(int i = 0; i < 8; i++)
        printf("R%d=%02XH ", i, CPU.IRAM[bank*8+i]);
    printf("\n===========================\n");
}

/* ─────────────────────────────────────────────
   RUN
───────────────────────────────────────────── */
int run(void)
{
    uint16_t PC = 0;
    uint8_t  op, b1, b2;

    /* Load IROM from file if it exists */
    FILE *fp = fopen("IROM.bin", "rb");
    if(fp) {
        fread(CPU.IROM, 1, IROM_SIZE, fp);
        fclose(fp);
    }
    /* Load XROM from file if it exists */
    fp = fopen("XROM.bin", "rb");
    if(fp) {
        fread(CPU.XROM, 1, XROM_SIZE, fp);
        fclose(fp);
    }

    printf("START ADDRESS (hex): ");
    scanf("%hX", &PC);
    getchar();
    clr();

    printf("Running from %04XH ...\n", PC);

    while(1) {
        /* ── FETCH ── */
        fetch(&PC, &op, &b1, &b2);

        /* ── DECODE ── */
        DecodedInstr d = decode(op, b1, b2, PC);

        /* ── EXECUTE ── */
        if(execute(&d, &PC)) break;

        delay();
    }

    reg_dump(PC);
    printf("\nPress Enter to return to menu...");
    getchar();
    clr();
    return 0;
}

/* ─────────────────────────────────────────────
   HELPER: SFR name lookup (for assembler/unasm)
───────────────────────────────────────────── */
void address(char *word, int *valid)
{
    (void)valid;
    struct { const char *name; const char *addr; } sfrs[] = {
        {"P0","80H"},  {"SP","81H"},   {"DPL","82H"},  {"DPH","83H"},
        {"PCON","87H"},{"TCON","88H"}, {"TMOD","89H"}, {"TL0","8AH"},
        {"TL1","8BH"}, {"TH0","8CH"},  {"TH1","8DH"},  {"P1","90H"},
        {"SCON","98H"},{"SBUF","99H"}, {"P2","0A0H"},  {"IE","0A8H"},
        {"P3","0B0H"}, {"IP","0B8H"},  {"PSW","0D0H"}, {"ACC","0E0H"},
        {"B","0F0H"},  {NULL,NULL}
    };
    for(int k = 0; sfrs[k].name; k++)
        if(strcmp(word, sfrs[k].name) == 0) {
            strcpy(word, sfrs[k].addr);
            return;
        }
}

int extractVal(char *strVal, int *type, int *valid)
{
    char *rVal = (char*)malloc(strlen(strVal) + 1);
    int k, val;
    for(k = 0; k < (int)strlen(strVal); k++) {
        char ch = strVal[k];
        if(isdigit(ch) || ch=='#' || ch=='@' || ch=='/' || ch=='H' || ch=='D'
           || (ch>='A' && ch<='F') || (ch>='a' && ch<='f')) continue;
        else { *type = -1; free(rVal); return 0; }
    }
    if(strVal[0]!='#' && strVal[0]!='@' && strVal[0]!='/') {
        *type = 0;
        strcpy(rVal, strVal);
    } else {
        for(k = 0; k < (int)strlen(strVal)-1; k++)
            rVal[k] = strVal[k+1];
        rVal[k] = '\0';
        if(strVal[0]=='#') *type=1;
        else if(strVal[0]=='@') *type=2;
        else if(strVal[0]=='/') *type=3;
    }
    char ch = rVal[0];
    if((ch>='A' && ch<='F') || (ch>='a' && ch<='f')) {
        *type = -1; free(rVal); return 0;
    }
    if(rVal[strlen(rVal)-1]=='H')
        sscanf(rVal, "%xH", &val);
    else if(rVal[strlen(rVal)-1]=='D')
        sscanf(rVal, "%dD", &val);
    else
        sscanf(rVal, "%x", &val);
    free(rVal);
    return val;
}

/* ─────────────────────────────────────────────
   ASSEMBLER
───────────────────────────────────────────── */
int assembler(void)
{
    int i, j, mnemonicnum, ch, mnemonicch, byteno;
    int bytes=1, type[3]={0,0,0}, valid=1;
    char input[100] = {};
    unsigned char *MEM = CPU.IROM;


    printf("ORIGIN : ");
    scanf("%X", &i);
    getchar();
    clr();

    char mnemonic[4][16] = {"","","",""};
    unsigned char opcodes[3];

    /* Open/create IROM.bin for random-write */
    FILE *fp = fopen("IROM.bin", "rb+");
    if(!fp) {
        fp = fopen("IROM.bin", "wb");
        if(fp) {
            unsigned char zeros[IROM_SIZE];
            memset(zeros, 0, IROM_SIZE);
            fwrite(zeros, 1, IROM_SIZE, fp);
            fclose(fp);
            fp = fopen("IROM.bin", "rb+");
        }
    }

    while(1) {
        clr();
        printf("%04X>> ", i);
        valid = 1;
        fgets(input, 100, stdin);
        input[strlen(input)-1] = '\0';
        mnemonicnum = 0; ch = 0; mnemonicch = 0; opcodes[0] = 0;

        if(strncmp(input, ".", 1) == 0) break;

        for(int k = 0; k < 4; k++) mnemonic[k][0] = '\0';

        while(input[ch] != '\0') {
            if(input[ch] != ' ' && input[ch] != ',') {
                mnemonic[mnemonicnum][mnemonicch++] = input[ch];
            } else if((input[ch]==' ' && mnemonicnum==0) ||
                      (input[ch]==',' && mnemonicnum<3)) {
                mnemonic[mnemonicnum][mnemonicch] = '\0';
                address(mnemonic[mnemonicnum], &valid);
                mnemonicnum++;
                mnemonicch = 0;
            } else valid = 0;
            ch++;
        }
        mnemonic[mnemonicnum][mnemonicch] = '\0';
        address(mnemonic[mnemonicnum], &valid);
        j = i;

        /* ── ENCODE ── (same logic as before, unchanged) ── */
        if(strcmp(mnemonic[0],"NOP")==0 && mnemonicnum==0){
            opcodes[0]=0x00; bytes=1;}
        else if(strcmp(mnemonic[0],"AJMP")==0 && mnemonicnum==1){
            unsigned int a = extractVal(mnemonic[1],&type[0],&valid);
            if((a>>11)==((i+2)>>11) && type[0]!=-1){
                opcodes[0]=0x01|(((a>>8)&0x07)<<5);
                opcodes[1]=a&0xFF; bytes=2;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"LJMP")==0 && mnemonicnum==1){
            int a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){opcodes[0]=0x02;opcodes[1]=(a>>8)&0xff;opcodes[2]=a&0xff;bytes=3;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"RR")==0 && strcmp(mnemonic[1],"A")==0 && mnemonicnum==1){
            opcodes[0]=0x03;bytes=1;}
        else if(strcmp(mnemonic[0],"INC")==0 && mnemonicnum==1){
            unsigned char a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            if(strcmp(mnemonic[1],"A")==0){opcodes[0]=0x04;bytes=1;}
            else if(type[0]==0){opcodes[0]=0x05;opcodes[1]=a;bytes=2;}
            else if(strcmp(mnemonic[1],"@R0")==0){opcodes[0]=0x06;bytes=1;}
            else if(strcmp(mnemonic[1],"@R1")==0){opcodes[0]=0x07;bytes=1;}
            else if(mnemonic[1][0]=='R'){int x;sscanf(mnemonic[1],"R%d",&x);
                (x<8&&x>=0)?(opcodes[0]=0x08+x):(valid=0);bytes=1;}
            else if(strcmp(mnemonic[1],"DPTR")==0){opcodes[0]=0xa3;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"JBC")==0 && mnemonicnum==2){
            int a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            int b=0; if(valid) b=extractVal(mnemonic[2],&type[1],&valid);
            if(type[0]==0&&type[1]==0&&b-(i+3)>-128&&b-(i+3)<=127){
                opcodes[0]=0x10;opcodes[1]=a;opcodes[2]=b-(i+3);bytes=3;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"ACALL")==0 && mnemonicnum==1){
            unsigned int a=extractVal(mnemonic[1],&type[0],&valid);
            if((a>>11)==((i+2)>>11)&&type[0]!=-1){
                opcodes[0]=0x11|(((a>>8)&0x07)<<5);opcodes[1]=a&0xFF;bytes=2;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"LCALL")==0 && mnemonicnum==1){
            unsigned int a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){opcodes[0]=0x12;opcodes[1]=(a>>8)&0xff;opcodes[2]=a&0xff;bytes=3;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"RRC")==0&&strcmp(mnemonic[1],"A")==0&&mnemonicnum==1){
            opcodes[0]=0x13;bytes=1;}
        else if(strcmp(mnemonic[0],"DEC")==0&&mnemonicnum==1){
            unsigned int a=extractVal(mnemonic[1],&type[0],&valid);
            if(strcmp(mnemonic[1],"A")==0){opcodes[0]=0x14;bytes=1;}
            else if(type[0]==0){opcodes[0]=0x15;opcodes[1]=a;bytes=2;}
            else if(strcmp(mnemonic[1],"@R0")==0){opcodes[0]=0x16;bytes=1;}
            else if(strcmp(mnemonic[1],"@R1")==0){opcodes[0]=0x17;bytes=1;}
            else if(mnemonic[1][0]=='R'){int x;sscanf(mnemonic[1],"R%d",&x);
                (x<8&&x>=0)?(opcodes[0]=0x18+x):(valid=0);bytes=1;}}
        else if(strcmp(mnemonic[0],"JB")==0&&mnemonicnum==2){
            int a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            int b=0;if(valid) b=extractVal(mnemonic[2],&type[1],&valid);
            if(type[0]==0&&type[1]==0&&b-(i+3)>-128&&b-(i+3)<=127){
                opcodes[0]=0x20;opcodes[1]=a;opcodes[2]=b-(i+3);bytes=3;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"RET")==0){opcodes[0]=0x22;bytes=1;}
        else if(strcmp(mnemonic[0],"RL")==0&&strcmp(mnemonic[1],"A")==0&&mnemonicnum==1){
            opcodes[0]=0x23;bytes=1;}
        else if(strcmp(mnemonic[0],"ADD")==0&&strcmp(mnemonic[1],"A")==0&&mnemonicnum==2){
            short int a=extractVal(mnemonic[2],&type[0],&valid);
            if(type[0]==1){opcodes[0]=0x24;opcodes[1]=a&0xff;bytes=2;}
            else if(type[0]==0){opcodes[0]=0x25;opcodes[1]=a&0xff;bytes=2;}
            else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0x26;bytes=1;}
            else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0x27;bytes=1;}
            else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                (x<8&&x>=0)?(opcodes[0]=0x28+x):(valid=0);bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"JNB")==0&&mnemonicnum==2){
            int a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            int b=0;if(valid) b=extractVal(mnemonic[2],&type[1],&valid);
            if(type[0]==0&&type[1]==0&&b-(i+3)>-128&&b-(i+3)<=127){
                opcodes[0]=0x30;opcodes[1]=a;opcodes[2]=b-(i+3);bytes=3;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"RETI")==0){opcodes[0]=0x32;bytes=1;}
        else if(strcmp(mnemonic[0],"RLC")==0&&strcmp(mnemonic[1],"A")==0&&mnemonicnum==1){
            opcodes[0]=0x33;bytes=1;}
        else if(strcmp(mnemonic[0],"ADDC")==0&&strcmp(mnemonic[1],"A")==0&&mnemonicnum==2){
            short int a=extractVal(mnemonic[2],&type[0],&valid);
            if(type[0]==1){opcodes[0]=0x34;opcodes[1]=a&0xff;bytes=2;}
            else if(type[0]==0){opcodes[0]=0x35;opcodes[1]=a&0xff;bytes=2;}
            else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0x36;bytes=1;}
            else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0x37;bytes=1;}
            else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                (x<8&&x>=0)?(opcodes[0]=0x38+x):(valid=0);bytes=1;}}
        else if(strcmp(mnemonic[0],"JC")==0&&mnemonicnum==1){
            int a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0&&a-(i+2)>-128&&a-(i+2)<=127){
                opcodes[0]=0x40;opcodes[1]=a-(i+2);bytes=2;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"ORL")==0&&mnemonicnum==2){
            short int a,b;
            a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){
                b=extractVal(mnemonic[2],&type[1],&valid);
                if(strcmp(mnemonic[2],"A")==0){opcodes[0]=0x42;opcodes[1]=a&0xff;bytes=2;}
                else if(type[1]==1){opcodes[0]=0x43;opcodes[1]=a&0xff;opcodes[2]=b&0xff;bytes=3;}}
            else if(strcmp(mnemonic[1],"A")==0){
                a=extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==1){opcodes[0]=0x44;opcodes[1]=a&0xff;bytes=2;}
                else if(type[0]==0){opcodes[0]=0x45;opcodes[1]=a&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0x46;bytes=1;}
                else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0x47;bytes=1;}
                else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                    (x<8&&x>=0)?(opcodes[0]=0x48+x):(valid=0);bytes=1;}}
            else if(strcmp(mnemonic[1],"C")==0){
                a=0xff&extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==0){opcodes[0]=0x72;opcodes[1]=a;bytes=2;}
                else if(type[0]==3){opcodes[0]=0xa0;opcodes[1]=a;bytes=2;}}}
        else if(strcmp(mnemonic[0],"JNC")==0&&mnemonicnum==1){
            int a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0&&a-(i+2)>-128&&a-(i+2)<=127){
                opcodes[0]=0x50;opcodes[1]=a-(i+2);bytes=2;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"ANL")==0&&mnemonicnum==2){
            short int a,b;
            a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){
                b=extractVal(mnemonic[2],&type[1],&valid);
                if(strcmp(mnemonic[2],"A")==0){opcodes[0]=0x52;opcodes[1]=a&0xff;bytes=2;}
                else if(type[1]==1){opcodes[0]=0x53;opcodes[1]=a&0xff;opcodes[2]=b&0xff;bytes=3;}
                else valid=0;}
            else if(strcmp(mnemonic[1],"A")==0){
                a=extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==1){opcodes[0]=0x54;opcodes[1]=a&0xff;bytes=2;}
                else if(type[0]==0){opcodes[0]=0x55;opcodes[1]=a&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0x56;bytes=1;}
                else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0x57;bytes=1;}
                else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                    (x<8&&x>=0)?(opcodes[0]=0x58+x):(valid=0);bytes=1;}}
            else if(strcmp(mnemonic[1],"C")==0){
                a=0xff&extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==0){opcodes[0]=0x82;opcodes[1]=a;bytes=2;}
                else if(type[0]==3){opcodes[0]=0xb0;opcodes[1]=a;bytes=2;}}
            else valid=0;}
        else if(strcmp(mnemonic[0],"JZ")==0&&mnemonicnum==1){
            int a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0&&a-(i+2)>-128&&a-(i+2)<=127){
                opcodes[0]=0x60;opcodes[1]=a-(i+2);bytes=2;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"XRL")==0&&mnemonicnum==2){
            short int a,b;
            a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){
                b=extractVal(mnemonic[2],&type[1],&valid);
                if(strcmp(mnemonic[2],"A")==0){opcodes[0]=0x62;opcodes[1]=a&0xff;bytes=2;}
                else if(type[1]==1){opcodes[0]=0x63;opcodes[1]=a&0xff;opcodes[2]=b&0xff;bytes=3;}}
            else if(strcmp(mnemonic[1],"A")==0){
                a=extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==1){opcodes[0]=0x64;opcodes[1]=a&0xff;bytes=2;}
                else if(type[0]==0){opcodes[0]=0x65;opcodes[1]=a&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0x66;bytes=1;}
                else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0x67;bytes=1;}
                else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                    (x<8&&x>=0)?(opcodes[0]=0x68+x):(valid=0);bytes=1;}
                else valid=0;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"JNZ")==0&&mnemonicnum==1){
            int a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0&&a-(i+2)>-128&&a-(i+2)<=127){
                opcodes[0]=0x70;opcodes[1]=a-(i+2);bytes=2;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"JMP")==0&&mnemonicnum==1){
            if(strcmp(mnemonic[1],"@A+DPTR")==0){opcodes[0]=0x73;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"MOV")==0&&mnemonicnum==2){
            short int a,b;
            a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){
                b=extractVal(mnemonic[2],&type[1],&valid);
                if(strcmp(mnemonic[2],"A")==0){opcodes[0]=0xf5;opcodes[1]=a&0xff;bytes=2;}
                else if(type[1]==1){opcodes[0]=0x75;opcodes[1]=a&0xff;opcodes[2]=b&0xff;bytes=3;}
                else if(type[1]==0){opcodes[0]=0x85;opcodes[1]=b&0xff;opcodes[2]=a&0xff;bytes=3;}
                else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0x86;opcodes[1]=a&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0x87;opcodes[1]=a&0xff;bytes=2;}
                else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                    (x<8&&x>=0)?(opcodes[0]=0x88+x):(valid=0);
                    if(valid){opcodes[1]=a&0xff;}bytes=2;}
                else if(strcmp(mnemonic[2],"C")==0){opcodes[0]=0x92;opcodes[1]=a&0xff;bytes=2;}}
            else if(strcmp(mnemonic[1],"A")==0){
                a=extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==1){opcodes[0]=0x74;opcodes[1]=a&0xff;bytes=2;}
                else if(type[0]==0){opcodes[0]=0xe5;opcodes[1]=a&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0xe6;bytes=1;}
                else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0xe7;bytes=1;}
                else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                    (x<8&&x>=0)?(opcodes[0]=0xe8+x):(valid=0);bytes=1;}
                else valid=0;}
            else if(strcmp(mnemonic[1],"@R0")==0){
                b=extractVal(mnemonic[2],&type[1],&valid);
                if(type[1]==1){opcodes[0]=0x76;opcodes[1]=b&0xff;bytes=2;}
                else if(type[1]==0){opcodes[0]=0xa6;opcodes[1]=b&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"A")==0){opcodes[0]=0xf6;bytes=1;}}
            else if(strcmp(mnemonic[1],"@R1")==0){
                b=extractVal(mnemonic[2],&type[1],&valid);
                if(type[1]==1){opcodes[0]=0x77;opcodes[1]=b&0xff;bytes=2;}
                else if(type[1]==0){opcodes[0]=0xa7;opcodes[1]=b&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"A")==0){opcodes[0]=0xf7;bytes=1;}}
            else if(mnemonic[1][0]=='R'){
                int x; b=extractVal(mnemonic[2],&type[1],&valid);
                sscanf(mnemonic[1],"R%d",&x);
                if(x<8&&x>=0){
                    if(type[1]==1) opcodes[0]=0x78+x;
                    else if(type[1]==0) opcodes[0]=0xa8+x;
                    else if(strcmp(mnemonic[2],"A")==0){opcodes[0]=0xf8+x;bytes=1;}
                } else valid=0;
                if(valid&&strcmp(mnemonic[2],"A")!=0){opcodes[1]=b&0xff;bytes=2;}}
            else if(strcmp(mnemonic[1],"DPTR")==0){
                b=extractVal(mnemonic[2],&type[1],&valid);
                if(type[1]==1){opcodes[0]=0x90;opcodes[1]=(b>>8)&0xff;opcodes[2]=b&0xff;bytes=3;}}
            else if(strcmp(mnemonic[1],"C")==0){
                a=0xff&extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==0){opcodes[0]=0xA2;opcodes[1]=a;bytes=2;}}
            else valid=0;}
        else if(strcmp(mnemonic[0],"MOVC")==0){
            if(strcmp(mnemonic[1],"@A+PC")==0&&mnemonicnum==1){opcodes[0]=0x83;bytes=1;}
            else if(strcmp(mnemonic[1],"A")==0&&strcmp(mnemonic[2],"@A+DPTR")==0&&mnemonicnum==2){
                opcodes[0]=0x93;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"DIV")==0&&mnemonicnum==1){
            if(strcmp(mnemonic[1],"AB")==0){opcodes[0]=0x84;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"SUBB")==0&&strcmp(mnemonic[1],"A")==0&&mnemonicnum==2){
            short int a=extractVal(mnemonic[2],&type[0],&valid);
            if(type[0]==1){opcodes[0]=0x94;opcodes[1]=a&0xff;bytes=2;}
            else if(type[0]==0){opcodes[0]=0x95;opcodes[1]=a&0xff;bytes=2;}
            else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0x96;bytes=1;}
            else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0x97;bytes=1;}
            else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                (x<8&&x>=0)?(opcodes[0]=0x98+x):(valid=0);bytes=1;}}
        else if(strcmp(mnemonic[0],"MUL")==0&&mnemonicnum==1){
            if(strcmp(mnemonic[1],"AB")==0){opcodes[0]=0xa4;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"CPL")==0&&mnemonicnum==1){
            unsigned char a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            if(strcmp(mnemonic[1],"A")==0){opcodes[0]=0xf4;bytes=1;}
            else if(strcmp(mnemonic[1],"C")==0){opcodes[0]=0xb3;bytes=1;}
            else if(type[0]==0){opcodes[0]=0xb2;opcodes[1]=a;bytes=2;}}
        else if(strcmp(mnemonic[0],"CJNE")==0&&mnemonicnum==3){
            short int a,b;
            a=extractVal(mnemonic[2],&type[0],&valid);
            b=extractVal(mnemonic[3],&type[1],&valid);
            if(!(((type[0]==1)||(strcmp(mnemonic[1],"A")==0))&&type[1]==0&&
                 b-(i+3)>-128&&b-(i+3)<=127)) valid=0;
            if(valid){
                if(strcmp(mnemonic[1],"A")==0){
                    if(type[0]==1){opcodes[0]=0xb4;opcodes[1]=a&0xff;opcodes[2]=b-(i+3);bytes=3;}
                    else if(type[0]==0){opcodes[0]=0xb5;opcodes[1]=a&0xff;opcodes[2]=b-(i+3);bytes=3;}}
                else if(strcmp(mnemonic[1],"@R0")==0){opcodes[0]=0xb6;opcodes[1]=a&0xff;opcodes[2]=b-(i+3);bytes=3;}
                else if(strcmp(mnemonic[1],"@R1")==0){opcodes[0]=0xb7;opcodes[1]=a&0xff;opcodes[2]=b-(i+3);bytes=3;}
                else if(mnemonic[1][0]=='R'){int x;sscanf(mnemonic[1],"R%d",&x);
                    (x<8&&x>=0)?(opcodes[0]=0xb8+x):(valid=0);
                    opcodes[1]=a&0xff;opcodes[2]=b-(i+3);bytes=3;}}}
        else if(strcmp(mnemonic[0],"PUSH")==0&&mnemonicnum==1){
            unsigned char a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){opcodes[0]=0xc0;opcodes[1]=a;bytes=2;}}
        else if(strcmp(mnemonic[0],"CLR")==0&&mnemonicnum==1){
            unsigned char a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            if(strcmp(mnemonic[1],"A")==0){opcodes[0]=0xe4;bytes=1;}
            else if(strcmp(mnemonic[1],"C")==0){opcodes[0]=0xc3;bytes=1;}
            else if(type[0]==0){opcodes[0]=0xc2;opcodes[1]=a;bytes=2;}}
        else if(strcmp(mnemonic[0],"SWAP")==0&&mnemonicnum==1){
            if(strcmp(mnemonic[1],"A")==0){opcodes[0]=0xc4;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"XCH")==0&&mnemonicnum==2){
            if(strcmp(mnemonic[1],"A")==0){
                int a=extractVal(mnemonic[2],&type[0],&valid);
                if(type[0]==0){opcodes[0]=0xc5;opcodes[1]=a&0xff;bytes=2;}
                else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0xc6;bytes=1;}
                else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0xc7;bytes=1;}
                else if(mnemonic[2][0]=='R'){int x;sscanf(mnemonic[2],"R%d",&x);
                    (x<8&&x>=0)?(opcodes[0]=0xc8+x):(valid=0);bytes=1;}
                else valid=0;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"POP")==0&&mnemonicnum==1){
            unsigned char a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0){opcodes[0]=0xd0;opcodes[1]=a;bytes=2;}}
        else if(strcmp(mnemonic[0],"SETB")==0&&mnemonicnum==1){
            unsigned char a=0xff&extractVal(mnemonic[1],&type[0],&valid);
            if(strcmp(mnemonic[1],"C")==0){opcodes[0]=0xd3;bytes=1;}
            else if(type[0]==0){opcodes[0]=0xd2;opcodes[1]=a;bytes=2;}}
        else if(strcmp(mnemonic[0],"DA")==0&&mnemonicnum==1){
            if(strcmp(mnemonic[1],"A")==0){opcodes[0]=0xd4;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"DJNZ")==0&&mnemonicnum==2){
            short int a,b;
            a=extractVal(mnemonic[1],&type[0],&valid);
            b=extractVal(mnemonic[2],&type[1],&valid);
            if(type[0]==0){
                if(b-(i+3)>-128&&b-(i+3)<=127){
                    opcodes[0]=0xd5;opcodes[1]=a&0xff;opcodes[2]=b-(i+3);bytes=3;}
                else valid=0;}
            else if(mnemonic[1][0]=='R'){
                int x;sscanf(mnemonic[1],"R%d",&x);
                if(x<8&&x>=0&&type[1]==0&&b-(i+2)>-128&&b-(i+2)<=127){
                    opcodes[0]=0xd8+x;opcodes[1]=(b-(i+2))&0xff;bytes=2;}
                else valid=0;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"XCHD")==0&&strcmp(mnemonic[1],"A")==0&&mnemonicnum==2){
            if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0xd6;bytes=1;}
            else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0xd7;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"MOVX")==0&&mnemonicnum==2){
            if(strcmp(mnemonic[1],"A")==0){
                if(strcmp(mnemonic[2],"@DPTR")==0){opcodes[0]=0xe0;bytes=1;}
                else if(strcmp(mnemonic[2],"@R0")==0){opcodes[0]=0xe2;bytes=1;}
                else if(strcmp(mnemonic[2],"@R1")==0){opcodes[0]=0xe3;bytes=1;}
                else valid=0;}
            else if(strcmp(mnemonic[1],"@DPTR")==0&&strcmp(mnemonic[2],"A")==0){opcodes[0]=0xf0;bytes=1;}
            else if(strcmp(mnemonic[1],"@R0")==0&&strcmp(mnemonic[2],"A")==0){opcodes[0]=0xf2;bytes=1;}
            else if(strcmp(mnemonic[1],"@R1")==0&&strcmp(mnemonic[2],"A")==0){opcodes[0]=0xf3;bytes=1;}
            else valid=0;}
        else if(strcmp(mnemonic[0],"SJMP")==0&&mnemonicnum==1){
            int a=extractVal(mnemonic[1],&type[0],&valid);
            if(type[0]==0&&a-(i+2)>=-128&&a-(i+2)<=127){
                opcodes[0]=0x80;opcodes[1]=(a-(i+2))&0xff;bytes=2;}
            else valid=0;}
        else valid=0;

        if(strcmp(mnemonic[0],"NOP")!=0 && opcodes[0]==0) valid=0;

        if(valid) {
            for(byteno=0;byteno<bytes;byteno++){
                printf("%02X ", opcodes[byteno]);
                MEM[i++] = opcodes[byteno];
            }
            printf("\n");
            /* Write to bin file */
            if(fp){
                fseek(fp, j, SEEK_SET);
                fwrite(opcodes, 1, bytes, fp);
            }
        } else {
            printf("Invalid instruction!\n");
        }
        getchar();
    }

    if(fp) fclose(fp);
    clr();
    return 0;
}

/* ─────────────────────────────────────────────
   UNASSEMBLER
───────────────────────────────────────────── */

/* Mnemonic strings for disassembly display */
static const char *mnem_name[256] = {
    "NOP",  "AJMP", "LJMP", "RR",   "INC",  "INC",  "INC",  "INC",  // 00
    "INC",  "INC",  "INC",  "INC",  "INC",  "INC",  "INC",  "INC",  // 08
    "JBC",  "ACALL","LCALL","RRC",  "DEC",  "DEC",  "DEC",  "DEC",  // 10
    "DEC",  "DEC",  "DEC",  "DEC",  "DEC",  "DEC",  "DEC",  "DEC",  // 18
    "JB",   "AJMP", "RET",  "RL",   "ADD",  "ADD",  "ADD",  "ADD",  // 20
    "ADD",  "ADD",  "ADD",  "ADD",  "ADD",  "ADD",  "ADD",  "ADD",  // 28
    "JNB",  "ACALL","RETI", "RLC",  "ADDC", "ADDC", "ADDC", "ADDC",// 30
    "ADDC", "ADDC", "ADDC", "ADDC", "ADDC", "ADDC", "ADDC", "ADDC",// 38
    "JC",   "AJMP", "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL", // 40
    "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL", // 48
    "JNC",  "ACALL","ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL", // 50
    "ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL", // 58
    "JZ",   "AJMP", "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL", // 60
    "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL", // 68
    "JNZ",  "ACALL","ORL",  "JMP",  "MOV",  "MOV",  "MOV",  "MOV", // 70
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV", // 78
    "SJMP", "AJMP", "ANL",  "MOVC", "DIV",  "MOV",  "MOV",  "MOV", // 80
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV", // 88
    "MOV",  "ACALL","MOV",  "MOVC", "SUBB", "SUBB", "SUBB", "SUBB",// 90
    "SUBB", "SUBB", "SUBB", "SUBB", "SUBB", "SUBB", "SUBB", "SUBB",// 98
    "ORL",  "AJMP", "MOV",  "INC",  "MUL",  "???",  "MOV",  "MOV", // A0
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV", // A8
    "ANL",  "ACALL","CPL",  "CPL",  "CJNE", "CJNE", "CJNE", "CJNE",// B0
    "CJNE", "CJNE", "CJNE", "CJNE", "CJNE", "CJNE", "CJNE", "CJNE",// B8
    "PUSH", "AJMP", "CLR",  "CLR",  "SWAP", "XCH",  "XCH",  "XCH", // C0
    "XCH",  "XCH",  "XCH",  "XCH",  "XCH",  "XCH",  "XCH",  "XCH", // C8
    "POP",  "ACALL","SETB", "SETB", "DA",   "DJNZ", "XCHD", "XCHD",// D0
    "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ",// D8
    "MOVX", "AJMP", "MOVX", "MOVX", "CLR",  "MOV",  "MOV",  "MOV", // E0
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV", // E8
    "MOVX", "ACALL","MOVX", "MOVX", "CPL",  "MOV",  "MOV",  "MOV", // F0
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV"  // F8
};

static const char *dis_fmt[256] = {
    "", "addr11", "addr16", "A", "A", "d1", "@R0", "@R1", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn",  // 0x00
    "bit,rel", "addr11", "addr16", "A", "A", "d1", "@R0", "@R1", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn",  // 0x10
    "bit,rel", "addr11", "", "A", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0x20
    "bit,rel", "addr11", "", "A", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0x30
    "rel1", "addr11", "d1,A", "d1,#b2", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0x40
    "rel1", "addr11", "d1,A", "d1,#b2", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0x50
    "rel1", "addr11", "d1,A", "d1,#b2", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0x60
    "rel1", "addr11", "C,bit", "@A+DPTR_jmp", "A,#b1", "d1,#b2", "@R0,#b1", "@R1,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1",  // 0x70
    "rel1", "addr11", "C,bit", "@A+PC", "AB", "d2,d1", "d1,@R0", "d1,@R1", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn",  // 0x80
    "DPTR,#16", "addr11", "bit,C", "@A+DPTR", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0x90
    "C,/bit", "addr11", "C,bit", "DPTR", "AB", "???", "@R0,d1", "@R1,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1",  // 0xa0
    "C,/bit", "addr11", "bit", "C", "A,#b1,rel", "A,d1,rel", "@R0,#b1,rel", "@R1,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel",  // 0xb0
    "d1", "addr11", "bit", "C", "A", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0xc0
    "d1", "addr11", "bit", "C", "A", "d1,rel", "A,@R0", "A,@R1", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel",  // 0xd0
    "A,@DPTR", "addr11", "A,@R0", "A,@R1", "A", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",  // 0xe0
    "@DPTR,A", "addr11", "@R0,A", "@R1,A", "A", "d1,A", "@R0,A", "@R1,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A",  // 0xf0
};

static void disasm_operands(uint8_t op, uint8_t b1, uint8_t b2, uint16_t next_pc, char *buf)
{
    const char *f = dis_fmt[op];
    int8_t off;
    uint16_t target;
    buf[0] = '\0';

    if(strcmp(f,"")==0) return;

    /* Simple fixed strings */
    if(strcmp(f,"A")==0)           { sprintf(buf,"A"); return; }
    if(strcmp(f,"AB")==0)          { sprintf(buf,"AB"); return; }
    if(strcmp(f,"C")==0)           { sprintf(buf,"C"); return; }
    if(strcmp(f,"DPTR")==0)        { sprintf(buf,"DPTR"); return; }
    if(strcmp(f,"@R0")==0)         { sprintf(buf,"@R0"); return; }
    if(strcmp(f,"@R1")==0)         { sprintf(buf,"@R1"); return; }
    if(strcmp(f,"@A+PC")==0)       { sprintf(buf,"A, @A+PC"); return; }
    if(strcmp(f,"@A+DPTR")==0)     { sprintf(buf,"A, @A+DPTR"); return; }
    if(strcmp(f,"@A+DPTR_jmp")==0) { sprintf(buf,"@A+DPTR"); return; }
    if(strcmp(f,"???")==0)         { sprintf(buf,"???"); return; }
    if(strcmp(f,"@DPTR,A")==0)     { sprintf(buf,"@DPTR, A"); return; }
    if(strcmp(f,"@R0,A")==0)       { sprintf(buf,"@R0, A"); return; }
    if(strcmp(f,"@R1,A")==0)       { sprintf(buf,"@R1, A"); return; }
    if(strcmp(f,"A,@DPTR")==0)     { sprintf(buf,"A, @DPTR"); return; }
    if(strcmp(f,"A,@R0")==0)       { sprintf(buf,"A, @R0"); return; }
    if(strcmp(f,"A,@R1")==0)       { sprintf(buf,"A, @R1"); return; }

    /* Rn variants */
    if(strcmp(f,"Rn")==0)          { sprintf(buf,"R%d", op&7); return; }
    if(strcmp(f,"A,Rn")==0)        { sprintf(buf,"A, R%d", op&7); return; }
    if(strcmp(f,"Rn,A")==0)        { sprintf(buf,"R%d, A", op&7); return; }
    if(strcmp(f,"Rn,#b1")==0)      { sprintf(buf,"R%d, #%02XH", op&7, b1); return; }
    if(strcmp(f,"Rn,d1")==0)       { sprintf(buf,"R%d, %02XH", op&7, b1); return; }
    if(strcmp(f,"Rn,rel")==0)      {
        off=(int8_t)b1; target=(uint16_t)(next_pc+off);
        sprintf(buf,"R%d, %04XH", op&7, target); return; }
    if(strcmp(f,"Rn,#b1,rel")==0)  {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"R%d, #%02XH, %04XH", op&7, b1, target); return; }

    /* direct (d1/d2) variants */
    if(strcmp(f,"d1")==0)          { sprintf(buf,"%02XH", b1); return; }
    if(strcmp(f,"d1,A")==0)        { sprintf(buf,"%02XH, A", b1); return; }
    if(strcmp(f,"d1,#b2")==0)      { sprintf(buf,"%02XH, #%02XH", b1, b2); return; }
    if(strcmp(f,"d2,d1")==0)       { sprintf(buf,"%02XH, %02XH", b2, b1); return; }  /* MOV dst,src: [85][src][dst] */
    if(strcmp(f,"d1,@R0")==0)      { sprintf(buf,"%02XH, @R0", b1); return; }
    if(strcmp(f,"d1,@R1")==0)      { sprintf(buf,"%02XH, @R1", b1); return; }
    if(strcmp(f,"d1,Rn")==0)       { sprintf(buf,"%02XH, R%d", b1, op&7); return; }
    if(strcmp(f,"d1,rel")==0)      {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"%02XH, %04XH", b1, target); return; }

    /* immediate variants */
    if(strcmp(f,"A,#b1")==0)       { sprintf(buf,"A, #%02XH", b1); return; }
    if(strcmp(f,"A,d1")==0)        { sprintf(buf,"A, %02XH", b1); return; }
    if(strcmp(f,"A,#b1,rel")==0)   {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"A, #%02XH, %04XH", b1, target); return; }
    if(strcmp(f,"A,d1,rel")==0)    {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"A, %02XH, %04XH", b1, target); return; }

    /* @Ri variants */
    if(strcmp(f,"@R0,#b1")==0)     { sprintf(buf,"@R0, #%02XH", b1); return; }
    if(strcmp(f,"@R1,#b1")==0)     { sprintf(buf,"@R1, #%02XH", b1); return; }
    if(strcmp(f,"@R0,d1")==0)      { sprintf(buf,"@R0, %02XH", b1); return; }
    if(strcmp(f,"@R1,d1")==0)      { sprintf(buf,"@R1, %02XH", b1); return; }
    if(strcmp(f,"@R0,#b1,rel")==0) {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"@R0, #%02XH, %04XH", b1, target); return; }
    if(strcmp(f,"@R1,#b1,rel")==0) {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"@R1, #%02XH, %04XH", b1, target); return; }

    /* bit variants */
    if(strcmp(f,"bit")==0)         { sprintf(buf,"%02XH", b1); return; }
    if(strcmp(f,"bit,C")==0)       { sprintf(buf,"%02XH, C", b1); return; }
    if(strcmp(f,"C,bit")==0)       { sprintf(buf,"C, %02XH", b1); return; }
    if(strcmp(f,"C,/bit")==0)      { sprintf(buf,"C, /%02XH", b1); return; }
    if(strcmp(f,"bit,rel")==0)     {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"%02XH, %04XH", b1, target); return; }

    /* relative jumps */
    if(strcmp(f,"rel1")==0)        {
        off=(int8_t)b1; target=(uint16_t)(next_pc+off);
        sprintf(buf,"%04XH", target); return; }

    /* DPTR,#16 */
    if(strcmp(f,"DPTR,#16")==0)    {
        sprintf(buf,"DPTR, #%04XH", (uint16_t)(b1<<8)|b2); return; }

    /* addr11 — upper 5 from next_pc, A10-A8 from opcode[7:5] */
    if(strcmp(f,"addr11")==0)      {
        target = (next_pc & 0xF800) | ((op & 0xE0)<<3) | b1;
        sprintf(buf,"%04XH", target); return; }

    /* addr16 */
    if(strcmp(f,"addr16")==0)      {
        sprintf(buf,"%04XH", (uint16_t)(b1<<8)|b2); return; }

    sprintf(buf,"???");
    return;
}

int unassembler(void)
{
    char input[8];
    unsigned char *MEM;
    uint32_t MEMSIZE;

    /* Always disassemble IROM */
    MEM = CPU.IROM; MEMSIZE = IROM_SIZE;
    FILE *fp = fopen("IROM.bin","rb");
    if(fp){ fread(MEM,1,MEMSIZE,fp); fclose(fp); }

    int i;
    printf("ORIGIN (hex): ");
    scanf("%x", &i);
    getchar();
    clr();

    while(1) {
        printf("%04X>> ", i);
        fgets(input, sizeof(input), stdin);
        if(input[0] == '.') break;
        if((unsigned)i >= MEMSIZE) { printf("Address out of range\n"); break; }

        uint8_t op = MEM[i];
        uint8_t b1 = ((unsigned)(i+1)<MEMSIZE) ? MEM[i+1] : 0;
        uint8_t b2 = ((unsigned)(i+2)<MEMSIZE) ? MEM[i+2] : 0;
        uint16_t next_pc = (uint16_t)(i + instSize[op]);

        /* Print raw bytes */
        printf("  ");
        for(int k=0; k<instSize[op]; k++)
            printf("%02X ", MEM[i+k]);
        /* Pad to 10 chars */
        for(int k=instSize[op]; k<3; k++) printf("   ");

        /* Mnemonic + operands */
        char ops[32] = "";
        disasm_operands(op, b1, b2, next_pc, ops);
        if(ops[0])
            printf("  %s  %s\n", mnem_name[op], ops);
        else
            printf("  %s\n", mnem_name[op]);

        i += instSize[op];
    }
    clr();
    return 0;
}

/* ─────────────────────────────────────────────
   STORE DATA
───────────────────────────────────────────── */
int store_data(void)
{
    unsigned char *MEM;
    uint32_t MEMSIZE;

    /* SD = external data RAM, like the trainer kit */
    MEM     = CPU.XRAM;
    MEMSIZE = XRAM_SIZE;

    int addr;
    printf("ORIGIN : ");
    scanf("%x", &addr);
    getchar();

    /* Track modified range for file write-back */
    int first_written = addr;
    int last_written  = addr - 1;   /* empty range sentinel */

    char input[16];
    int  val;

    clr();

    while((unsigned)addr < MEMSIZE) {
        /* Show address and current value, prompt for new */
        printf("%04X  %02X - ", addr, MEM[addr]);
        fgets(input, sizeof(input), stdin);

        /* Strip newline */
        int len = strlen(input);
        if(len > 0 && input[len-1] == '\n') input[--len] = '\0';

        /* '.' exits immediately */
        if(input[0] == '.') break;

        /* Empty input = keep current value, just advance */
        if(len == 0) {
            addr++;
            clr();
            continue;
        }

        /* Try to parse a hex byte */
        if(sscanf(input, "%x", &val) == 1) {
            MEM[addr] = (unsigned char)(val & 0xFF);
            if(addr < first_written) first_written = addr;
            if(addr > last_written)  last_written  = addr;
        }
        /* Invalid input: stay on same address, redisplay */
        else {
            clr();
            continue;
        }

        addr++;
        clr();
    }

    /* RAM — no file write-back needed */
    (void)first_written;
    (void)last_written;

    clr();
    return 0;
}

/* ─────────────────────────────────────────────
   MENU
───────────────────────────────────────────── */
int ios(void)
{
    char input[16];
    printf("MC 8051\n");
    printf("A   - assemble\n");
    printf("U   - unassemble\n");
    printf("SD  - store data\n");
    printf("GO  - run program\n");
    printf("Q   - quit\n>> ");
    scanf("%s", input);
    getchar();
    if(strcmp(input,"Q")==0)  return 0;
    if(strcmp(input,"A")==0)  return 1;
    if(strcmp(input,"U")==0)  return 2;
    if(strcmp(input,"SD")==0) return 3;
    if(strcmp(input,"GO")==0) return 4;
    return -1;
}

/* ─────────────────────────────────────────────
   MAIN
───────────────────────────────────────────── */
int main(void)
{
    int powered = 1, state;
    /* Init SP to 07H as real 8051 */
    CPU.IRAM[0x81] = 0x07;

    while(powered) {
        state = ios();
        clr();
        switch(state) {
        case 0: powered = 0;    break;
        case 1: assembler();    break;
        case 2: unassembler();  break;
        case 3: store_data();   break;
        case 4: run();          break;
        default:
            printf("Unknown command.\n"); break;
        }
    }
    printf("Quitting...\n");
    return 0;
}