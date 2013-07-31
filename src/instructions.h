// Bit manipulation
#define RES(B, R) R &= ~(1 << B)
#define SET(B, R) R |= (1 << B)
#define BIT(B, R) REG_F = (R & (1 << B) ? 0 : ZF) | HF | (REG_F & CF)

// Bit shift right
#define RLC(R)  R = (R << 1) | ((R & 0x80) >> 7);               \
                REG_F = (R ? 0 : ZF) | (R & 1 ? CF : 0)

#define RRC(R)  R = (R >> 1) | ((R & 1) << 7);                  \
                REG_F = (R ? 0 : ZF) | (R & 0x80 ? CF : 0)

#define RL(R)   TR.LO = R;                                      \
                R = (R << 1) | ((REG_F & CF) >> 4);             \
                REG_F = (R ? 0 : ZF) | (TR.LO & 0x80 ? CF : 0)

#define RR(R)   TR.LO = R;                                      \
                R = (R >> 1) | ((REG_F & CF) << 3);             \
                REG_F = (R ? 0 : ZF) | (TR.LO & 1 ? CF : 0)

// Bit shift left
#define SLA(R)  TR.LO = R;                                      \
                R = (R << 1);                                   \
                REG_F = (R ? 0 : ZF) | (TR.LO & 0x80 ? CF : 0)

#define SRA(R)  TR.LO = R;                                      \
                R = (R >> 1) | (R & 0x80);                      \
                REG_F = (R ? 0 : ZF) | (TR.LO & 1 ? CF : 0)

#define SWAP(R) R = (R << 4) | (R >> 4);                        \
                REG_F = (R ? 0 : ZF)

#define SRL(R)  TR.LO = R;                                      \
                R = (R >> 1);                                   \
                REG_F = (R ? 0 : ZF) | (TR.LO & 1 ? CF : 0)

// Add
#define ADD_BYTES(R1, R2)                                       \
    TR.W = R1 + R2;                                             \
    REG_F = (TR.LO ? 0 : ZF) |                                  \
            (((R1 ^ R2 ^ TR.LO) & 0x10) ? HF : 0) |             \
            (TR.HI ? CF :0);                                    \
    R1 = TR.LO
    
#define ADC_BYTES(R1, R2)                                       \
    TR.W = R1 + R2 + ((REG_F & CF) ? 1 : 0);                    \
    REG_F = (TR.LO ? 0 : ZF) |                                  \
            (((R1 ^ R2 ^ TR.LO) & 0x10) ? HF : 0) |             \
            (TR.HI ? CF : 0);                                   \
    R1=TR.LO

// Subtract
#define SUB_BYTES(R1, R2)                                       \
    TR.W = R1 - R2;                                             \
    REG_F = (TR.LO ? 0 : ZF) |                                  \
            NF |                                                \
            (((R1^R2^TR.LO)&0x10) ? HF : 0) |                   \
            (TR.HI ? CF : 0);                                   \
    R1=TR.LO

#define SBC_BYTES(R1, R2)                                       \
    TR.W = R1 - R2 - ((REG_F & CF) ? 1 : 0);                    \
    REG_F = ((TR.LO) ? 0 : ZF) |                                \
        NF |                                                    \
        (((R1^R2^TR.LO)&0x10) ? HF : 0) |                       \
        (TR.HI ? CF : 0);                                       \
    R1=TR.LO

// Bitwise operations
#define AND_BYTE(R) REG_A &= R; REG_F = (REG_A ? 0 : ZF) | HF
#define XOR_BYTE(R) REG_A ^= R; REG_F = (REG_A ? 0 : ZF)
#define OR_BYTE(R)  REG_A |= R; REG_F = (REG_A ? 0 : ZF)

#define CP_BYTES(R1, R2)                                        \
    TR.W=R1-R2;\
    REG_F = (TR.LO ? 0 : ZF) |                          \
            NF |                                                \
            (((R1^R2^TR.LO)&0x10) ? HF : 0) |               \
            (TR.HI ? CF : 0)
            
#define LD_BYTE(R1, R2) R1 = R2
