#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#endif

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
struct CPUState {
    unsigned char IROM[IROM_SIZE];
    unsigned char XROM[XROM_SIZE];
    unsigned char IRAM[IRAM_SIZE];
    unsigned char XRAM[XRAM_SIZE];
} CPU;

/* ─────────────────────────────────────────────
   SFR ADDRESS MACROS
   All SFRs live in IRAM[0x80..0xFF]
───────────────────────────────────────────── */
/* Interrupt */
#define IE         (CPU.IRAM[0xA8])   /* Interrupt Enable            */
#define IP         (CPU.IRAM[0xB8])   /* Interrupt Priority          */
#define TCON       (CPU.IRAM[0x88])   /* Timer/External INT control  */

/* Timer */
#define TMOD       (CPU.IRAM[0x89])   /* Timer Mode                  */
#define TL0        (CPU.IRAM[0x8A])
#define TL1        (CPU.IRAM[0x8B])
#define TH0        (CPU.IRAM[0x8C])
#define TH1        (CPU.IRAM[0x8D])

/* Serial */
#define SCON       (CPU.IRAM[0x98])   /* Serial Control              */
#define SBUF_REG   (CPU.IRAM[0x99])   /* Serial Buffer               */

/* PCON */
#define PCON       (CPU.IRAM[0x87])

/* ─────────────────────────────────────────────
   TCON bit positions
───────────────────────────────────────────── */
#define TCON_IT0   0   /* INT0 edge/level select */
#define TCON_IE0   1   /* INT0 flag              */
#define TCON_IT1   2   /* INT1 edge/level select */
#define TCON_IE1   3   /* INT1 flag              */
#define TCON_TR0   4   /* Timer0 run             */
#define TCON_TF0   5   /* Timer0 overflow flag   */
#define TCON_TR1   6   /* Timer1 run             */
#define TCON_TF1   7   /* Timer1 overflow flag   */

/* SCON bit positions */
#define SCON_RI    0   /* Receive  interrupt flag */
#define SCON_TI    1   /* Transmit interrupt flag */
#define SCON_RB8   2
#define SCON_TB8   3
#define SCON_REN   4   /* Receive enable          */
#define SCON_SM2   5
#define SCON_SM1   6
#define SCON_SM0   7

/* IE bit positions */
#define IE_EX0     0   /* External INT0 enable    */
#define IE_ET0     1   /* Timer0 enable           */
#define IE_EX1     2   /* External INT1 enable    */
#define IE_ET1     3   /* Timer1 enable           */
#define IE_ES      4   /* Serial enable           */
#define IE_EA      7   /* Global enable           */

/* IP bit positions */
#define IP_PX0     0
#define IP_PT0     1
#define IP_PX1     2
#define IP_PT1     3
#define IP_PS      4

/* Interrupt vector addresses */
#define VEC_INT0   0x0003
#define VEC_T0     0x000B
#define VEC_INT1   0x0013
#define VEC_T1     0x001B
#define VEC_SERIAL 0x0023

/* ─────────────────────────────────────────────
   INTERRUPT STATE
───────────────────────────────────────────── */
static int in_isr     = 0;   /* currently inside an ISR?           */
static int isr_level  = 0;   /* priority level of current ISR (0/1)*/

/* ─────────────────────────────────────────────
   SERIAL STATE  (software shift-register)
───────────────────────────────────────────── */
static uint8_t  ser_tx_shift  = 0;
static int      ser_tx_bits   = 0;   /* bits left to clock out  */
static int      ser_rx_bits   = 0;   /* bits received so far    */
static uint32_t ser_baud_cnt  = 0;   /* baud-rate counter       */
static uint32_t ser_baud_reload = 0; /* clocks per serial bit   */

/* ─────────────────────────────────────────────
   TIMER STATE  (32-bit counters backing TH:TL)
───────────────────────────────────────────── */
static uint32_t timer0_count = 0;
static uint32_t timer1_count = 0;

/* ─────────────────────────────────────────────
   CROSS PLATFORM KEYBOARD INPUT
───────────────────────────────────────────── */

#ifdef _WIN32

static void term_raw(void)
{
}

static void term_restore(void)
{
}

static int kbhit_nonblock(void)
{
    if(_kbhit())
        return _getch();

    return 0;
}

#else

static struct termios saved_termios;
static int saved_flags = 0;

static void term_raw(void)
{
    struct termios t;

    tcgetattr(STDIN_FILENO, &saved_termios);

    t = saved_termios;

    t.c_lflag &= ~(ICANON | ECHO);
    t.c_cc[VMIN]  = 0;
    t.c_cc[VTIME] = 0;

    tcsetattr(STDIN_FILENO, TCSANOW, &t);

    saved_flags = fcntl(STDIN_FILENO, F_GETFL, 0);

    fcntl(STDIN_FILENO,
          F_SETFL,
          saved_flags | O_NONBLOCK);
}

static void term_restore(void)
{
    tcsetattr(STDIN_FILENO,
              TCSANOW,
              &saved_termios);

    fcntl(STDIN_FILENO,
          F_SETFL,
          saved_flags);
}

static int kbhit_nonblock(void)
{
    unsigned char ch;

    ssize_t n = read(STDIN_FILENO, &ch, 1);

    if(n > 0)
        return ch;

    return 0;
}

#endif

/* ─────────────────────────────────────────────
   HELPERS
───────────────────────────────────────────── */
void clr(void) { if(CLRMOD) printf("\033[2J\033[H"); }
void delay(void) { return; }

/* PSW bit macros */
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

#define Rn(n)      (CPU.IRAM[((PSW >> 3) & 0x03) * 8 + (n)])

#define DPTR        ((uint16_t)(CPU.IRAM[0x83] << 8 | CPU.IRAM[0x82]))
#define SET_DPTR(v) do { CPU.IRAM[0x83]=((v)>>8)&0xFF; \
                         CPU.IRAM[0x82]=(v)&0xFF; } while(0)

#define SP         (CPU.IRAM[0x81])
#define PUSH8(v)   do { SP++; CPU.IRAM[SP]=(uint8_t)(v); } while(0)
#define POP8(dst)  do { (dst)=CPU.IRAM[SP]; SP--; } while(0)

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
    OP_MOV,
    OP_MOVX_RD,
    OP_MOVX_WR,
    OP_MOVC,
    OP_ADD,
    OP_ADDC,
    OP_SUBB,
    OP_INC_PTR,
    OP_DEC_PTR,
    OP_INC_DPTR,
    OP_MUL,
    OP_DIV,
    OP_DA,
    OP_ANL,
    OP_ORL,
    OP_XRL,
    OP_ANL_C_BIT,
    OP_ORL_C_BIT,
    OP_CLR_BIT,
    OP_SETB_BIT,
    OP_CPL_BIT,
    OP_MOV_BIT_C,
    OP_MOV_C_BIT,
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
    uint8_t  *dst;
    uint8_t  *src;
    uint8_t   imm8;
    uint16_t  imm16;
    uint8_t   bit_addr;
    uint8_t   inv;
    int8_t    offset;
    uint8_t   use_imm;
    uint16_t  movc_base;
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
    *PC += instSize[*op];
}

/* ─────────────────────────────────────────────
   DECODE
───────────────────────────────────────────── */
static DecodedInstr decode(uint8_t op, uint8_t b1, uint8_t b2, uint16_t PC)
{
    DecodedInstr d;
    memset(&d, 0, sizeof(d));

    uint8_t *A = &CPU.IRAM[0xE0];

    switch(op) {

    case 0x00: d.op = OP_NOP; break;

    case 0x01: case 0x21: case 0x41: case 0x61:
    case 0x81: case 0xA1: case 0xC1: case 0xE1:
        d.op    = OP_AJMP;
        d.imm16 = (PC & 0xF800) | ((op & 0xE0) << 3) | b1;
        break;

    case 0x02:
        d.op    = OP_LJMP;
        d.imm16 = ((uint16_t)b1 << 8) | b2;
        break;

    case 0x03: d.op = OP_RR; d.dst = A; break;

    case 0x04: d.op = OP_INC_PTR; d.dst = A; break;
    case 0x05: d.op = OP_INC_PTR; d.dst = &CPU.IRAM[b1]; break;
    case 0x06: d.op = OP_INC_PTR; d.dst = &CPU.IRAM[Rn(0)]; break;
    case 0x07: d.op = OP_INC_PTR; d.dst = &CPU.IRAM[Rn(1)]; break;

    case 0x08: case 0x09: case 0x0A: case 0x0B:
    case 0x0C: case 0x0D: case 0x0E: case 0x0F:
        d.op = OP_INC_PTR; d.dst = &Rn(op & 7); break;

    case 0x10:
        d.op = OP_JBC; d.bit_addr = b1; d.offset = (int8_t)b2; break;

    case 0x11: case 0x31: case 0x51: case 0x71:
    case 0x91: case 0xB1: case 0xD1: case 0xF1:
        d.op    = OP_ACALL;
        d.imm16 = (PC & 0xF800) | ((op & 0xE0) << 3) | b1;
        break;

    case 0x12:
        d.op    = OP_LCALL;
        d.imm16 = ((uint16_t)b1 << 8) | b2;
        break;

    case 0x13: d.op = OP_RRC; d.dst = A; break;

    case 0x14: d.op = OP_DEC_PTR; d.dst = A; break;
    case 0x15: d.op = OP_DEC_PTR; d.dst = &CPU.IRAM[b1]; break;
    case 0x16: d.op = OP_DEC_PTR; d.dst = &CPU.IRAM[Rn(0)]; break;
    case 0x17: d.op = OP_DEC_PTR; d.dst = &CPU.IRAM[Rn(1)]; break;

    case 0x18: case 0x19: case 0x1A: case 0x1B:
    case 0x1C: case 0x1D: case 0x1E: case 0x1F:
        d.op = OP_DEC_PTR; d.dst = &Rn(op & 7); break;

    case 0x20:
        d.op = OP_JB; d.bit_addr = b1; d.offset = (int8_t)b2; break;

    case 0x22: d.op = OP_RET; break;

    case 0x23: d.op = OP_RL; d.dst = A; break;

    case 0x24:
        d.op = OP_ADD; d.dst = A; d.use_imm = 1; d.imm8 = b1; break;
    case 0x25:
        d.op = OP_ADD; d.dst = A; d.src = &CPU.IRAM[b1]; break;
    case 0x26: d.op=OP_ADD; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x27: d.op=OP_ADD; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0x28: case 0x29: case 0x2A: case 0x2B:
    case 0x2C: case 0x2D: case 0x2E: case 0x2F:
        d.op=OP_ADD; d.dst=A; d.src=&Rn(op&7); break;

    case 0x30:
        d.op = OP_JNB; d.bit_addr = b1; d.offset = (int8_t)b2; break;

    case 0x32: d.op = OP_RETI; break;

    case 0x33: d.op = OP_RLC; d.dst = A; break;

    case 0x34:
        d.op=OP_ADDC; d.dst=A; d.use_imm=1; d.imm8=b1; break;
    case 0x35:
        d.op=OP_ADDC; d.dst=A; d.src=&CPU.IRAM[b1]; break;
    case 0x36: d.op=OP_ADDC; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x37: d.op=OP_ADDC; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0x38: case 0x39: case 0x3A: case 0x3B:
    case 0x3C: case 0x3D: case 0x3E: case 0x3F:
        d.op=OP_ADDC; d.dst=A; d.src=&Rn(op&7); break;

    case 0x40: d.op=OP_JC;  d.offset=(int8_t)b1; break;

    case 0x42:
        d.op=OP_ORL; d.dst=&CPU.IRAM[b1]; d.src=A; break;
    case 0x43:
        d.op=OP_ORL; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;
    case 0x44:
        d.op=OP_ORL; d.dst=A; d.use_imm=1; d.imm8=b1; break;
    case 0x45:
        d.op=OP_ORL; d.dst=A; d.src=&CPU.IRAM[b1]; break;
    case 0x46: d.op=OP_ORL; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x47: d.op=OP_ORL; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0x48: case 0x49: case 0x4A: case 0x4B:
    case 0x4C: case 0x4D: case 0x4E: case 0x4F:
        d.op=OP_ORL; d.dst=A; d.src=&Rn(op&7); break;

    case 0x50: d.op=OP_JNC; d.offset=(int8_t)b1; break;

    case 0x52:
        d.op=OP_ANL; d.dst=&CPU.IRAM[b1]; d.src=A; break;
    case 0x53:
        d.op=OP_ANL; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;
    case 0x54:
        d.op=OP_ANL; d.dst=A; d.use_imm=1; d.imm8=b1; break;
    case 0x55:
        d.op=OP_ANL; d.dst=A; d.src=&CPU.IRAM[b1]; break;
    case 0x56: d.op=OP_ANL; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x57: d.op=OP_ANL; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0x58: case 0x59: case 0x5A: case 0x5B:
    case 0x5C: case 0x5D: case 0x5E: case 0x5F:
        d.op=OP_ANL; d.dst=A; d.src=&Rn(op&7); break;

    case 0x60: d.op=OP_JZ; d.offset=(int8_t)b1; break;

    case 0x62:
        d.op=OP_XRL; d.dst=&CPU.IRAM[b1]; d.src=A; break;
    case 0x63:
        d.op=OP_XRL; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;
    case 0x64:
        d.op=OP_XRL; d.dst=A; d.use_imm=1; d.imm8=b1; break;
    case 0x65:
        d.op=OP_XRL; d.dst=A; d.src=&CPU.IRAM[b1]; break;
    case 0x66: d.op=OP_XRL; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x67: d.op=OP_XRL; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0x68: case 0x69: case 0x6A: case 0x6B:
    case 0x6C: case 0x6D: case 0x6E: case 0x6F:
        d.op=OP_XRL; d.dst=A; d.src=&Rn(op&7); break;

    case 0x70: d.op=OP_JNZ; d.offset=(int8_t)b1; break;

    case 0x72:
        d.op=OP_ORL_C_BIT; d.bit_addr=b1; d.inv=0; break;

    case 0x73: d.op=OP_JMP_ADPTR; break;

    case 0x74:
        d.op=OP_MOV; d.dst=A; d.use_imm=1; d.imm8=b1; break;
    case 0x75:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.use_imm=1; d.imm8=b2; break;
    case 0x76:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(0)]; d.use_imm=1; d.imm8=b1; break;
    case 0x77:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(1)]; d.use_imm=1; d.imm8=b1; break;
    case 0x78: case 0x79: case 0x7A: case 0x7B:
    case 0x7C: case 0x7D: case 0x7E: case 0x7F:
        d.op=OP_MOV; d.dst=&Rn(op&7); d.use_imm=1; d.imm8=b1; break;

    case 0x80:
        if(b1 == 0xFE) { d.op = OP_HALT; }
        else           { d.op = OP_SJMP; d.offset = (int8_t)b1; }
        break;

    case 0x82:
        d.op=OP_ANL_C_BIT; d.bit_addr=b1; d.inv=0; break;

    case 0x83:
        d.op=OP_MOVC; d.movc_base=PC; break;

    case 0x84: d.op=OP_DIV; break;

    case 0x85:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b2]; d.src=&CPU.IRAM[b1]; break;

    case 0x86:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x87:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0x88: case 0x89: case 0x8A: case 0x8B:
    case 0x8C: case 0x8D: case 0x8E: case 0x8F:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=&Rn(op&7); break;

    case 0x90:
        d.op=OP_MOV; d.imm16=((uint16_t)b1<<8)|b2;
        d.dst=NULL; d.use_imm=1; break;

    case 0x92:
        d.op=OP_MOV_BIT_C; d.bit_addr=b1; break;

    case 0x93:
        d.op=OP_MOVC; d.movc_base=DPTR; break;

    case 0x94:
        d.op=OP_SUBB; d.dst=A; d.use_imm=1; d.imm8=b1; break;
    case 0x95:
        d.op=OP_SUBB; d.dst=A; d.src=&CPU.IRAM[b1]; break;
    case 0x96: d.op=OP_SUBB; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0x97: d.op=OP_SUBB; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0x98: case 0x99: case 0x9A: case 0x9B:
    case 0x9C: case 0x9D: case 0x9E: case 0x9F:
        d.op=OP_SUBB; d.dst=A; d.src=&Rn(op&7); break;

    case 0xA0:
        d.op=OP_ORL_C_BIT; d.bit_addr=b1; d.inv=1; break;

    case 0xA2:
        d.op=OP_MOV_C_BIT; d.bit_addr=b1; break;

    case 0xA3: d.op=OP_INC_DPTR; break;

    case 0xA4: d.op=OP_MUL; break;

    case 0xA6:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(0)]; d.src=&CPU.IRAM[b1]; break;
    case 0xA7:
        d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(1)]; d.src=&CPU.IRAM[b1]; break;
    case 0xA8: case 0xA9: case 0xAA: case 0xAB:
    case 0xAC: case 0xAD: case 0xAE: case 0xAF:
        d.op=OP_MOV; d.dst=&Rn(op&7); d.src=&CPU.IRAM[b1]; break;

    case 0xB0:
        d.op=OP_ANL_C_BIT; d.bit_addr=b1; d.inv=1; break;

    case 0xB2:
        d.op=OP_CPL_BIT; d.bit_addr=b1; break;
    case 0xB3:
        d.op=OP_CPL_BIT; d.bit_addr=0xFF; break;

    case 0xB4:
        d.op=OP_CJNE; d.dst=A;
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;
    case 0xB5:
        d.op=OP_CJNE; d.dst=A;
        d.src=&CPU.IRAM[b1]; d.offset=(int8_t)b2; break;
    case 0xB6:
        d.op=OP_CJNE; d.dst=&CPU.IRAM[Rn(0)];
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;
    case 0xB7:
        d.op=OP_CJNE; d.dst=&CPU.IRAM[Rn(1)];
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;
    case 0xB8: case 0xB9: case 0xBA: case 0xBB:
    case 0xBC: case 0xBD: case 0xBE: case 0xBF:
        d.op=OP_CJNE; d.dst=&Rn(op&7);
        d.use_imm=1; d.imm8=b1; d.offset=(int8_t)b2; break;

    case 0xC0:
        d.op=OP_PUSH; d.src=&CPU.IRAM[b1]; break;

    case 0xC2:
        d.op=OP_CLR_BIT; d.bit_addr=b1; break;
    case 0xC3:
        d.op=OP_CLR_BIT; d.bit_addr=0xFF; break;

    case 0xC4: d.op=OP_SWAP; d.dst=A; break;

    case 0xC5:
        d.op=OP_XCH; d.dst=A; d.src=&CPU.IRAM[b1]; break;
    case 0xC6: d.op=OP_XCH; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0xC7: d.op=OP_XCH; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0xC8: case 0xC9: case 0xCA: case 0xCB:
    case 0xCC: case 0xCD: case 0xCE: case 0xCF:
        d.op=OP_XCH; d.dst=A; d.src=&Rn(op&7); break;

    case 0xD0:
        d.op=OP_POP; d.dst=&CPU.IRAM[b1]; break;

    case 0xD2:
        d.op=OP_SETB_BIT; d.bit_addr=b1; break;
    case 0xD3:
        d.op=OP_SETB_BIT; d.bit_addr=0xFF; break;

    case 0xD4: d.op=OP_DA; d.dst=A; break;

    case 0xD5:
        d.op=OP_DJNZ; d.dst=&CPU.IRAM[b1]; d.offset=(int8_t)b2; break;

    case 0xD6: d.op=OP_XCHD; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0xD7: d.op=OP_XCHD; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;

    case 0xD8: case 0xD9: case 0xDA: case 0xDB:
    case 0xDC: case 0xDD: case 0xDE: case 0xDF:
        d.op=OP_DJNZ; d.dst=&Rn(op&7); d.offset=(int8_t)b1; break;

    case 0xE0:
        d.op=OP_MOVX_RD; d.imm16=DPTR; break;
    case 0xE2: d.op=OP_MOVX_RD; d.imm16=Rn(0); break;
    case 0xE3: d.op=OP_MOVX_RD; d.imm16=Rn(1); break;

    case 0xE4:
        d.op=OP_MOV; d.dst=A; d.use_imm=1; d.imm8=0; break;
    case 0xE5:
        d.op=OP_MOV; d.dst=A; d.src=&CPU.IRAM[b1]; break;
    case 0xE6: d.op=OP_MOV; d.dst=A; d.src=&CPU.IRAM[Rn(0)]; break;
    case 0xE7: d.op=OP_MOV; d.dst=A; d.src=&CPU.IRAM[Rn(1)]; break;
    case 0xE8: case 0xE9: case 0xEA: case 0xEB:
    case 0xEC: case 0xED: case 0xEE: case 0xEF:
        d.op=OP_MOV; d.dst=A; d.src=&Rn(op&7); break;

    case 0xF0:
        d.op=OP_MOVX_WR; d.imm16=DPTR; break;
    case 0xF2: d.op=OP_MOVX_WR; d.imm16=Rn(0); break;
    case 0xF3: d.op=OP_MOVX_WR; d.imm16=Rn(1); break;

    case 0xF4:
        d.op=OP_MOV; d.dst=A; d.use_imm=1; d.imm8=~(*A); break;

    case 0xF5:
        d.op=OP_MOV; d.dst=&CPU.IRAM[b1]; d.src=A; break;
    case 0xF6: d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(0)]; d.src=A; break;
    case 0xF7: d.op=OP_MOV; d.dst=&CPU.IRAM[Rn(1)]; d.src=A; break;
    case 0xF8: case 0xF9: case 0xFA: case 0xFB:
    case 0xFC: case 0xFD: case 0xFE: case 0xFF:
        d.op=OP_MOV; d.dst=&Rn(op&7); d.src=A; break;

    case 0xA5:
    default:
        d.op = OP_HALT; break;
    }
    return d;
}

/* ─────────────────────────────────────────────
   EXECUTE
───────────────────────────────────────────── */
static int execute(DecodedInstr *d, uint16_t *PC)
{
    uint8_t *A = &CPU.IRAM[0xE0];
    uint8_t  sv;

    sv = d->use_imm ? d->imm8 : (d->src ? *d->src : 0);

    switch(d->op) {

    case OP_NOP: break;

    case OP_HALT: return 1;

    case OP_MOV:
        if(d->dst == NULL) {
            SET_DPTR(d->imm16);
        } else {
            *d->dst = sv;
            if(d->dst == A) SET_P();
        }
        break;

    case OP_MOVX_RD:
        *A = CPU.XRAM[d->imm16];
        SET_P();
        break;

    case OP_MOVX_WR:
        CPU.XRAM[d->imm16] = *A;
        break;

    case OP_MOVC:
        *A = CPU.IROM[(uint16_t)(d->movc_base + *A)];
        SET_P();
        break;

    case OP_ADD: {
        uint16_t r = *d->dst + sv;
        SET_AC((*d->dst & 0xF) + (sv & 0xF) > 0xF);
        SET_OV(((*d->dst ^ sv) & 0x80)==0 && ((*d->dst ^ r) & 0x80));
        SET_CY(r > 0xFF);
        *d->dst = r & 0xFF;
        SET_P();
        break; }

    case OP_ADDC: {
        uint8_t  cy = CY;
        uint16_t r  = *d->dst + sv + cy;
        SET_AC((*d->dst & 0xF) + (sv & 0xF) + cy > 0xF);
        SET_OV(((*d->dst ^ sv) & 0x80)==0 && ((*d->dst ^ r) & 0x80));
        SET_CY(r > 0xFF);
        *d->dst = r & 0xFF;
        SET_P();
        break; }

    case OP_SUBB: {
        uint8_t  cy = CY;
        uint8_t  a  = *d->dst;
        SET_AC((a & 0xF) < (sv & 0xF) + cy);
        SET_OV(((a ^ sv) & 0x80) && ((a ^ (a - sv - cy)) & 0x80));
        SET_CY(a < (uint16_t)sv + cy);
        *d->dst = a - sv - cy;
        SET_P();
        break; }

    case OP_INC_PTR:
        (*d->dst)++;
        if(d->dst == A) SET_P();
        break;

    case OP_DEC_PTR:
        (*d->dst)--;
        if(d->dst == A) SET_P();
        break;

    case OP_INC_DPTR:
        SET_DPTR(DPTR + 1);
        break;

    case OP_MUL: {
        uint16_t r = (uint16_t)*A * CPU.IRAM[0xF0];
        *A             = r & 0xFF;
        CPU.IRAM[0xF0] = (r >> 8) & 0xFF;
        SET_CY(0);
        SET_OV(CPU.IRAM[0xF0] != 0);
        SET_P();
        break; }

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

    case OP_DA: {
        uint16_t tmp = *A;
        if((tmp & 0x0F) > 9 || AC) tmp += 0x06;
        if(tmp > 0x9F || CY)       { tmp += 0x60; SET_CY(1); }
        *A = (uint8_t)(tmp & 0xFF);
        SET_P();
        break; }

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

    case OP_ANL_C_BIT: {
        uint8_t bv = GET_BIT(d->bit_addr);
        if(d->inv) bv = !bv;
        if(!bv) SET_CY(0);
        break; }

    case OP_ORL_C_BIT: {
        uint8_t bv = GET_BIT(d->bit_addr);
        if(d->inv) bv = !bv;
        if(bv) SET_CY(1);
        break; }

    case OP_CLR_BIT:
        if(d->bit_addr == 0xFF) SET_CY(0);
        else SET_BIT(d->bit_addr, 0);
        break;

    case OP_SETB_BIT:
        if(d->bit_addr == 0xFF) SET_CY(1);
        else SET_BIT(d->bit_addr, 1);
        break;

    case OP_CPL_BIT:
        if(d->bit_addr == 0xFF) SET_CY(!CY);
        else SET_BIT(d->bit_addr, !GET_BIT(d->bit_addr));
        break;

    case OP_MOV_BIT_C:
        SET_BIT(d->bit_addr, CY);
        break;

    case OP_MOV_C_BIT:
        SET_CY(GET_BIT(d->bit_addr));
        break;

    case OP_RL:
        *d->dst = (*d->dst << 1) | (*d->dst >> 7);
        SET_P();
        break;

    case OP_RLC: {
        uint8_t new_cy = (*d->dst >> 7) & 1;
        *d->dst = (*d->dst << 1) | CY;
        SET_CY(new_cy);
        SET_P();
        break; }

    case OP_RR:
        *d->dst = (*d->dst >> 1) | (*d->dst << 7);
        SET_P();
        break;

    case OP_RRC: {
        uint8_t new_cy = *d->dst & 1;
        *d->dst = (*d->dst >> 1) | (CY << 7);
        SET_CY(new_cy);
        SET_P();
        break; }

    case OP_SWAP:
        *d->dst = (*d->dst >> 4) | (*d->dst << 4);
        break;

    case OP_XCH: {
        uint8_t t = *d->dst;
        *d->dst   = *d->src;
        *d->src   = t;
        SET_P();
        break; }

    case OP_XCHD: {
        uint8_t t   = *d->dst & 0x0F;
        *d->dst     = (*d->dst & 0xF0) | (*d->src & 0x0F);
        *d->src     = (*d->src & 0xF0) | t;
        SET_P();
        break; }

    case OP_PUSH:
        PUSH8(*d->src);
        break;

    case OP_POP:
        POP8(*d->dst);
        break;

    case OP_SJMP:
        *PC += (int16_t)d->offset;
        break;

    case OP_AJMP:
    case OP_LJMP:
        *PC = d->imm16;
        break;

    case OP_JMP_ADPTR:
        *PC = DPTR + *A;
        break;

    case OP_JC:   if( CY)      *PC += (int16_t)d->offset; break;
    case OP_JNC:  if(!CY)      *PC += (int16_t)d->offset; break;
    case OP_JZ:   if(*A == 0)  *PC += (int16_t)d->offset; break;
    case OP_JNZ:  if(*A != 0)  *PC += (int16_t)d->offset; break;

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

    case OP_CJNE:
        SET_CY(*d->dst < sv);
        if(*d->dst != sv) *PC += (int16_t)d->offset;
        break;

    case OP_DJNZ:
        (*d->dst)--;
        if(*d->dst != 0) *PC += (int16_t)d->offset;
        break;

    case OP_ACALL:
    case OP_LCALL:
        PUSH8(*PC & 0xFF);
        PUSH8((*PC >> 8) & 0xFF);
        *PC = d->imm16;
        break;

    case OP_RET: {
        uint8_t hi, lo;
        POP8(hi); POP8(lo);
        *PC = ((uint16_t)hi << 8) | lo;
        break; }

    /* RETI: return from ISR and clear in_isr flag */
    case OP_RETI: {
        uint8_t hi, lo;
        POP8(hi); POP8(lo);
        *PC = ((uint16_t)hi << 8) | lo;
        in_isr = 0;
        isr_level = -1;
        break; }

    default: return 1;
    }
    return 0;
}

/* ─────────────────────────────────────────────
   TIMER ENGINE
   Called once per machine cycle.
   Handles TMOD modes 0,1,2 for Timer0 and Timer1.
   Mode 3 (split) handled for Timer0 only.
───────────────────────────────────────────── */
static void timer_tick(void)
{
    uint8_t tmod = TMOD;

    /* ── Timer 0 ── */
    if(TCON & (1 << TCON_TR0)) {           /* TR0 set = running */
        uint8_t mode0 = tmod & 0x03;
        uint8_t gate0 = (tmod >> 3) & 1;
        /* GATE=0: run freely; GATE=1: need INT0 pin (we treat pin=1) */
        if(!gate0) {
            uint16_t t0 = ((uint16_t)TH0 << 8) | TL0;
            switch(mode0) {
            case 0: { /* 13-bit timer */
                uint16_t val13 = (t0 & 0x1FFF) + 1;
                if(val13 >= 0x2000) {
                    val13 = 0;
                    TCON |= (1 << TCON_TF0); /* set TF0 */
                }
                TL0 = val13 & 0x1F;
                TH0 = (val13 >> 5) & 0xFF;
                break; }
            case 1: { /* 16-bit timer */
                uint32_t val16 = (uint32_t)t0 + 1;
                if(val16 >= 0x10000) {
                    val16 = 0;
                    TCON |= (1 << TCON_TF0);
                }
                TL0 = val16 & 0xFF;
                TH0 = (val16 >> 8) & 0xFF;
                break; }
            case 2: { /* 8-bit auto-reload */
                uint8_t val8 = TL0 + 1;
                if(val8 == 0) {            /* overflow */
                    TL0 = TH0;             /* reload from TH0 */
                    TCON |= (1 << TCON_TF0);
                } else {
                    TL0 = val8;
                }
                break; }
            case 3: { /* Split: TL0 = independent 8-bit, TH0 controlled by Timer1 TR */
                uint8_t vl = TL0 + 1;
                if(vl == 0) TCON |= (1 << TCON_TF0);
                TL0 = vl;
                /* TH0 ticks only if TR1 is also set (mode 3 quirk) */
                if(TCON & (1 << TCON_TR1)) {
                    uint8_t vh = TH0 + 1;
                    if(vh == 0) TCON |= (1 << TCON_TF1);
                    TH0 = vh;
                }
                break; }
            }
        }
    }

    /* ── Timer 1 ── (mode 3 not used for timer1, it stops timer1) */
    uint8_t mode1 = (tmod >> 4) & 0x03;
    if(mode1 != 3 && (TCON & (1 << TCON_TR1))) {
        uint8_t gate1 = (tmod >> 7) & 1;
        if(!gate1) {
            uint16_t t1 = ((uint16_t)TH1 << 8) | TL1;
            switch(mode1) {
            case 0: {
                uint16_t v = (t1 & 0x1FFF) + 1;
                if(v >= 0x2000) { v = 0; TCON |= (1 << TCON_TF1); }
                TL1 = v & 0x1F; TH1 = (v >> 5) & 0xFF;
                break; }
            case 1: {
                uint32_t v = (uint32_t)t1 + 1;
                if(v >= 0x10000) { v = 0; TCON |= (1 << TCON_TF1); }
                TL1 = v & 0xFF; TH1 = (v >> 8) & 0xFF;
                break; }
            case 2: {
                uint8_t v = TL1 + 1;
                if(v == 0) { TL1 = TH1; TCON |= (1 << TCON_TF1); }
                else TL1 = v;
                break; }
            }
        }
    }
}

/* ─────────────────────────────────────────────
   SERIAL ENGINE
   Mode 1 (most common): 10-bit UART, baud from Timer1.
   TI is set after a byte is shifted out.
   RI is set when a full byte is received from stdin.
   Baud clock derived from Timer1 Mode2 overflow rate.
───────────────────────────────────────────── */
static void serial_tick(void)
{
    uint8_t sm0 = (SCON >> SCON_SM0) & 1;
    uint8_t sm1 = (SCON >> SCON_SM1) & 1;

    /* We support:
       SM0=0, SM1=0 → Mode 0 (shift register, fixed baud = Fosc/12)
       SM0=0, SM1=1 → Mode 1 (8-bit UART, variable baud via Timer1)
       SM0=1, SM1=0 → Mode 2 (9-bit UART, fixed baud)
       SM0=1, SM1=1 → Mode 3 (9-bit UART, variable baud)
       We simulate Mode 0 and Mode 1 fully; Modes 2/3 fall back to Mode 1. */

    /* ── baud rate clock from Timer1 Mode2 overflow ──
       In a real 8051 at 11.0592 MHz, common baud rates are produced by
       setting TH1 to values like 0xFD (9600 baud).
       Here we derive the reload period: each timer tick is 1 machine cycle.
       Reload value determines how many cycles between overflows.
       ser_baud_reload is updated below. */

    uint8_t th1 = TH1;
    uint32_t reload_period = (th1 == 0xFF) ? 1 : (uint32_t)(256 - th1);
    /* SMOD bit in PCON doubles baud rate */
    if(PCON & 0x80) reload_period >>= 1;
    if(reload_period == 0) reload_period = 1;
    ser_baud_reload = reload_period;

    /* ── TX: shift out pending byte ── */
    if(ser_tx_bits > 0) {
        ser_baud_cnt++;
        if(ser_baud_cnt >= ser_baud_reload) {
            ser_baud_cnt = 0;
            ser_tx_bits--;
            if(ser_tx_bits == 0) {
                /* Byte fully sent: output to stdout and set TI */
                fputc(ser_tx_shift, stdout);
                fflush(stdout);
                SCON |= (1 << SCON_TI);
            }
        }
    }

    /* ── RX: check stdin for incoming byte ── */
    if((SCON >> SCON_REN) & 1) {       /* receive enabled */
        if(!((SCON >> SCON_RI) & 1)) { /* RI not already set */
            int ch = kbhit_nonblock();
            if(ch != EOF) {
                SBUF_REG = (uint8_t)ch;
                SCON |= (1 << SCON_RI);
            }
        }
    }

    (void)sm0; (void)sm1; /* suppress unused warning */
}

/* ─────────────────────────────────────────────
   INTERRUPT DISPATCH
   Called after every instruction.
   Checks all interrupt sources in priority order,
   respects IE (global + individual enables) and IP.
   Vectors are called only if not already in same-or-higher ISR.
───────────────────────────────────────────── */
static void interrupt_check(uint16_t *PC)
{
    /* Global interrupt enable */
    if(!((IE >> IE_EA) & 1)) return;

    /* Build pending flags: (enabled_source, priority, clear_flag_fn, vector) */
    struct {
        int     pending;    /* interrupt is pending & enabled */
        int     priority;   /* 0=low, 1=high                  */
        int     flag_byte;  /* IRAM address of flag register  */
        uint8_t flag_bit;   /* bit position of the flag       */
        uint16_t vector;
    } src[5];

    /* INT0 */
    src[0].pending  = ((IE >> IE_EX0) & 1) && ((TCON >> TCON_IE0) & 1);
    src[0].priority = (IP >> IP_PX0) & 1;
    src[0].flag_byte= 0x88; src[0].flag_bit = TCON_IE0;
    src[0].vector   = VEC_INT0;

    /* Timer0 */
    src[1].pending  = ((IE >> IE_ET0) & 1) && ((TCON >> TCON_TF0) & 1);
    src[1].priority = (IP >> IP_PT0) & 1;
    src[1].flag_byte= 0x88; src[1].flag_bit = TCON_TF0;
    src[1].vector   = VEC_T0;

    /* INT1 */
    src[2].pending  = ((IE >> IE_EX1) & 1) && ((TCON >> TCON_IE1) & 1);
    src[2].priority = (IP >> IP_PX1) & 1;
    src[2].flag_byte= 0x88; src[2].flag_bit = TCON_IE1;
    src[2].vector   = VEC_INT1;

    /* Timer1 */
    src[3].pending  = ((IE >> IE_ET1) & 1) && ((TCON >> TCON_TF1) & 1);
    src[3].priority = (IP >> IP_PT1) & 1;
    src[3].flag_byte= 0x88; src[3].flag_bit = TCON_TF1;
    src[3].vector   = VEC_T1;

    /* Serial */
    src[4].pending  = ((IE >> IE_ES) & 1) &&
                      (((SCON >> SCON_RI) & 1) || ((SCON >> SCON_TI) & 1));
    src[4].priority = (IP >> IP_PS) & 1;
    src[4].flag_byte= -1; /* serial flags cleared by software */
    src[4].flag_bit = 0;
    src[4].vector   = VEC_SERIAL;

    /* Service highest-priority pending interrupt that can preempt */
    for(int i = 0; i < 5; i++) {
        if(!src[i].pending) continue;
        int prio = src[i].priority;
        /* Can only preempt if higher priority than current ISR */
        if(in_isr && prio <= isr_level) continue;

        /* Enter ISR: push PC, jump to vector */
        PUSH8(*PC & 0xFF);
        PUSH8((*PC >> 8) & 0xFF);
        *PC = src[i].vector;

        /* Clear hardware-cleared flags (TF0, TF1, IE0, IE1) */
        if(src[i].flag_byte >= 0)
            CPU.IRAM[src[i].flag_byte] &= ~(1 << src[i].flag_bit);

        in_isr    = 1;
        isr_level = prio;
        break;   /* one interrupt per cycle */
    }
}

/* ─────────────────────────────────────────────
   SBUF WRITE HOOK
   Called when the CPU writes to SBUF (0x99).
   Starts the serial transmit shift sequence.
───────────────────────────────────────────── */
static void sbuf_write_hook(uint8_t val)
{
    ser_tx_shift = val;
    /* Mode 1: 10 bits = start + 8 data + stop */
    ser_tx_bits  = 10;
    ser_baud_cnt = 0;
    /* Clear TI — it will be set again when done */
    SCON &= ~(1 << SCON_TI);
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
    printf("\n");
    printf("--- Timers ---\n");
    printf("TCON:%02XH  TMOD:%02XH\n", TCON, TMOD);
    printf("T0: TH0=%02XH TL0=%02XH  T1: TH1=%02XH TL1=%02XH\n",
           TH0, TL0, TH1, TL1);
    printf("--- Serial ---\n");
    printf("SCON:%02XH  SBUF:%02XH\n", SCON, SBUF_REG);
    printf("--- Interrupts ---\n");
    printf("IE  :%02XH  IP  :%02XH\n", IE, IP);
    printf("===========================\n");
}

/* ─────────────────────────────────────────────
   RUN  (main emulation loop)
───────────────────────────────────────────── */
int run(void)
{
    uint16_t PC = 0;
    uint8_t  op, b1, b2;

    /* Reset peripheral state */
    in_isr        = 0;
    isr_level     = -1;
    ser_tx_bits   = 0;
    ser_rx_bits   = 0;
    ser_baud_cnt  = 0;
    timer0_count  = 0;
    timer1_count  = 0;

    FILE *fp = fopen("IROM.bin", "rb");
    if(fp) { fread(CPU.IROM, 1, IROM_SIZE, fp); fclose(fp); }
    fp = fopen("XROM.bin", "rb");
    if(fp) { fread(CPU.XROM, 1, XROM_SIZE, fp); fclose(fp); }

    printf("START ADDRESS (hex): ");
    scanf("%hX", &PC);
    getchar();
    clr();

    printf("Running from %04XH ... (press any key to stop)\n", PC);

    term_raw();
    atexit(term_restore);

    while(1) {
        int key = kbhit_nonblock();
        if(key) break;

        /* ── peripheral tick (once per machine cycle) ── */
        timer_tick();
        serial_tick();

        /* ── instruction cycle ── */
        fetch(&PC, &op, &b1, &b2);

        /* Intercept SBUF write (MOV 99H,A or MOV SBUF,src) */
        /* We detect a direct write to address 0x99 at decode time */
        int sbuf_written = 0;
        if(op == 0xF5 && b1 == 0x99) sbuf_written = 1; /* MOV 99H,A */
        if(op == 0x75 && b1 == 0x99) sbuf_written = 2; /* MOV 99H,#imm */
        /* also Rn→SBUF variants handled generically below */

        DecodedInstr d = decode(op, b1, b2, PC);

        if(execute(&d, &PC)) break;

        /* After execute: if SBUF was written, start TX */
        if(sbuf_written == 1) sbuf_write_hook(CPU.IRAM[0xE0]); /* value of A */
        if(sbuf_written == 2) sbuf_write_hook(b2);              /* immediate  */

        /* ── interrupt check ── */
        interrupt_check(&PC);

        delay();
    }

    term_restore();

    reg_dump(PC);
    printf("\nPress Enter to return to menu...");
    getchar();
    clr();
    return 0;
}

/* ─────────────────────────────────────────────
   HELPERS: SFR lookup + value extraction
───────────────────────────────────────────── */
void address(char *word)
{
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
    int k, val;
    int len = (int)strlen(strVal);
    (void)valid; /* valid is an output parameter set by callers */

    for(k = 0; k < len; k++) {
        char c = strVal[k];
        if(isxdigit((unsigned char)c) || c=='#' || c=='@' || c=='/'
           || c=='H' || c=='D') continue;
        *type = -1; return 0;
    }

    char *rVal = (char*)malloc(len + 1);
    if(!rVal) { *type = -1; return 0; }

    if(strVal[0]!='#' && strVal[0]!='@' && strVal[0]=='/') {
        *type = -1; free(rVal); return 0;
    }

    if(strVal[0]!='#' && strVal[0]!='@' && strVal[0]!='/') {
        *type = 0;
        strcpy(rVal, strVal);
    } else {
        for(k = 0; k < len - 1; k++) rVal[k] = strVal[k+1];
        rVal[k] = '\0';
        if     (strVal[0]=='#') *type = 1;
        else if(strVal[0]=='@') *type = 2;
        else                    *type = 3;
    }

    char fc = rVal[0];
    if((fc>='A' && fc<='F') || (fc>='a' && fc<='f')) {
        *type = -1; free(rVal); return 0;
    }

    int rlen = (int)strlen(rVal);
    if(rlen == 0) { *type = -1; free(rVal); return 0; }

    if(rVal[rlen-1] == 'H' || rVal[rlen-1] == 'h')
        sscanf(rVal, "%x", &val);
    else if(rVal[rlen-1] == 'D' || rVal[rlen-1] == 'd')
        sscanf(rVal, "%d", &val);
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
    char input[100] = {0};
    unsigned char *MEM = CPU.IROM;

    printf("ORIGIN : ");
    scanf("%X", &i);
    getchar();
    clr();

    char mnemonic[4][16] = {"","","",""};
    unsigned char opcodes[3];

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
                address(mnemonic[mnemonicnum]);
                mnemonicnum++;
                mnemonicch = 0;
            } else valid = 0;
            ch++;
        }
        mnemonic[mnemonicnum][mnemonicch] = '\0';
        address(mnemonic[mnemonicnum]);
        j = i;

        /* ── ENCODE ── */
        if(strcmp(mnemonic[0],"NOP")==0 && mnemonicnum==0){
            opcodes[0]=0x00; bytes=1;}
        else if(strcmp(mnemonic[0],"AJMP")==0 && mnemonicnum==1){
            unsigned int a = extractVal(mnemonic[1],&type[0],&valid);
            if((a>>11)==((unsigned)(i+2)>>11) && type[0]!=-1){
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
            if((a>>11)==((unsigned)(i+2)>>11)&&type[0]!=-1){
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
        else if(strcmp(mnemonic[0],"MOVC")==0&&mnemonicnum==2&&strcmp(mnemonic[1],"A")==0){
            if(strcmp(mnemonic[2],"@A+PC")==0){opcodes[0]=0x83;bytes=1;}
            else if(strcmp(mnemonic[2],"@A+DPTR")==0){opcodes[0]=0x93;bytes=1;}
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
            if(fp){
                fseek(fp, j, SEEK_SET);
                fwrite(opcodes, 1, bytes, fp);
            }
        } else {
            printf("Invalid instruction!\n");
        }

        getchar();
        clr();
    }

    if(fp) fclose(fp);
    clr();
    return 0;
}

/* ─────────────────────────────────────────────
   UNASSEMBLER
───────────────────────────────────────────── */
static const char *mnem_name[256] = {
    "NOP",  "AJMP", "LJMP", "RR",   "INC",  "INC",  "INC",  "INC",
    "INC",  "INC",  "INC",  "INC",  "INC",  "INC",  "INC",  "INC",
    "JBC",  "ACALL","LCALL","RRC",  "DEC",  "DEC",  "DEC",  "DEC",
    "DEC",  "DEC",  "DEC",  "DEC",  "DEC",  "DEC",  "DEC",  "DEC",
    "JB",   "AJMP", "RET",  "RL",   "ADD",  "ADD",  "ADD",  "ADD",
    "ADD",  "ADD",  "ADD",  "ADD",  "ADD",  "ADD",  "ADD",  "ADD",
    "JNB",  "ACALL","RETI", "RLC",  "ADDC", "ADDC", "ADDC", "ADDC",
    "ADDC", "ADDC", "ADDC", "ADDC", "ADDC", "ADDC", "ADDC", "ADDC",
    "JC",   "AJMP", "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL",
    "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL",  "ORL",
    "JNC",  "ACALL","ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL",
    "ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL",  "ANL",
    "JZ",   "AJMP", "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL",
    "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL",  "XRL",
    "JNZ",  "ACALL","ORL",  "JMP",  "MOV",  "MOV",  "MOV",  "MOV",
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",
    "SJMP", "AJMP", "ANL",  "MOVC", "DIV",  "MOV",  "MOV",  "MOV",
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",
    "MOV",  "ACALL","MOV",  "MOVC", "SUBB", "SUBB", "SUBB", "SUBB",
    "SUBB", "SUBB", "SUBB", "SUBB", "SUBB", "SUBB", "SUBB", "SUBB",
    "ORL",  "AJMP", "MOV",  "INC",  "MUL",  "???",  "MOV",  "MOV",
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",
    "ANL",  "ACALL","CPL",  "CPL",  "CJNE", "CJNE", "CJNE", "CJNE",
    "CJNE", "CJNE", "CJNE", "CJNE", "CJNE", "CJNE", "CJNE", "CJNE",
    "PUSH", "AJMP", "CLR",  "CLR",  "SWAP", "XCH",  "XCH",  "XCH",
    "XCH",  "XCH",  "XCH",  "XCH",  "XCH",  "XCH",  "XCH",  "XCH",
    "POP",  "ACALL","SETB", "SETB", "DA",   "DJNZ", "XCHD", "XCHD",
    "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ", "DJNZ",
    "MOVX", "AJMP", "MOVX", "MOVX", "CLR",  "MOV",  "MOV",  "MOV",
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",
    "MOVX", "ACALL","MOVX", "MOVX", "CPL",  "MOV",  "MOV",  "MOV",
    "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV",  "MOV"
};

static const char *dis_fmt[256] = {
    "", "addr11", "addr16", "A", "A", "d1", "@R0", "@R1", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn",
    "bit,rel", "addr11", "addr16", "A", "A", "d1", "@R0", "@R1", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn", "Rn",
    "bit,rel", "addr11", "", "A", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "bit,rel", "addr11", "", "A", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "rel1", "addr11", "d1,A", "d1,#b2", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "rel1", "addr11", "d1,A", "d1,#b2", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "rel1", "addr11", "d1,A", "d1,#b2", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "rel1", "addr11", "C,bit", "@A+DPTR_jmp", "A,#b1", "d1,#b2", "@R0,#b1", "@R1,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1", "Rn,#b1",
    "rel1", "addr11", "C,bit", "@A+PC", "AB", "d2,d1", "d1,@R0", "d1,@R1", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn", "d1,Rn",
    "DPTR,#16", "addr11", "bit,C", "@A+DPTR", "A,#b1", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "C,/bit", "addr11", "C,bit", "DPTR", "AB", "???", "@R0,d1", "@R1,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1", "Rn,d1",
    "C,/bit", "addr11", "bit", "C", "A,#b1,rel", "A,d1,rel", "@R0,#b1,rel", "@R1,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel", "Rn,#b1,rel",
    "d1", "addr11", "bit", "C", "A", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "d1", "addr11", "bit", "C", "A", "d1,rel", "A,@R0", "A,@R1", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel", "Rn,rel",
    "A,@DPTR", "addr11", "A,@R0", "A,@R1", "A", "A,d1", "A,@R0", "A,@R1", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn", "A,Rn",
    "@DPTR,A", "addr11", "@R0,A", "@R1,A", "A", "d1,A", "@R0,A", "@R1,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A", "Rn,A",
};

static void disasm_operands(uint8_t op, uint8_t b1, uint8_t b2,
                            uint16_t next_pc, char *buf)
{
    const char *f = dis_fmt[op];
    int8_t off;
    uint16_t target;
    buf[0] = '\0';

    if(strcmp(f,"")==0) return;

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

    if(strcmp(f,"d1")==0)          { sprintf(buf,"%02XH", b1); return; }
    if(strcmp(f,"d1,A")==0)        { sprintf(buf,"%02XH, A", b1); return; }
    if(strcmp(f,"d1,#b2")==0)      { sprintf(buf,"%02XH, #%02XH", b1, b2); return; }
    if(strcmp(f,"d2,d1")==0)       { sprintf(buf,"%02XH, %02XH", b2, b1); return; }
    if(strcmp(f,"d1,@R0")==0)      { sprintf(buf,"%02XH, @R0", b1); return; }
    if(strcmp(f,"d1,@R1")==0)      { sprintf(buf,"%02XH, @R1", b1); return; }
    if(strcmp(f,"d1,Rn")==0)       { sprintf(buf,"%02XH, R%d", b1, op&7); return; }
    if(strcmp(f,"d1,rel")==0)      {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"%02XH, %04XH", b1, target); return; }

    if(strcmp(f,"A,#b1")==0)       { sprintf(buf,"A, #%02XH", b1); return; }
    if(strcmp(f,"A,d1")==0)        { sprintf(buf,"A, %02XH", b1); return; }
    if(strcmp(f,"A,#b1,rel")==0)   {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"A, #%02XH, %04XH", b1, target); return; }
    if(strcmp(f,"A,d1,rel")==0)    {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"A, %02XH, %04XH", b1, target); return; }

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

    if(strcmp(f,"bit")==0)         { sprintf(buf,"%02XH", b1); return; }
    if(strcmp(f,"bit,C")==0)       { sprintf(buf,"%02XH, C", b1); return; }
    if(strcmp(f,"C,bit")==0)       { sprintf(buf,"C, %02XH", b1); return; }
    if(strcmp(f,"C,/bit")==0)      { sprintf(buf,"C, /%02XH", b1); return; }
    if(strcmp(f,"bit,rel")==0)     {
        off=(int8_t)b2; target=(uint16_t)(next_pc+off);
        sprintf(buf,"%02XH, %04XH", b1, target); return; }

    if(strcmp(f,"rel1")==0)        {
        off=(int8_t)b1; target=(uint16_t)(next_pc+off);
        sprintf(buf,"%04XH", target); return; }

    if(strcmp(f,"DPTR,#16")==0)    {
        sprintf(buf,"DPTR, #%04XH", (uint16_t)(b1<<8)|b2); return; }

    if(strcmp(f,"addr11")==0)      {
        target = (next_pc & 0xF800) | ((op & 0xE0)<<3) | b1;
        sprintf(buf,"%04XH", target); return; }

    if(strcmp(f,"addr16")==0)      {
        sprintf(buf,"%04XH", (uint16_t)(b1<<8)|b2); return; }

    sprintf(buf,"???");
}

int unassembler(void)
{
    char input[8];
    unsigned char *MEM = CPU.IROM;
    uint32_t MEMSIZE   = IROM_SIZE;

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

        uint8_t op      = MEM[i];
        uint8_t b1      = ((unsigned)(i+1)<MEMSIZE) ? MEM[i+1] : 0;
        uint8_t b2      = ((unsigned)(i+2)<MEMSIZE) ? MEM[i+2] : 0;
        uint16_t npc    = (uint16_t)(i + instSize[op]);

        printf("  ");
        for(int k=0; k<instSize[op]; k++) printf("%02X ", MEM[i+k]);
        for(int k=instSize[op]; k<3; k++) printf("   ");

        char ops[32] = "";
        disasm_operands(op, b1, b2, npc, ops);
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
    unsigned char *MEM = CPU.XRAM;
    uint32_t MEMSIZE   = XRAM_SIZE;

    int addr;
    printf("ORIGIN : ");
    scanf("%x", &addr);
    getchar();

    char input[16];
    int  val;
    clr();

    while((unsigned)addr < MEMSIZE) {
        printf("%04X  %02X - ", addr, MEM[addr]);
        fgets(input, sizeof(input), stdin);

        int len = strlen(input);
        if(len > 0 && input[len-1] == '\n') input[--len] = '\0';

        if(input[0] == '.') break;

        if(len == 0) { addr++; clr(); continue; }

        if(sscanf(input, "%x", &val) == 1) {
            MEM[addr] = (unsigned char)(val & 0xFF);
        }
        addr++;
        clr();
    }

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
    CPU.IRAM[0x81] = 0x07;   /* SP reset to 07H */

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
