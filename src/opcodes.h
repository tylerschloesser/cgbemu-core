case 0x00: //NOP
    CLOCK_CYCLES(4);
case 0x01: //LD BC, d16
    REG_C = READ(PC.W++);
    REG_B = READ(PC.W++);
    CLOCK_CYCLES(12);
case 0x02: //LD (BC), A
    WRITE(BC.W, REG_A);
    CLOCK_CYCLES(8);
case 0x03: //INC BC
    BC.W++;
    CLOCK_CYCLES(8); 
case 0x04: //INC B
    REG_B++;
    REG_F = (REG_B ? 0 : ZF) | ((REG_B & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x05: //DEC B
    REG_B--;
    REG_F = (REG_B ? 0 : ZF) | NF | ((REG_B & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x06: //LD B, d8
    REG_B = READ(PC.W++);
    CLOCK_CYCLES(8);
case 0x07: //RLCA
    REG_F = (REG_A & 0x80) ? CF : 0;
    REG_A = (REG_F & CF) ? ((REG_A << 1) | 1) : (REG_A << 1);
    CLOCK_CYCLES(4);
case 0x08: //LD (a16), SP
    TR.LO = READ(PC.W++);
    TR.HI = READ(PC.W++);
    WRITE(TR.W++, SP.LO);
    WRITE(TR.W, SP.HI);
    CLOCK_CYCLES(20);
case 0x09: //ADD HL, BC
    TR.W = HL.W + BC.W;
    REG_F = (REG_F & ZF) | (((HL.W ^ BC.W ^ TR.W) & 0x1000) ? HF : 0) | (TR.W < BC.W ? CF : 0);
    HL.W = TR.W;
    CLOCK_CYCLES(8);
case 0x0A: //LD A, (BC)
    REG_A = READ(BC.W);
    CLOCK_CYCLES(8);
case 0x0B: //DEC BC
    BC.W--;
    CLOCK_CYCLES(8);
case 0x0C: //INC C
    REG_C++;
    REG_F = (REG_C ? 0 : ZF) | ((REG_C & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x0D: //DEC C
    REG_C--;
    REG_F = (REG_C ? 0 : ZF) | NF | ((REG_C & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x0E: //LD C, d8
    REG_C = READ(PC.W++);
    CLOCK_CYCLES(8);
case 0x0F: //RRCA
    REG_F = (REG_A & 1) ? CF : 0;
    REG_A = (REG_F & CF) ? ((REG_A >> 1) | 0x80) : (REG_A >> 1);
    CLOCK_CYCLES(4);
case 0x10: //STOP 0
    //printf("STOP 0\n");
    //TODO I'm not exactly sure what should happen on stop, so let's just print this message and 
    //deal with it later...
    CLOCK_CYCLES(4);
case 0x11: //LD DE, d16
    REG_E = READ(PC.W++);
    REG_D = READ(PC.W++);
    CLOCK_CYCLES(12);
case 0x12: //LD (DE), A
    WRITE(DE.W, REG_A);
    CLOCK_CYCLES(8);
case 0x13: //INC DE
    DE.W++;
    CLOCK_CYCLES(8);
case 0x14: //INC D
    REG_D++;
    REG_F = (REG_D ? 0 : ZF) | ((REG_D & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x15: //DEC D
    REG_D--;
    REG_F = (REG_D ? 0 : ZF) | NF | ((REG_D & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x16: //LD D, d8
    REG_D = READ(PC.W++);
    CLOCK_CYCLES(8);
case 0x17: //RLA
    TR.LO = (REG_F & CF);
    REG_F = (REG_A & 0x80) ? CF : 0;
    REG_A = (REG_A << 1) | (TR.LO ? 1 : 0);
    CLOCK_CYCLES(4);
case 0x18: //JR r8
    PC.W += (signed char)READ(PC.W) + 1;
    CLOCK_CYCLES(12);
case 0x19: //ADD HL, DE
    TR.W = HL.W + DE.W;
    REG_F = (REG_F & ZF) | (((HL.W ^ DE.W ^ TR.W) & 0x1000) ? HF : 0) | (TR.W < DE.W ? CF : 0);
    HL.W = TR.W;
    CLOCK_CYCLES(8);
case 0x1A: //LD A, (DE)
    REG_A = READ(DE.W);
    CLOCK_CYCLES(8);
case 0x1B: //DEC DE
    DE.W--;
    CLOCK_CYCLES(8);
case 0x1C: //INC E
    REG_E++;
    REG_F = (REG_E ? 0 : ZF) | ((REG_E & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x1D: //DEC E
    REG_E--;
    REG_F = (REG_E ? 0 : ZF) | NF | ((REG_E & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x1E: //LD E, d8
    REG_E = READ(PC.W++);
    CLOCK_CYCLES(8);
case 0x1F: //RRA
    TR.LO = (REG_F & CF);
    REG_F = (REG_A & 1) ? CF : 0;
    REG_A = (REG_A >> 1) | (TR.LO ? 0x80 : 0);
    CLOCK_CYCLES(4);
case 0x20: //JR NZ, r8
    if(REG_F & ZF) {
        PC.W++;
        CLOCK_CYCLES(8);
    } else {
        PC.W += (signed char)READ(PC.W) + 1;
        CLOCK_CYCLES(12);
    }
case 0x21: //LD HL, d16
    REG_L = READ(PC.W++);
    REG_H = READ(PC.W++);
    CLOCK_CYCLES(12);
case 0x22: //LD(HL+), A
    WRITE(HL.W++, REG_A);
    CLOCK_CYCLES(8);
case 0x23: //INC HL
    HL.W++;
    CLOCK_CYCLES(8);
case 0x24: //INC H
    REG_H++;
    REG_F = (REG_H ? 0 : ZF) | ((REG_H & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x25: //DEC H
    REG_H--;
    REG_F = (REG_H ? 0 : ZF) | NF | ((REG_H & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x26: //LD H, d8
    REG_H = READ(PC.W++);
    CLOCK_CYCLES(8);
case 0x27: //DAA

    TR.W = REG_A;
    TR.W |= (REG_F&(CF|HF|NF))<<4;
    AF.W = DAA_table[TR.W];
    CLOCK_CYCLES(4);

case 0x28: //JR Z, r8
    if(REG_F & ZF) {
        PC.W += (signed char)READ(PC.W) + 1;
        CLOCK_CYCLES(12);
    } else {
        PC.W++;
        CLOCK_CYCLES(8);
    }
case 0x29: //ADD HL, HL
    TR.W = HL.W + HL.W;
    REG_F = (REG_F & ZF) | (((HL.W ^ HL.W ^ TR.W) & 0x1000) ? HF : 0) | (TR.W < HL.W ? CF : 0);
    HL.W = TR.W;
    CLOCK_CYCLES(8);
case 0x2A: //LD A, (HL+)
    REG_A = READ(HL.W++);
    CLOCK_CYCLES(8);
case 0x2B: //DEC HL
    HL.W--;
    CLOCK_CYCLES(8);
case 0x2C: //INC L
    REG_L++;
    REG_F = (REG_L ? 0 : ZF) | ((REG_L & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x2D: //DEC L
    REG_L--;
    REG_F = (REG_L ? 0 : ZF) | NF | ((REG_L & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x2E: //LD L, d8
    REG_L = READ(PC.W++);
    CLOCK_CYCLES(8);
case 0x2F: /* CPL */
    REG_A ^= 0xFF;
    REG_F = (REG_F & ZF) | NF | HF | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x30: /* JR NC, r8 */
    if(REG_F & CF) {
        PC.W++;
        CLOCK_CYCLES(8);
    } else {
        PC.W += (s8)READ(PC.W) + 1;
        CLOCK_CYCLES(12);
    }
case 0x31: /* LD SP, d16 */
    SP.LO = READ(PC.W++);
    SP.HI = READ(PC.W++);
    CLOCK_CYCLES(12);
case 0x32: /* LD (HL-), A */
    WRITE(HL.W--, REG_A);
    CLOCK_CYCLES(8);
case 0x33: /* INC SP */
    SP.W++;
    CLOCK_CYCLES(8);
case 0x34: /* INC (HL) */
    TR.LO = READ(HL.W) + 1;
    WRITE(HL.W, TR.LO);
    REG_F = (TR.LO ? 0 : ZF) | ((TR.LO & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(12);
case 0x35: /* DEC (HL) */
    TR.LO = READ(HL.W) - 1;
    WRITE(HL.W, TR.LO);
    REG_F = (TR.LO ? 0 : ZF) | NF | ((TR.LO & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(12);
case 0x36: /* LD (HL), d8 */
    WRITE(HL.W, READ(PC.W++));
    CLOCK_CYCLES(12);
case 0x37: /* SCF */
    REG_F = (REG_F & ZF) | CF;
    CLOCK_CYCLES(4);
case 0x38: /* JR C, r8 */
    if(REG_F & CF) {
        PC.W += (s8)READ(PC.W) + 1;
        CLOCK_CYCLES(12);
    } else {
        PC.W++;
        CLOCK_CYCLES(8);
    }
case 0x39: /* ADD HL, SP */
    TR.W = HL.W + SP.W;
    REG_F = (REG_F & ZF) | (((HL.W ^ SP.W ^ TR.W) & 0x1000) ? HF : 0) | (TR.W < SP.W ? CF : 0);
    HL.W = TR.W;
    CLOCK_CYCLES(8);
case 0x3A: /* LD A, (HL-) */
    REG_A = READ(HL.W--);   
    CLOCK_CYCLES(8);
case 0x3B: /* DEC SP */
    SP.W--;
    CLOCK_CYCLES(8);
case 0x3C: /* INC A */
    REG_A++;
    REG_F = (REG_A ? 0 : ZF) | ((REG_A & 0xF) ? 0 : HF) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x3D: /* DEC A */
    REG_A--;
    REG_F = (REG_A ? 0 : ZF) | NF | ((REG_A & 0xF) == 0xF ? HF : 0) | (REG_F & CF);
    CLOCK_CYCLES(4);
case 0x3E: /* LD A, d8 */
    REG_A = READ(PC.W++);
    CLOCK_CYCLES(8);
case 0x3F: /* CCF */
    REG_F = (REG_F & ZF) | (~(REG_F & CF) & CF);
    CLOCK_CYCLES(4);
    
case 0x40: REG_B = REG_B; CLOCK_CYCLES(4);
case 0x41: REG_B = REG_C; CLOCK_CYCLES(4);
case 0x42: REG_B = REG_D; CLOCK_CYCLES(4);
case 0x43: REG_B = REG_E; CLOCK_CYCLES(4);
case 0x44: REG_B = REG_H; CLOCK_CYCLES(4);
case 0x45: REG_B = REG_L; CLOCK_CYCLES(4);
case 0x46: REG_B = READ(HL.W); CLOCK_CYCLES(8);
case 0x47: REG_B = REG_A; CLOCK_CYCLES(4);

case 0x48: REG_C = REG_B; CLOCK_CYCLES(4);
case 0x49: REG_C = REG_C; CLOCK_CYCLES(4);
case 0x4A: REG_C = REG_D; CLOCK_CYCLES(4);
case 0x4B: REG_C = REG_E; CLOCK_CYCLES(4);
case 0x4C: REG_C = REG_H; CLOCK_CYCLES(4);
case 0x4D: REG_C = REG_L; CLOCK_CYCLES(4);
case 0x4E: REG_C = READ(HL.W); CLOCK_CYCLES(8);
case 0x4F: REG_C = REG_A; CLOCK_CYCLES(4);

case 0x50: REG_D = REG_B; CLOCK_CYCLES(4);
case 0x51: REG_D = REG_C; CLOCK_CYCLES(4);
case 0x52: REG_D = REG_D; CLOCK_CYCLES(4);
case 0x53: REG_D = REG_E; CLOCK_CYCLES(4);
case 0x54: REG_D = REG_H; CLOCK_CYCLES(4);
case 0x55: REG_D = REG_L; CLOCK_CYCLES(4);
case 0x56: REG_D = READ(HL.W); CLOCK_CYCLES(8);
case 0x57: REG_D = REG_A; CLOCK_CYCLES(4);

case 0x58: REG_E = REG_B; CLOCK_CYCLES(4);
case 0x59: REG_E = REG_C; CLOCK_CYCLES(4);
case 0x5A: REG_E = REG_D; CLOCK_CYCLES(4);
case 0x5B: REG_E = REG_E; CLOCK_CYCLES(4);
case 0x5C: REG_E = REG_H; CLOCK_CYCLES(4);
case 0x5D: REG_E = REG_L; CLOCK_CYCLES(4);
case 0x5E: REG_E = READ(HL.W); CLOCK_CYCLES(8);
case 0x5F: REG_E = REG_A; CLOCK_CYCLES(4);

case 0x60: REG_H = REG_B; CLOCK_CYCLES(4);
case 0x61: REG_H = REG_C; CLOCK_CYCLES(4);
case 0x62: REG_H = REG_D; CLOCK_CYCLES(4);
case 0x63: REG_H = REG_E; CLOCK_CYCLES(4);
case 0x64: REG_H = REG_H; CLOCK_CYCLES(4);
case 0x65: REG_H = REG_L; CLOCK_CYCLES(4);
case 0x66: REG_H = READ(HL.W); CLOCK_CYCLES(8);
case 0x67: REG_H = REG_A; CLOCK_CYCLES(4);

case 0x68: REG_L = REG_B; CLOCK_CYCLES(4);
case 0x69: REG_L = REG_C; CLOCK_CYCLES(4);
case 0x6A: REG_L = REG_D; CLOCK_CYCLES(4);
case 0x6B: REG_L = REG_E; CLOCK_CYCLES(4);
case 0x6C: REG_L = REG_H; CLOCK_CYCLES(4);
case 0x6D: REG_L = REG_L; CLOCK_CYCLES(4);
case 0x6E: REG_L = READ(HL.W); CLOCK_CYCLES(8);
case 0x6F: REG_L = REG_A; CLOCK_CYCLES(4);

case 0x70: WRITE(HL.W, REG_B); CLOCK_CYCLES(8);
case 0x71: WRITE(HL.W, REG_C); CLOCK_CYCLES(8);
case 0x72: WRITE(HL.W, REG_D); CLOCK_CYCLES(8);
case 0x73: WRITE(HL.W, REG_E); CLOCK_CYCLES(8);
case 0x74: WRITE(HL.W, REG_H); CLOCK_CYCLES(8);
case 0x75: WRITE(HL.W, REG_L); CLOCK_CYCLES(8);

case 0x76: //HALT
    waiting_for_interrupt = true;
    CLOCK_CYCLES(4);
    
case 0x77: WRITE(HL.W, REG_A); CLOCK_CYCLES(8);

case 0x78: REG_A = REG_B; CLOCK_CYCLES(4);
case 0x79: REG_A = REG_C; CLOCK_CYCLES(4);
case 0x7A: REG_A = REG_D; CLOCK_CYCLES(4);
case 0x7B: REG_A = REG_E; CLOCK_CYCLES(4);
case 0x7C: REG_A = REG_H; CLOCK_CYCLES(4);
case 0x7D: REG_A = REG_L; CLOCK_CYCLES(4);
case 0x7E: REG_A = READ(HL.W); CLOCK_CYCLES(8);
case 0x7F: REG_A = REG_A; CLOCK_CYCLES(4);

case 0x80: ADD_BYTES(REG_A, REG_B); CLOCK_CYCLES(4);
case 0x81: ADD_BYTES(REG_A, REG_C); CLOCK_CYCLES(4);
case 0x82: ADD_BYTES(REG_A, REG_D); CLOCK_CYCLES(4);
case 0x83: ADD_BYTES(REG_A, REG_E); CLOCK_CYCLES(4);
case 0x84: ADD_BYTES(REG_A, REG_H); CLOCK_CYCLES(4);
case 0x85: ADD_BYTES(REG_A, REG_L); CLOCK_CYCLES(4);
case 0x86: temp = READ(HL.W); ADD_BYTES(REG_A, temp); CLOCK_CYCLES(8);
case 0x87: ADD_BYTES(REG_A, REG_A); CLOCK_CYCLES(4);
    
case 0x88: ADC_BYTES(REG_A, REG_B); CLOCK_CYCLES(4);
case 0x89: ADC_BYTES(REG_A, REG_C); CLOCK_CYCLES(4);
case 0x8A: ADC_BYTES(REG_A, REG_D); CLOCK_CYCLES(4);
case 0x8B: ADC_BYTES(REG_A, REG_E); CLOCK_CYCLES(4);
case 0x8C: ADC_BYTES(REG_A, REG_H); CLOCK_CYCLES(4);
case 0x8D: ADC_BYTES(REG_A, REG_L); CLOCK_CYCLES(4);
case 0x8E: temp = READ(HL.W); ADC_BYTES(REG_A, temp); CLOCK_CYCLES(8);
case 0x8F: ADC_BYTES(REG_A, REG_A); CLOCK_CYCLES(4);
    
case 0x90: SUB_BYTES(REG_A, REG_B); CLOCK_CYCLES(4);
case 0x91: SUB_BYTES(REG_A, REG_C); CLOCK_CYCLES(4);
case 0x92: SUB_BYTES(REG_A, REG_D); CLOCK_CYCLES(4);
case 0x93: SUB_BYTES(REG_A, REG_E); CLOCK_CYCLES(4);
case 0x94: SUB_BYTES(REG_A, REG_H); CLOCK_CYCLES(4);
case 0x95: SUB_BYTES(REG_A, REG_L); CLOCK_CYCLES(4);
case 0x96: temp = READ(HL.W); SUB_BYTES(REG_A, temp); CLOCK_CYCLES(8);
case 0x97: SUB_BYTES(REG_A, REG_A); CLOCK_CYCLES(4);

case 0x98: SBC_BYTES(REG_A, REG_B); CLOCK_CYCLES(4);
case 0x99: SBC_BYTES(REG_A, REG_C); CLOCK_CYCLES(4);
case 0x9A: SBC_BYTES(REG_A, REG_D); CLOCK_CYCLES(4);
case 0x9B: SBC_BYTES(REG_A, REG_E); CLOCK_CYCLES(4);
case 0x9C: SBC_BYTES(REG_A, REG_H); CLOCK_CYCLES(4);
case 0x9D: SBC_BYTES(REG_A, REG_L); CLOCK_CYCLES(4);
case 0x9E: temp = READ(HL.W); SBC_BYTES(REG_A, temp); CLOCK_CYCLES(8);
case 0x9F: SBC_BYTES(REG_A, REG_A); CLOCK_CYCLES(4);
    
case 0xA0: AND_BYTE(REG_B); CLOCK_CYCLES(4);
case 0xA1: AND_BYTE(REG_C); CLOCK_CYCLES(4);
case 0xA2: AND_BYTE(REG_D); CLOCK_CYCLES(4);
case 0xA3: AND_BYTE(REG_E); CLOCK_CYCLES(4);
case 0xA4: AND_BYTE(REG_H); CLOCK_CYCLES(4);
case 0xA5: AND_BYTE(REG_L); CLOCK_CYCLES(4);
case 0xA6: TR.HI = READ(HL.W); AND_BYTE(TR.HI); CLOCK_CYCLES(8);
case 0xA7: AND_BYTE(REG_A); CLOCK_CYCLES(4);

case 0xA8: XOR_BYTE(REG_B); CLOCK_CYCLES(4);
case 0xA9: XOR_BYTE(REG_C); CLOCK_CYCLES(4);
case 0xAA: XOR_BYTE(REG_D); CLOCK_CYCLES(4);
case 0xAB: XOR_BYTE(REG_E); CLOCK_CYCLES(4);
case 0xAC: XOR_BYTE(REG_H); CLOCK_CYCLES(4);
case 0xAD: XOR_BYTE(REG_L); CLOCK_CYCLES(4);
case 0xAE: TR.HI = READ(HL.W); XOR_BYTE(TR.HI); CLOCK_CYCLES(8);
case 0xAF: XOR_BYTE(REG_A); CLOCK_CYCLES(4);

case 0xB0: OR_BYTE(REG_B); CLOCK_CYCLES(4);
case 0xB1: OR_BYTE(REG_C); CLOCK_CYCLES(4);
case 0xB2: OR_BYTE(REG_D); CLOCK_CYCLES(4);
case 0xB3: OR_BYTE(REG_E); CLOCK_CYCLES(4);
case 0xB4: OR_BYTE(REG_H); CLOCK_CYCLES(4);
case 0xB5: OR_BYTE(REG_L); CLOCK_CYCLES(4);
case 0xB6: TR.HI = READ(HL.W); OR_BYTE(TR.HI); CLOCK_CYCLES(8);
case 0xB7: OR_BYTE(REG_A); CLOCK_CYCLES(4);
    
case 0xB8: CP_BYTES(REG_A, REG_B); CLOCK_CYCLES(4);
case 0xB9: CP_BYTES(REG_A, REG_C); CLOCK_CYCLES(4);
case 0xBA: CP_BYTES(REG_A, REG_D); CLOCK_CYCLES(4);
case 0xBB: CP_BYTES(REG_A, REG_E); CLOCK_CYCLES(4);
case 0xBC: CP_BYTES(REG_A, REG_H); CLOCK_CYCLES(4);
case 0xBD: CP_BYTES(REG_A, REG_L); CLOCK_CYCLES(4);

case 0xBE: temp = READ(HL.W); CP_BYTES(REG_A, temp); CLOCK_CYCLES(8);
case 0xBF: CP_BYTES(REG_A, REG_A); CLOCK_CYCLES(4);
    
case 0xC0: //RET NZ
    if(REG_F & ZF) {
        CLOCK_CYCLES(8);
    } else {
        PC.LO = READ(SP.W++);
        PC.HI = READ(SP.W++);
        CLOCK_CYCLES(20);
    }
case 0xC1: //POP BC
    REG_C = READ(SP.W++);
    REG_B = READ(SP.W++);
    CLOCK_CYCLES(12);
case 0xC2: //JP NZ, a16
    if(REG_F & ZF) {
        PC.W += 2;
        CLOCK_CYCLES(12);
    } else {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W);
        PC.W = TR.W;
        CLOCK_CYCLES(16);
    }
case 0xC3: //JP a16
    TR.LO = READ(PC.W++);
    TR.HI = READ(PC.W);
    PC.W = TR.W;
    CLOCK_CYCLES(16);
case 0xC4: //CALL NZ, a16
    if(REG_F & ZF) {
        PC.W += 2;
        CLOCK_CYCLES(12);
    } else {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W++);
        WRITE(--SP.W, PC.HI);
        WRITE(--SP.W, PC.LO);
        PC.W = TR.W;
        CLOCK_CYCLES(24);
    }
case 0xC5: //PUSH BC
    WRITE(--SP.W, REG_B);
    WRITE(--SP.W, REG_C);
    CLOCK_CYCLES(16);
case 0xC6: //ADD A, d8
    TR.HI = READ(PC.W++);
    TR.LO = REG_A + TR.HI;
    REG_F = (TR.LO ? 0 : ZF) | (((REG_A ^ TR.HI ^ TR.LO) & 0x10) ? HF : 0) | (TR.LO < TR.HI ? CF : 0);
    REG_A = TR.LO;
    CLOCK_CYCLES(8);
case 0xC7: //RST 00H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0;
    CLOCK_CYCLES(16);
case 0xC8: //RET Z
    if(REG_F & ZF) {
        PC.LO = READ(SP.W++);
        PC.HI = READ(SP.W++);
        CLOCK_CYCLES(20);
    } else {
        CLOCK_CYCLES(8);
    }
case 0xC9: //RET
    PC.LO = READ(SP.W++);
    PC.HI = READ(SP.W++);
    CLOCK_CYCLES(16);
case 0xCA: //JP Z, a16
    if(REG_F & ZF) {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W);
        PC.W = TR.W;
        CLOCK_CYCLES(16);
    } else {
        PC.W += 2;
        CLOCK_CYCLES(12);
    }
case 0xCB: //PREFIX CB
{
    IR.W = READ(PC.W++);
    switch(IR.W) {
case 0x00: RLC(REG_B); CLOCK_CYCLES(8);
case 0x01: RLC(REG_C); CLOCK_CYCLES(8);
case 0x02: RLC(REG_D); CLOCK_CYCLES(8);
case 0x03: RLC(REG_E); CLOCK_CYCLES(8);
case 0x04: RLC(REG_H); CLOCK_CYCLES(8);
case 0x05: RLC(REG_L); CLOCK_CYCLES(8);
case 0x06: TR.HI = READ(HL.W); RLC(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x07: RLC(REG_A); CLOCK_CYCLES(8);

case 0x08: RRC(REG_B); CLOCK_CYCLES(8);
case 0x09: RRC(REG_C); CLOCK_CYCLES(8);
case 0x0A: RRC(REG_D); CLOCK_CYCLES(8);
case 0x0B: RRC(REG_E); CLOCK_CYCLES(8);
case 0x0C: RRC(REG_H); CLOCK_CYCLES(8);
case 0x0D: RRC(REG_L); CLOCK_CYCLES(8);
case 0x0E: TR.HI = READ(HL.W); RRC(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x0F: RRC(REG_A); CLOCK_CYCLES(8);

case 0x10: RL(REG_B); CLOCK_CYCLES(8);
case 0x11: RL(REG_C); CLOCK_CYCLES(8);
case 0x12: RL(REG_D); CLOCK_CYCLES(8);
case 0x13: RL(REG_E); CLOCK_CYCLES(8);
case 0x14: RL(REG_H); CLOCK_CYCLES(8);
case 0x15: RL(REG_L); CLOCK_CYCLES(8);
case 0x16: TR.HI = READ(HL.W); RL(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x17: RL(REG_A); CLOCK_CYCLES(8);

case 0x18: RR(REG_B); CLOCK_CYCLES(8);
case 0x19: RR(REG_C); CLOCK_CYCLES(8);
case 0x1A: RR(REG_D); CLOCK_CYCLES(8);
case 0x1B: RR(REG_E); CLOCK_CYCLES(8);
case 0x1C: RR(REG_H); CLOCK_CYCLES(8);
case 0x1D: RR(REG_L); CLOCK_CYCLES(8);
case 0x1E: TR.HI = READ(HL.W); RR(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x1F: RR(REG_A); CLOCK_CYCLES(8);

case 0x20: SLA(REG_B); CLOCK_CYCLES(8);
case 0x21: SLA(REG_C); CLOCK_CYCLES(8);
case 0x22: SLA(REG_D); CLOCK_CYCLES(8);
case 0x23: SLA(REG_E); CLOCK_CYCLES(8);
case 0x24: SLA(REG_H); CLOCK_CYCLES(8);
case 0x25: SLA(REG_L); CLOCK_CYCLES(8);
case 0x26: TR.HI = READ(HL.W); SLA(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x27: SLA(REG_A); CLOCK_CYCLES(8);

case 0x28: SRA(REG_B); CLOCK_CYCLES(8);
case 0x29: SRA(REG_C); CLOCK_CYCLES(8);
case 0x2A: SRA(REG_D); CLOCK_CYCLES(8); 
case 0x2B: SRA(REG_E); CLOCK_CYCLES(8); 
case 0x2C: SRA(REG_H); CLOCK_CYCLES(8); 
case 0x2D: SRA(REG_L); CLOCK_CYCLES(8);
case 0x2E: TR.HI = READ(HL.W); SRA(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x2F: SRA(REG_A); CLOCK_CYCLES(8);

case 0x30: SWAP(REG_B); CLOCK_CYCLES(8);
case 0x31: SWAP(REG_C); CLOCK_CYCLES(8);
case 0x32: SWAP(REG_D); CLOCK_CYCLES(8);
case 0x33: SWAP(REG_E); CLOCK_CYCLES(8);
case 0x34: SWAP(REG_H); CLOCK_CYCLES(8);
case 0x35: SWAP(REG_L); CLOCK_CYCLES(8);
case 0x36: TR.HI = READ(HL.W); SWAP(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x37: SWAP(REG_A); CLOCK_CYCLES(8);

case 0x38: SRL(REG_B); CLOCK_CYCLES(8);
case 0x39: SRL(REG_C); CLOCK_CYCLES(8);
case 0x3A: SRL(REG_D); CLOCK_CYCLES(8);
case 0x3B: SRL(REG_E); CLOCK_CYCLES(8);
case 0x3C: SRL(REG_H); CLOCK_CYCLES(8);
case 0x3D: SRL(REG_L); CLOCK_CYCLES(8);
case 0x3E: TR.HI = READ(HL.W); SRL(TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x3F: SRL(REG_A); CLOCK_CYCLES(8);
    
case 0x40: BIT(0, REG_B); CLOCK_CYCLES(8);
case 0x41: BIT(0, REG_C); CLOCK_CYCLES(8);
case 0x42: BIT(0, REG_D); CLOCK_CYCLES(8);
case 0x43: BIT(0, REG_E); CLOCK_CYCLES(8);
case 0x44: BIT(0, REG_H); CLOCK_CYCLES(8);
case 0x45: BIT(0, REG_L); CLOCK_CYCLES(8);
case 0x46: TR.HI = READ(HL.W); BIT(0, TR.HI); CLOCK_CYCLES(16);
case 0x47: BIT(0, REG_A); CLOCK_CYCLES(8);

case 0x48: BIT(1, REG_B); CLOCK_CYCLES(8);
case 0x49: BIT(1, REG_C); CLOCK_CYCLES(8);
case 0x4A: BIT(1, REG_D); CLOCK_CYCLES(8);
case 0x4B: BIT(1, REG_E); CLOCK_CYCLES(8);
case 0x4C: BIT(1, REG_H); CLOCK_CYCLES(8);
case 0x4D: BIT(1, REG_L); CLOCK_CYCLES(8);
case 0x4E: TR.HI = READ(HL.W); BIT(1, TR.HI); CLOCK_CYCLES(16);
case 0x4F: BIT(1, REG_A); CLOCK_CYCLES(8);

case 0x50: BIT(2, REG_B); CLOCK_CYCLES(8);
case 0x51: BIT(2, REG_C); CLOCK_CYCLES(8);
case 0x52: BIT(2, REG_D); CLOCK_CYCLES(8);
case 0x53: BIT(2, REG_E); CLOCK_CYCLES(8);
case 0x54: BIT(2, REG_H); CLOCK_CYCLES(8);
case 0x55: BIT(2, REG_L); CLOCK_CYCLES(8);
case 0x56: TR.HI = READ(HL.W); BIT(2, TR.HI); CLOCK_CYCLES(16);
case 0x57: BIT(2, REG_A); CLOCK_CYCLES(8);

case 0x58: BIT(3, REG_B); CLOCK_CYCLES(8);
case 0x59: BIT(3, REG_C); CLOCK_CYCLES(8);
case 0x5A: BIT(3, REG_D); CLOCK_CYCLES(8);
case 0x5B: BIT(3, REG_E); CLOCK_CYCLES(8);
case 0x5C: BIT(3, REG_H); CLOCK_CYCLES(8);
case 0x5D: BIT(3, REG_L); CLOCK_CYCLES(8);
case 0x5E: TR.HI = READ(HL.W); BIT(3, TR.HI); CLOCK_CYCLES(16);
case 0x5F: BIT(3, REG_A); CLOCK_CYCLES(8);

case 0x60: BIT(4, REG_B); CLOCK_CYCLES(8);
case 0x61: BIT(4, REG_C); CLOCK_CYCLES(8);
case 0x62: BIT(4, REG_D); CLOCK_CYCLES(8);
case 0x63: BIT(4, REG_E); CLOCK_CYCLES(8);
case 0x64: BIT(4, REG_H); CLOCK_CYCLES(8);
case 0x65: BIT(4, REG_L); CLOCK_CYCLES(8);
case 0x66: TR.HI = READ(HL.W); BIT(4, TR.HI); CLOCK_CYCLES(16);
case 0x67: BIT(4, REG_A); CLOCK_CYCLES(8);

case 0x68: BIT(5, REG_B); CLOCK_CYCLES(8);
case 0x69: BIT(5, REG_C); CLOCK_CYCLES(8);
case 0x6A: BIT(5, REG_D); CLOCK_CYCLES(8);
case 0x6B: BIT(5, REG_E); CLOCK_CYCLES(8);
case 0x6C: BIT(5, REG_H); CLOCK_CYCLES(8);
case 0x6D: BIT(5, REG_L); CLOCK_CYCLES(8);
case 0x6E: TR.HI = READ(HL.W); BIT(5, TR.HI); CLOCK_CYCLES(16);
case 0x6F: BIT(5, REG_A); CLOCK_CYCLES(8);

case 0x70: BIT(6, REG_B); CLOCK_CYCLES(8);
case 0x71: BIT(6, REG_C); CLOCK_CYCLES(8);
case 0x72: BIT(6, REG_D); CLOCK_CYCLES(8);
case 0x73: BIT(6, REG_E); CLOCK_CYCLES(8);
case 0x74: BIT(6, REG_H); CLOCK_CYCLES(8);
case 0x75: BIT(6, REG_L); CLOCK_CYCLES(8);
case 0x76: TR.HI = READ(HL.W); BIT(6, TR.HI); CLOCK_CYCLES(16);
case 0x77: BIT(6, REG_A); CLOCK_CYCLES(8);

case 0x78: BIT(7, REG_B); CLOCK_CYCLES(8);
case 0x79: BIT(7, REG_C); CLOCK_CYCLES(8);
case 0x7A: BIT(7, REG_D); CLOCK_CYCLES(8);
case 0x7B: BIT(7, REG_E); CLOCK_CYCLES(8);
case 0x7C: BIT(7, REG_H); CLOCK_CYCLES(8);
case 0x7D: BIT(7, REG_L); CLOCK_CYCLES(8);
case 0x7E: TR.HI = READ(HL.W); BIT(7, TR.HI); CLOCK_CYCLES(16);
case 0x7F: BIT(7, REG_A); CLOCK_CYCLES(8);
    
case 0x80: RES(0, REG_B); CLOCK_CYCLES(8);
case 0x81: RES(0, REG_C); CLOCK_CYCLES(8);
case 0x82: RES(0, REG_D); CLOCK_CYCLES(8);
case 0x83: RES(0, REG_E); CLOCK_CYCLES(8);
case 0x84: RES(0, REG_H); CLOCK_CYCLES(8);
case 0x85: RES(0, REG_L); CLOCK_CYCLES(8);
case 0x86: TR.HI = READ(HL.W); RES(0, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x87:  RES(0, REG_A); CLOCK_CYCLES(8);

case 0x88: RES(1, REG_B); CLOCK_CYCLES(8);
case 0x89: RES(1, REG_C); CLOCK_CYCLES(8);
case 0x8A: RES(1, REG_D); CLOCK_CYCLES(8);
case 0x8B: RES(1, REG_E); CLOCK_CYCLES(8);
case 0x8C: RES(1, REG_H); CLOCK_CYCLES(8);
case 0x8D: RES(1, REG_L); CLOCK_CYCLES(8);
case 0x8E: TR.HI = READ(HL.W); RES(1, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x8F: RES(1, REG_A); CLOCK_CYCLES(8);

case 0x90: RES(2, REG_B); CLOCK_CYCLES(8);
case 0x91: RES(2, REG_C); CLOCK_CYCLES(8);
case 0x92: RES(2, REG_D); CLOCK_CYCLES(8);
case 0x93: RES(2, REG_E); CLOCK_CYCLES(8);
case 0x94: RES(2, REG_H); CLOCK_CYCLES(8);
case 0x95: RES(2, REG_L); CLOCK_CYCLES(8);
case 0x96: TR.HI = READ(HL.W); RES(2, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x97: RES(2, REG_A); CLOCK_CYCLES(8);

case 0x98: RES(3, REG_B); CLOCK_CYCLES(8);
case 0x99: RES(3, REG_C); CLOCK_CYCLES(8);
case 0x9A: RES(3, REG_D); CLOCK_CYCLES(8);
case 0x9B: RES(3, REG_E); CLOCK_CYCLES(8);
case 0x9C: RES(3, REG_H); CLOCK_CYCLES(8);
case 0x9D: RES(3, REG_L); CLOCK_CYCLES(8);
case 0x9E: TR.HI = READ(HL.W); RES(3, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0x9F: RES(3, REG_A); CLOCK_CYCLES(8);

case 0xA0: RES(4, REG_B); CLOCK_CYCLES(8);
case 0xA1: RES(4, REG_C); CLOCK_CYCLES(8);
case 0xA2: RES(4, REG_D); CLOCK_CYCLES(8);
case 0xA3: RES(4, REG_E); CLOCK_CYCLES(8);
case 0xA4: RES(4, REG_H); CLOCK_CYCLES(8);
case 0xA5: RES(4, REG_L); CLOCK_CYCLES(8);
case 0xA6: TR.HI = READ(HL.W); RES(4, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xA7: RES(4, REG_A); CLOCK_CYCLES(8);

case 0xA8: RES(5, REG_B); CLOCK_CYCLES(8);
case 0xA9: RES(5, REG_C); CLOCK_CYCLES(8);
case 0xAA: RES(5, REG_D); CLOCK_CYCLES(8);
case 0xAB: RES(5, REG_E); CLOCK_CYCLES(8);
case 0xAC: RES(5, REG_H); CLOCK_CYCLES(8);
case 0xAD: RES(5, REG_L); CLOCK_CYCLES(8);
case 0xAE: TR.HI = READ(HL.W); RES(5, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xAF: RES(5, REG_A); CLOCK_CYCLES(8);

case 0xB0: RES(6, REG_B); CLOCK_CYCLES(8);
case 0xB1: RES(6, REG_C); CLOCK_CYCLES(8);
case 0xB2: RES(6, REG_D); CLOCK_CYCLES(8);
case 0xB3: RES(6, REG_E); CLOCK_CYCLES(8);
case 0xB4: RES(6, REG_H); CLOCK_CYCLES(8);
case 0xB5: RES(6, REG_L); CLOCK_CYCLES(8);
case 0xB6: TR.HI = READ(HL.W); RES(6, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xB7: RES(6, REG_A); CLOCK_CYCLES(8);

case 0xB8: RES(7, REG_B); CLOCK_CYCLES(8);
case 0xB9: RES(7, REG_C); CLOCK_CYCLES(8);
case 0xBA: RES(7, REG_D); CLOCK_CYCLES(8);
case 0xBB: RES(7, REG_E); CLOCK_CYCLES(8);
case 0xBC: RES(7, REG_H); CLOCK_CYCLES(8);
case 0xBD: RES(7, REG_L); CLOCK_CYCLES(8);
case 0xBE: TR.HI = READ(HL.W); RES(7, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xBF: RES(7, REG_A); CLOCK_CYCLES(8);
    
case 0xC0: SET(0, REG_B); CLOCK_CYCLES(8);
case 0xC1: SET(0, REG_C); CLOCK_CYCLES(8);
case 0xC2: SET(0, REG_D); CLOCK_CYCLES(8);
case 0xC3: SET(0, REG_E); CLOCK_CYCLES(8);
case 0xC4: SET(0, REG_H); CLOCK_CYCLES(8);
case 0xC5: SET(0, REG_L); CLOCK_CYCLES(8);
case 0xC6: TR.HI = READ(HL.W); SET(0, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xC7: SET(0, REG_A); CLOCK_CYCLES(8);

case 0xC8: SET(1, REG_B); CLOCK_CYCLES(8);
case 0xC9: SET(1, REG_C); CLOCK_CYCLES(8);
case 0xCA: SET(1, REG_D); CLOCK_CYCLES(8);
case 0xCB: SET(1, REG_E); CLOCK_CYCLES(8);
case 0xCC: SET(1, REG_H); CLOCK_CYCLES(8);
case 0xCD: SET(1, REG_L); CLOCK_CYCLES(8);
case 0xCE: TR.HI = READ(HL.W); SET(1, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xCF: SET(1, REG_A); CLOCK_CYCLES(8);

case 0xD0: SET(2, REG_B); CLOCK_CYCLES(8);
case 0xD1: SET(2, REG_C); CLOCK_CYCLES(8);
case 0xD2: SET(2, REG_D); CLOCK_CYCLES(8);
case 0xD3: SET(2, REG_E); CLOCK_CYCLES(8);
case 0xD4: SET(2, REG_H); CLOCK_CYCLES(8);
case 0xD5: SET(2, REG_L); CLOCK_CYCLES(8);
case 0xD6: TR.HI = READ(HL.W); SET(2, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xD7: SET(2, REG_A); CLOCK_CYCLES(8);

case 0xD8: SET(3, REG_B); CLOCK_CYCLES(8);
case 0xD9: SET(3, REG_C); CLOCK_CYCLES(8);
case 0xDA: SET(3, REG_D); CLOCK_CYCLES(8);
case 0xDB: SET(3, REG_E); CLOCK_CYCLES(8);
case 0xDC: SET(3, REG_H); CLOCK_CYCLES(8);
case 0xDD: SET(3, REG_L); CLOCK_CYCLES(8);
case 0xDE: TR.HI = READ(HL.W); SET(3, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xDF: SET(3, REG_A); CLOCK_CYCLES(8);

case 0xE0: SET(4, REG_B); CLOCK_CYCLES(8);
case 0xE1: SET(4, REG_C); CLOCK_CYCLES(8);
case 0xE2: SET(4, REG_D); CLOCK_CYCLES(8);
case 0xE3: SET(4, REG_E); CLOCK_CYCLES(8);
case 0xE4: SET(4, REG_H); CLOCK_CYCLES(8);
case 0xE5: SET(4, REG_L); CLOCK_CYCLES(8);
case 0xE6: TR.HI = READ(HL.W); SET(4, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xE7: SET(4, REG_A); CLOCK_CYCLES(8);

case 0xE8: SET(5, REG_B); CLOCK_CYCLES(8);
case 0xE9: SET(5, REG_C); CLOCK_CYCLES(8);
case 0xEA: SET(5, REG_D); CLOCK_CYCLES(8);
case 0xEB: SET(5, REG_E); CLOCK_CYCLES(8);
case 0xEC: SET(5, REG_H); CLOCK_CYCLES(8);
case 0xED: SET(5, REG_L); CLOCK_CYCLES(8);
case 0xEE: TR.HI = READ(HL.W); SET(5, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xEF: SET(5, REG_A); CLOCK_CYCLES(8);

case 0xF0: SET(6, REG_B); CLOCK_CYCLES(8);
case 0xF1: SET(6, REG_C); CLOCK_CYCLES(8);
case 0xF2: SET(6, REG_D); CLOCK_CYCLES(8);
case 0xF3: SET(6, REG_E); CLOCK_CYCLES(8);
case 0xF4: SET(6, REG_H); CLOCK_CYCLES(8);
case 0xF5: SET(6, REG_L); CLOCK_CYCLES(8);
case 0xF6: TR.HI = READ(HL.W); SET(6, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xF7: SET(6, REG_A); CLOCK_CYCLES(8);

case 0xF8: SET(7, REG_B); CLOCK_CYCLES(8);
case 0xF9: SET(7, REG_C); CLOCK_CYCLES(8);
case 0xFA: SET(7, REG_D); CLOCK_CYCLES(8);
case 0xFB: SET(7, REG_E); CLOCK_CYCLES(8);
case 0xFC: SET(7, REG_H); CLOCK_CYCLES(8);
case 0xFD: SET(7, REG_L); CLOCK_CYCLES(8);
case 0xFE: TR.HI = READ(HL.W); SET(7, TR.HI); WRITE(HL.W, TR.HI); CLOCK_CYCLES(16);
case 0xFF: SET(7, REG_A); CLOCK_CYCLES(8);
    } //switch
}
case 0xCC: //CALL Z, a16
    if(REG_F & ZF) {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W++);
        WRITE(--SP.W, PC.HI);
        WRITE(--SP.W, PC.LO);
        PC.W = TR.W;
        CLOCK_CYCLES(24);
    } else {
        PC.W += 2;
        CLOCK_CYCLES(12);
    }
case 0xCD: //CALL a16v
{
    TR.LO = READ(PC.W++);
    TR.HI = READ(PC.W++);
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.W = TR.W;
    CLOCK_CYCLES(24);
}
case 0xCE: //ADC A, d8
{
    u8 R1 = REG_A;
    u8 R2 = READ(PC.W++);
    
    TR.W = R1 + R2 + ((REG_F & CF) ? 1 : 0);
    REG_F = (TR.LO ? 0 : ZF) | 
            (((R1 ^ R2 ^ TR.LO) & 0x10) ? HF : 0) |
            (TR.HI ? CF : 0);
    R1=TR.LO;

    REG_A = R1;
    CLOCK_CYCLES(8); 
}
case 0xCF: //RST 08H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0x8;
    CLOCK_CYCLES(16);
case 0xD0: //RET NC
    if(REG_F & CF) {
        CLOCK_CYCLES(8);
    } else {
        PC.LO = READ(SP.W++);
        PC.HI = READ(SP.W++);
        CLOCK_CYCLES(20);
    }
case 0xD1: //POP DE
    REG_E = READ(SP.W++);
    REG_D = READ(SP.W++);
    CLOCK_CYCLES(12);
case 0xD2: //JP NC, a16
    if(REG_F & CF) {
        PC.W += 2;
        CLOCK_CYCLES(12);
    } else {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W);
        PC.W = TR.W;
        CLOCK_CYCLES(16);
    }
case 0xD3:
    INVALID_OPCODE(0xD3);
case 0xD4: //CALL NC, a16
    if(REG_F & CF) {
        PC.W += 2;
        CLOCK_CYCLES(12);
    } else {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W++);
        WRITE(--SP.W, PC.HI);
        WRITE(--SP.W, PC.LO);
        PC.W = TR.W;
        CLOCK_CYCLES(24);
    }
case 0xD5: //PUSH DE
    WRITE(--SP.W, REG_D);
    WRITE(--SP.W, REG_E);
    CLOCK_CYCLES(16);
case 0xD6: //SUB d8
    TR.HI = READ(PC.W++);
    REG_F = ((REG_A == TR.HI) ? ZF : 0) | NF | (((REG_A & 0xF) < (TR.HI & 0xF)) ? HF : 0) | ((REG_A < TR.HI) ? CF : 0);
    REG_A -= TR.HI;
    CLOCK_CYCLES(8);
case 0xD7: //RST 10H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0x10;
    CLOCK_CYCLES(16);
case 0xD8: //RET C
    if(REG_F & CF) {
        PC.LO = READ(SP.W++);
        PC.HI = READ(SP.W++);
        CLOCK_CYCLES(20);
    } else {
        CLOCK_CYCLES(8);
    }
case 0xD9: //RETI
    gb->ime_flag = 1;
    PC.LO = READ(SP.W++);
    PC.HI = READ(SP.W++);
    CLOCK_CYCLES(16);
case 0xDA: //JP C, a16
    if(REG_F & CF) {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W);
        PC.W = TR.W;
        CLOCK_CYCLES(16);
    } else {
        PC.W += 2;
        CLOCK_CYCLES(12);
    }
case 0xDB:
    INVALID_OPCODE(0xDB);
case 0xDC: //CALL C, a16
    if(REG_F & CF) {
        TR.LO = READ(PC.W++);
        TR.HI = READ(PC.W++);
        WRITE(--SP.W, PC.HI);
        WRITE(--SP.W, PC.LO);
        PC.W = TR.W;
        CLOCK_CYCLES(24);
    } else {
        PC.W += 2;
        CLOCK_CYCLES(12);
    }
case 0xDE: //SBC A, d8
{
    u8 R1 = REG_A;
    u8 R2 = READ(PC.W++);

    TR.W = R1 - R2 - ((REG_F & CF) ? 1 : 0);
    REG_F = ((TR.LO) ? 0 : ZF) |
        NF |
        (((R1^R2^TR.LO) & 0x10) ? HF : 0) |
        (TR.HI ? CF : 0);
    R1=TR.LO;

    REG_A = R1;
    CLOCK_CYCLES(8);
}
case 0xDF: //RST 18H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0x18;
    CLOCK_CYCLES(16);
case 0xE0: //LDH (a8), A
    WRITE(0xFF00+READ(PC.W++), REG_A);
    CLOCK_CYCLES(12);
case 0xE1: //POP HL
    REG_L = READ(SP.W++);
    REG_H = READ(SP.W++);
    CLOCK_CYCLES(12);
case 0xE2: //LD (C), A
    WRITE(0xFF00+REG_C, REG_A);
    CLOCK_CYCLES(8);
case 0xE3:
    INVALID_OPCODE(0xE3);
case 0xE4:
    INVALID_OPCODE(0xE4);
case 0xE5: //PUSH HL
    WRITE(--SP.W, REG_H);
    WRITE(--SP.W, REG_L);
    CLOCK_CYCLES(16);
case 0xE6: //AND d8
    TR.HI = READ(PC.W++);
    REG_A &= TR.HI;
    REG_F = (REG_A ? 0 : ZF) | HF;
    CLOCK_CYCLES(8);
case 0xE7: //RST 20H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0x20;
    CLOCK_CYCLES(16);
case 0xE8: //ADD SP, r8
{
    //TODO clean this up
    // http://stackoverflow.com/questions/5159603/gbz80-how-does-ld-hl-spe-affect-h-and-c-flags
    u8 D8 = READ(PC.W++);
    s8 S8 = ((D8 & 127) - (D8 & 128));
    u16 local_SP  = SP.W;
    local_SP += S8;

    REG_F = 0;
    if(S8 >= 0) {
        REG_F |= ((((SP.W & 0xFF) + (S8)) > 0xFF) ? CF : 0);
        REG_F |= ((((SP.W & 0x0F) + (S8 & 0x0F)) > 0xF) ? HF : 0);
    } else {
        REG_F |= (((local_SP & 0xFF) <= (SP.W & 0xFF)) ? CF : 0);
        REG_F |= (((local_SP & 0x0F) <= (SP.W & 0x0F)) ? HF : 0);
    }

    SP.W = local_SP;
    CLOCK_CYCLES(16);
}
case 0xE9: //JP (HL)
    PC.W = HL.W;
    CLOCK_CYCLES(4);
case 0xEA: //LD(a16), A
    TR.LO = READ(PC.W++);
    TR.HI = READ(PC.W++);
    WRITE(TR.W, REG_A);
    CLOCK_CYCLES(16);
case 0xEB:
    INVALID_OPCODE(0xEB);
case 0xEC:
    INVALID_OPCODE(0xEC);
case 0xED:
    INVALID_OPCODE(0xED);
case 0xEE: //XOR d8
    TR.HI = READ(PC.W++);
    REG_A ^= TR.HI;
    REG_F = (REG_A ? 0 : ZF);
    CLOCK_CYCLES(8);
case 0xEF: //RST 28H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0x28;
    CLOCK_CYCLES(16);
case 0xF0: //LDH A, (a8)
    REG_A = READ(0xFF00+READ(PC.W++));
    CLOCK_CYCLES(12);
case 0xF1: //POP AF
    REG_F = READ(SP.W++);
    //ADDED FOR TESTING
    REG_F &= 0xF0;
    REG_A = READ(SP.W++);
    CLOCK_CYCLES(12);
case 0xF2: //LD A, (C)
    REG_A = READ(0xFF00+REG_C);
    CLOCK_CYCLES(8);
case 0xF3: //DI
    gb->ime_flag = 0;
    CLOCK_CYCLES(4);
case 0xF4:
    INVALID_OPCODE(0xF4);
case 0xF5: //PUSH AF
    WRITE(--SP.W, REG_A);
    WRITE(--SP.W, REG_F);
    CLOCK_CYCLES(16);
case 0xF6: //OR d8
    REG_A |= READ(PC.W++);
    REG_F = (REG_A ? 0 : ZF);
    CLOCK_CYCLES(8);
case 0xF7: //RST 30H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0x30;
    CLOCK_CYCLES(16);
case 0xF8: //LD HL, SP + r8
{
    //TODO clean this up
    // http://stackoverflow.com/questions/5159603/gbz80-how-does-ld-hl-spe-affect-h-and-c-flags
    u8 D8 = READ(PC.W++);
    s8 S8 = ((D8 & 127) - (D8 & 128));
    u16 local_SP  = SP.W;
    local_SP += S8;

    REG_F = 0;
    if(S8 >= 0) {
        REG_F |= ((((SP.W & 0xFF) + (S8)) > 0xFF) ? CF : 0);
        REG_F |= ((((SP.W & 0x0F) + (S8 & 0x0F)) > 0xF) ? HF : 0);
    } else {
        REG_F |= (((local_SP & 0xFF) <= (SP.W & 0xFF)) ? CF : 0);
        REG_F |= (((local_SP & 0x0F) <= (SP.W & 0x0F)) ? HF : 0);
    }

    HL.W = local_SP;
    CLOCK_CYCLES(12);
}
case 0xF9: //LD SP, HL
    SP.W = HL.W;
    CLOCK_CYCLES(8);
case 0xFA: //LD A, (a16)
    TR.LO = READ(PC.W++);
    TR.HI = READ(PC.W++);
    REG_A = READ(TR.W);
    CLOCK_CYCLES(16);
case 0xFB: //EI
    gb->ime_flag = 1;
    CLOCK_CYCLES(4);
case 0xFC:
    INVALID_OPCODE(0xFC);
case 0xFD:
    INVALID_OPCODE(0xFD);
case 0xFE: //CP d8
    TR.HI = READ(PC.W++);
    REG_F = ((REG_A == TR.HI) ? ZF : 0) | NF | (((REG_A & 0xF) < (TR.HI & 0xF)) ? HF : 0) | ((REG_A < TR.HI) ? CF : 0);
    CLOCK_CYCLES(8);
case 0xFF: //RST 38H
    WRITE(--SP.W, PC.HI);
    WRITE(--SP.W, PC.LO);
    PC.HI = 0;
    PC.LO = 0x38;
    CLOCK_CYCLES(16);
