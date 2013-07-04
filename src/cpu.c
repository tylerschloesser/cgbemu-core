/*
//#include <sys/time.h>
#include <Windows.h>
*/

#include "debug.h"
#include "cpu.h"
#include "memory.h"
#include "graphics.h"

/* CPU Flags */
#define ZF 0x80
#define NF 0x40
#define HF 0x20
#define CF 0x10

/* TEMPORARY so we can use existing variables names */
#define waiting_for_interrupt 	cpu_state.waiting_for_interrupt
#define timer_counter 			cpu_state.timer_counter
//#define cpu_running 			cpu_state.running
#define scanline_counter 		cpu_state.scanline_counter
#define cycles 					cpu_state.cycles
#define mode_cycles 			cpu_state.mode_cycles
#define divider_counter 		cpu_state.divider_counter
#define PC 						cpu_state.PC
#define SP 						cpu_state.SP
#define BC 						cpu_state.BC
#define DE 						cpu_state.DE
#define HL 						cpu_state.HL
#define AF 						cpu_state.AF
#define IR 						cpu_state.IR

/* 8 bit register definitions */
#define REG_B BC.B.H
#define REG_C BC.B.L
#define REG_D DE.B.H
#define REG_E DE.B.L
#define REG_H HL.B.H
#define REG_L HL.B.L
#define REG_A AF.B.H
#define REG_F AF.B.L

/* high and low bit definitions */
#define HI B.H
#define LO B.L

/* memory access functions */
#define READ(S) MBC_read(S)
#define WRITE(D, B) MBC_write(D, B)

/* behavior at the end of each instruction */
#define CLOCK_CYCLES(x) return(x)

/* behavior on invalid opcode */
#define INVALID_OPCODE(X) 					\
	printf("Invalid opcode (%X)!\n", X); 	\
	display_cpu_values(); 					\
	getchar(); 								\
	exit(1)

/* CPU INSTRUCTIONS */
/* Bit manipulation instructions */
#define RES(B, R) R &= ~(1 << B)
#define SET(B, R) R |= (1 << B)
#define BIT(B, R) REG_F = (R & (1 << B) ? 0 : ZF) | HF | (REG_F & CF)

/* Bit shift right instructions */
#define RLC(R) 	R = (R << 1) | ((R & 0x80) >> 7); 				\
				REG_F = (R ? 0 : ZF) | (R & 1 ? CF : 0)
#define RRC(R) 	R = (R >> 1) | ((R & 1) << 7); 					\
				REG_F = (R ? 0 : ZF) | (R & 0x80 ? CF : 0)
#define RL(R) 	TR.LO = R; 										\
				R = (R << 1) | ((REG_F & CF) >> 4); 			\
				REG_F = (R ? 0 : ZF) | (TR.LO & 0x80 ? CF : 0)
#define RR(R) 	TR.LO = R; 										\
				R = (R >> 1) | ((REG_F & CF) << 3);				\
				REG_F = (R ? 0 : ZF) | (TR.LO & 1 ? CF : 0)

/* bit shift left instrucctions */
#define SLA(R) 	TR.LO = R; 										\
				R = (R << 1); 									\
				REG_F = (R ? 0 : ZF) | (TR.LO & 0x80 ? CF : 0)
#define SRA(R) 	TR.LO = R; 										\
				R = (R >> 1) | (R & 0x80);						\
				REG_F = (R ? 0 : ZF) | (TR.LO & 1 ? CF : 0)
#define SWAP(R) R = (R << 4) | (R >> 4); 						\
				REG_F = (R ? 0 : ZF)
#define SRL(R) 	TR.LO = R; 										\
				R = (R >> 1); 									\
				REG_F = (R ? 0 : ZF) | (TR.LO & 1 ? CF : 0)

/* add instructions */
#define ADD_BYTES(R1, R2) 										\
	TR.W = R1 + R2; 											\
	REG_F = (TR.LO ? 0 : ZF) | 									\
			(((R1 ^ R2 ^ TR.LO) & 0x10) ? HF : 0) | 			\
			(TR.HI ? CF :0); 								\
	R1 = TR.LO
	
/*
#define ADC_BYTES(R1, R2) 										\
	TR.LO = R1 + R2 + ((REG_F & CF) >> 4); 						\
	REG_F = (TR.LO ? 0 : ZF) | 									\
			(((R1 ^ R2 ^ TR.LO) & 0x10) ? HF : 0) | 			\
			(TR.LO < R2 ? CF : 0); 								\
	R1=TR.LO
*/
#define ADC_BYTES(R1, R2) 										\
	TR.W = R1 + R2 + ((REG_F & CF) ? 1 : 0); 						\
	REG_F = (TR.LO ? 0 : ZF) | 									\
			(((R1 ^ R2 ^ TR.LO) & 0x10) ? HF : 0) | 			\
			(TR.HI ? CF : 0);								\
	R1=TR.LO

/* subtract instructions */
/*
#define SUB_BYTES(R1, R2) 										\
	REG_F = ((R1 == R2) ? ZF : 0) | 							\
			NF | 												\
			(((R1 & 0xF) < (R2 & 0xF)) ? HF : 0) | 				\
			((R1 < R2) ? CF : 0); 								\
	R1-= R2
*/
#define SUB_BYTES(R1, R2) 										\
	TR.W = R1 - R2;												\
	REG_F = (TR.LO ? 0 : ZF) | 									\
			NF | 												\
			(((R1^R2^TR.LO)&0x10) ? HF : 0) | 					\
			(TR.HI ? CF : 0); 								\
	R1=TR.LO

/*
#define SBC_BYTES(R1, R2) 										\
	TR.HI = R2 + ((REG_F & CF) >> 4); 							\
	REG_F = ((R1 == TR.HI) ? ZF : 0) | 							\
		NF | 													\
		(((R1 & 0xF) < (TR.HI & 0xF)) ? HF : 0) | 				\
		((R1 < TR.HI) ? CF : 0); 								\
	R1-=TR.HI
*/
#define SBC_BYTES(R1, R2) 										\
	TR.W = R1 - R2 - ((REG_F & CF) ? 1 : 0); 					\
	REG_F = ((TR.LO) ? 0 : ZF) | 							\
		NF | 													\
		(((R1^R2^TR.LO)&0x10) ? HF : 0) | 				\
		(TR.HI ? CF : 0); 								\
	R1=TR.LO

/* bitwise operation instructions */
#define AND_BYTE(R) REG_A &= R; REG_F = (REG_A ? 0 : ZF) | HF
#define XOR_BYTE(R) REG_A ^= R; REG_F = (REG_A ? 0 : ZF)
#define OR_BYTE(R) 	REG_A |= R; REG_F = (REG_A ? 0 : ZF)

/* move instructions */
/*
#define CP_BYTES(R1, R2) 										\
	REG_F = ((R1 == R2) ? ZF : 0) | 							\
			NF | 												\
			(((R1 & 0xF) < (R2 & 0xF)) ? HF : 0) | 				\
			((R1 < R2) ? CF : 0)
*/
#define CP_BYTES(R1, R2) 										\
	TR.W=R1-R2;\
	REG_F = (TR.W ? 0 : ZF) | 							\
			NF | 												\
			(((R1^R2^TR.LO)&0x10) ? HF : 0) | 				\
			(TR.HI ? CF : 0)
			
#define LD_BYTE(R1, R2) R1 = R2
/* END CPU INSTRUCTIONS */

#define CLOCK_SPEED_HZ 4194304
//#define VBLANK_CYCLES CLOCK_SPEED_HZ / 60

#define GB_H_SYNC_KHZ 9198
#define GB_V_SYNC_HZ 59.73

/*
    how many cycles to wait until refreshing the screen 
    This is roughly CLOCK_SPEED_HZ / GB_V_SYNC_HZ
*/
#define SCREEN_REFRESH_CYCLES 70224


#define INITIAL_TIMER_COUNTER 1024
#define INITIAL_SCANLINE_COUNTER 456

#define DISPLAY_REFRESH_RATE_HZ 60

#define THREADSAFE_CPU

typedef enum {
	VBLANK,
	LCD_STATUS,
	TIMER,
	SERIAL,
	JOYPAD,
} InterruptType;

/* moved to header file
typedef enum {
	STOPPED,
	STARTED,
	PAUSED,
	INVALID,
} EmulatorState;
*/

static EmulatorState emulator_state = EM_INVALID;
static CpuState cpu_state;
static bool cpu_initialized = false;
/*
static SDL_mutex* cpu_mutex = NULL;
*/
//static struct timeval time_end, time_start;
static u64 freq = 0;
static u64 last_refresh = 0;

/*
static HANDLE emulator_state_changed = NULL;
*/

//static bool cpu_debugger_enabled = false;
/*
static Debugger *debugger = NULL;
*/


static u16 DAA_table[] = {
	0x0080,0x0100,0x0200,0x0300,0x0400,0x0500,0x0600,0x0700,
	0x0800,0x0900,0x1020,0x1120,0x1220,0x1320,0x1420,0x1520,
	0x1000,0x1100,0x1200,0x1300,0x1400,0x1500,0x1600,0x1700,
	0x1800,0x1900,0x2020,0x2120,0x2220,0x2320,0x2420,0x2520,
	0x2000,0x2100,0x2200,0x2300,0x2400,0x2500,0x2600,0x2700,
	0x2800,0x2900,0x3020,0x3120,0x3220,0x3320,0x3420,0x3520,
	0x3000,0x3100,0x3200,0x3300,0x3400,0x3500,0x3600,0x3700,
	0x3800,0x3900,0x4020,0x4120,0x4220,0x4320,0x4420,0x4520,
	0x4000,0x4100,0x4200,0x4300,0x4400,0x4500,0x4600,0x4700,
	0x4800,0x4900,0x5020,0x5120,0x5220,0x5320,0x5420,0x5520,
	0x5000,0x5100,0x5200,0x5300,0x5400,0x5500,0x5600,0x5700,
	0x5800,0x5900,0x6020,0x6120,0x6220,0x6320,0x6420,0x6520,
	0x6000,0x6100,0x6200,0x6300,0x6400,0x6500,0x6600,0x6700,
	0x6800,0x6900,0x7020,0x7120,0x7220,0x7320,0x7420,0x7520,
	0x7000,0x7100,0x7200,0x7300,0x7400,0x7500,0x7600,0x7700,
	0x7800,0x7900,0x8020,0x8120,0x8220,0x8320,0x8420,0x8520,
	0x8000,0x8100,0x8200,0x8300,0x8400,0x8500,0x8600,0x8700,
	0x8800,0x8900,0x9020,0x9120,0x9220,0x9320,0x9420,0x9520,
	0x9000,0x9100,0x9200,0x9300,0x9400,0x9500,0x9600,0x9700,
	0x9800,0x9900,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
	0x0810,0x0910,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
	0x1810,0x1910,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
	0x2810,0x2910,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
	0x3810,0x3910,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
	0x4810,0x4910,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
	0x5810,0x5910,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x6010,0x6110,0x6210,0x6310,0x6410,0x6510,0x6610,0x6710,
	0x6810,0x6910,0x7030,0x7130,0x7230,0x7330,0x7430,0x7530,
	0x7010,0x7110,0x7210,0x7310,0x7410,0x7510,0x7610,0x7710,
	0x7810,0x7910,0x8030,0x8130,0x8230,0x8330,0x8430,0x8530,
	0x8010,0x8110,0x8210,0x8310,0x8410,0x8510,0x8610,0x8710,
	0x8810,0x8910,0x9030,0x9130,0x9230,0x9330,0x9430,0x9530,
	0x9010,0x9110,0x9210,0x9310,0x9410,0x9510,0x9610,0x9710,
	0x9810,0x9910,0xA030,0xA130,0xA230,0xA330,0xA430,0xA530,
	0xA010,0xA110,0xA210,0xA310,0xA410,0xA510,0xA610,0xA710,
	0xA810,0xA910,0xB030,0xB130,0xB230,0xB330,0xB430,0xB530,
	0xB010,0xB110,0xB210,0xB310,0xB410,0xB510,0xB610,0xB710,
	0xB810,0xB910,0xC030,0xC130,0xC230,0xC330,0xC430,0xC530,
	0xC010,0xC110,0xC210,0xC310,0xC410,0xC510,0xC610,0xC710,
	0xC810,0xC910,0xD030,0xD130,0xD230,0xD330,0xD430,0xD530,
	0xD010,0xD110,0xD210,0xD310,0xD410,0xD510,0xD610,0xD710,
	0xD810,0xD910,0xE030,0xE130,0xE230,0xE330,0xE430,0xE530,
	0xE010,0xE110,0xE210,0xE310,0xE410,0xE510,0xE610,0xE710,
	0xE810,0xE910,0xF030,0xF130,0xF230,0xF330,0xF430,0xF530,
	0xF010,0xF110,0xF210,0xF310,0xF410,0xF510,0xF610,0xF710,
	0xF810,0xF910,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0090,0x0110,0x0210,0x0310,0x0410,0x0510,0x0610,0x0710,
	0x0810,0x0910,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1010,0x1110,0x1210,0x1310,0x1410,0x1510,0x1610,0x1710,
	0x1810,0x1910,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2010,0x2110,0x2210,0x2310,0x2410,0x2510,0x2610,0x2710,
	0x2810,0x2910,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3010,0x3110,0x3210,0x3310,0x3410,0x3510,0x3610,0x3710,
	0x3810,0x3910,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4010,0x4110,0x4210,0x4310,0x4410,0x4510,0x4610,0x4710,
	0x4810,0x4910,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5010,0x5110,0x5210,0x5310,0x5410,0x5510,0x5610,0x5710,
	0x5810,0x5910,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x0600,0x0700,0x0800,0x0900,0x0A00,0x0B00,0x0C00,0x0D00,
	0x0E00,0x0F00,0x1020,0x1120,0x1220,0x1320,0x1420,0x1520,
	0x1600,0x1700,0x1800,0x1900,0x1A00,0x1B00,0x1C00,0x1D00,
	0x1E00,0x1F00,0x2020,0x2120,0x2220,0x2320,0x2420,0x2520,
	0x2600,0x2700,0x2800,0x2900,0x2A00,0x2B00,0x2C00,0x2D00,
	0x2E00,0x2F00,0x3020,0x3120,0x3220,0x3320,0x3420,0x3520,
	0x3600,0x3700,0x3800,0x3900,0x3A00,0x3B00,0x3C00,0x3D00,
	0x3E00,0x3F00,0x4020,0x4120,0x4220,0x4320,0x4420,0x4520,
	0x4600,0x4700,0x4800,0x4900,0x4A00,0x4B00,0x4C00,0x4D00,
	0x4E00,0x4F00,0x5020,0x5120,0x5220,0x5320,0x5420,0x5520,
	0x5600,0x5700,0x5800,0x5900,0x5A00,0x5B00,0x5C00,0x5D00,
	0x5E00,0x5F00,0x6020,0x6120,0x6220,0x6320,0x6420,0x6520,
	0x6600,0x6700,0x6800,0x6900,0x6A00,0x6B00,0x6C00,0x6D00,
	0x6E00,0x6F00,0x7020,0x7120,0x7220,0x7320,0x7420,0x7520,
	0x7600,0x7700,0x7800,0x7900,0x7A00,0x7B00,0x7C00,0x7D00,
	0x7E00,0x7F00,0x8020,0x8120,0x8220,0x8320,0x8420,0x8520,
	0x8600,0x8700,0x8800,0x8900,0x8A00,0x8B00,0x8C00,0x8D00,
	0x8E00,0x8F00,0x9020,0x9120,0x9220,0x9320,0x9420,0x9520,
	0x9600,0x9700,0x9800,0x9900,0x9A00,0x9B00,0x9C00,0x9D00,
	0x9E00,0x9F00,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
	0x0E10,0x0F10,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
	0x1E10,0x1F10,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
	0x2E10,0x2F10,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
	0x3E10,0x3F10,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
	0x4E10,0x4F10,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
	0x5E10,0x5F10,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x6610,0x6710,0x6810,0x6910,0x6A10,0x6B10,0x6C10,0x6D10,
	0x6E10,0x6F10,0x7030,0x7130,0x7230,0x7330,0x7430,0x7530,
	0x7610,0x7710,0x7810,0x7910,0x7A10,0x7B10,0x7C10,0x7D10,
	0x7E10,0x7F10,0x8030,0x8130,0x8230,0x8330,0x8430,0x8530,
	0x8610,0x8710,0x8810,0x8910,0x8A10,0x8B10,0x8C10,0x8D10,
	0x8E10,0x8F10,0x9030,0x9130,0x9230,0x9330,0x9430,0x9530,
	0x9610,0x9710,0x9810,0x9910,0x9A10,0x9B10,0x9C10,0x9D10,
	0x9E10,0x9F10,0xA030,0xA130,0xA230,0xA330,0xA430,0xA530,
	0xA610,0xA710,0xA810,0xA910,0xAA10,0xAB10,0xAC10,0xAD10,
	0xAE10,0xAF10,0xB030,0xB130,0xB230,0xB330,0xB430,0xB530,
	0xB610,0xB710,0xB810,0xB910,0xBA10,0xBB10,0xBC10,0xBD10,
	0xBE10,0xBF10,0xC030,0xC130,0xC230,0xC330,0xC430,0xC530,
	0xC610,0xC710,0xC810,0xC910,0xCA10,0xCB10,0xCC10,0xCD10,
	0xCE10,0xCF10,0xD030,0xD130,0xD230,0xD330,0xD430,0xD530,
	0xD610,0xD710,0xD810,0xD910,0xDA10,0xDB10,0xDC10,0xDD10,
	0xDE10,0xDF10,0xE030,0xE130,0xE230,0xE330,0xE430,0xE530,
	0xE610,0xE710,0xE810,0xE910,0xEA10,0xEB10,0xEC10,0xED10,
	0xEE10,0xEF10,0xF030,0xF130,0xF230,0xF330,0xF430,0xF530,
	0xF610,0xF710,0xF810,0xF910,0xFA10,0xFB10,0xFC10,0xFD10,
	0xFE10,0xFF10,0x00B0,0x0130,0x0230,0x0330,0x0430,0x0530,
	0x0610,0x0710,0x0810,0x0910,0x0A10,0x0B10,0x0C10,0x0D10,
	0x0E10,0x0F10,0x1030,0x1130,0x1230,0x1330,0x1430,0x1530,
	0x1610,0x1710,0x1810,0x1910,0x1A10,0x1B10,0x1C10,0x1D10,
	0x1E10,0x1F10,0x2030,0x2130,0x2230,0x2330,0x2430,0x2530,
	0x2610,0x2710,0x2810,0x2910,0x2A10,0x2B10,0x2C10,0x2D10,
	0x2E10,0x2F10,0x3030,0x3130,0x3230,0x3330,0x3430,0x3530,
	0x3610,0x3710,0x3810,0x3910,0x3A10,0x3B10,0x3C10,0x3D10,
	0x3E10,0x3F10,0x4030,0x4130,0x4230,0x4330,0x4430,0x4530,
	0x4610,0x4710,0x4810,0x4910,0x4A10,0x4B10,0x4C10,0x4D10,
	0x4E10,0x4F10,0x5030,0x5130,0x5230,0x5330,0x5430,0x5530,
	0x5610,0x5710,0x5810,0x5910,0x5A10,0x5B10,0x5C10,0x5D10,
	0x5E10,0x5F10,0x6030,0x6130,0x6230,0x6330,0x6430,0x6530,
	0x00C0,0x0140,0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,
	0x0840,0x0940,0x0440,0x0540,0x0640,0x0740,0x0840,0x0940,
	0x1040,0x1140,0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,
	0x1840,0x1940,0x1440,0x1540,0x1640,0x1740,0x1840,0x1940,
	0x2040,0x2140,0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,
	0x2840,0x2940,0x2440,0x2540,0x2640,0x2740,0x2840,0x2940,
	0x3040,0x3140,0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,
	0x3840,0x3940,0x3440,0x3540,0x3640,0x3740,0x3840,0x3940,
	0x4040,0x4140,0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,
	0x4840,0x4940,0x4440,0x4540,0x4640,0x4740,0x4840,0x4940,
	0x5040,0x5140,0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,
	0x5840,0x5940,0x5440,0x5540,0x5640,0x5740,0x5840,0x5940,
	0x6040,0x6140,0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,
	0x6840,0x6940,0x6440,0x6540,0x6640,0x6740,0x6840,0x6940,
	0x7040,0x7140,0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,
	0x7840,0x7940,0x7440,0x7540,0x7640,0x7740,0x7840,0x7940,
	0x8040,0x8140,0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,
	0x8840,0x8940,0x8440,0x8540,0x8640,0x8740,0x8840,0x8940,
	0x9040,0x9140,0x9240,0x9340,0x9440,0x9540,0x9640,0x9740,
	0x9840,0x9940,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x4050,0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,
	0x4850,0x4950,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x5050,0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,
	0x5850,0x5950,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x6050,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,
	0x6850,0x6950,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,
	0x7850,0x7950,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x8050,0x8150,0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,
	0x8850,0x8950,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x9050,0x9150,0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,
	0x9850,0x9950,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
	0xA050,0xA150,0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,
	0xA850,0xA950,0xA450,0xA550,0xA650,0xA750,0xA850,0xA950,
	0xB050,0xB150,0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,
	0xB850,0xB950,0xB450,0xB550,0xB650,0xB750,0xB850,0xB950,
	0xC050,0xC150,0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,
	0xC850,0xC950,0xC450,0xC550,0xC650,0xC750,0xC850,0xC950,
	0xD050,0xD150,0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,
	0xD850,0xD950,0xD450,0xD550,0xD650,0xD750,0xD850,0xD950,
	0xE050,0xE150,0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,
	0xE850,0xE950,0xE450,0xE550,0xE650,0xE750,0xE850,0xE950,
	0xF050,0xF150,0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,
	0xF850,0xF950,0xF450,0xF550,0xF650,0xF750,0xF850,0xF950,
	0x00D0,0x0150,0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,
	0x0850,0x0950,0x0450,0x0550,0x0650,0x0750,0x0850,0x0950,
	0x1050,0x1150,0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,
	0x1850,0x1950,0x1450,0x1550,0x1650,0x1750,0x1850,0x1950,
	0x2050,0x2150,0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,
	0x2850,0x2950,0x2450,0x2550,0x2650,0x2750,0x2850,0x2950,
	0x3050,0x3150,0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,
	0x3850,0x3950,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x4050,0x4150,0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,
	0x4850,0x4950,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x5050,0x5150,0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,
	0x5850,0x5950,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x6050,0x6150,0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,
	0x6850,0x6950,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x7050,0x7150,0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,
	0x7850,0x7950,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x8050,0x8150,0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,
	0x8850,0x8950,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x9050,0x9150,0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,
	0x9850,0x9950,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
	0xFA60,0xFB60,0xFC60,0xFD60,0xFE60,0xFF60,0x00C0,0x0140,
	0x0240,0x0340,0x0440,0x0540,0x0640,0x0740,0x0840,0x0940,
	0x0A60,0x0B60,0x0C60,0x0D60,0x0E60,0x0F60,0x1040,0x1140,
	0x1240,0x1340,0x1440,0x1540,0x1640,0x1740,0x1840,0x1940,
	0x1A60,0x1B60,0x1C60,0x1D60,0x1E60,0x1F60,0x2040,0x2140,
	0x2240,0x2340,0x2440,0x2540,0x2640,0x2740,0x2840,0x2940,
	0x2A60,0x2B60,0x2C60,0x2D60,0x2E60,0x2F60,0x3040,0x3140,
	0x3240,0x3340,0x3440,0x3540,0x3640,0x3740,0x3840,0x3940,
	0x3A60,0x3B60,0x3C60,0x3D60,0x3E60,0x3F60,0x4040,0x4140,
	0x4240,0x4340,0x4440,0x4540,0x4640,0x4740,0x4840,0x4940,
	0x4A60,0x4B60,0x4C60,0x4D60,0x4E60,0x4F60,0x5040,0x5140,
	0x5240,0x5340,0x5440,0x5540,0x5640,0x5740,0x5840,0x5940,
	0x5A60,0x5B60,0x5C60,0x5D60,0x5E60,0x5F60,0x6040,0x6140,
	0x6240,0x6340,0x6440,0x6540,0x6640,0x6740,0x6840,0x6940,
	0x6A60,0x6B60,0x6C60,0x6D60,0x6E60,0x6F60,0x7040,0x7140,
	0x7240,0x7340,0x7440,0x7540,0x7640,0x7740,0x7840,0x7940,
	0x7A60,0x7B60,0x7C60,0x7D60,0x7E60,0x7F60,0x8040,0x8140,
	0x8240,0x8340,0x8440,0x8540,0x8640,0x8740,0x8840,0x8940,
	0x8A60,0x8B60,0x8C60,0x8D60,0x8E60,0x8F60,0x9040,0x9140,
	0x9240,0x9340,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x3A70,0x3B70,0x3C70,0x3D70,0x3E70,0x3F70,0x4050,0x4150,
	0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x4A70,0x4B70,0x4C70,0x4D70,0x4E70,0x4F70,0x5050,0x5150,
	0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x5A70,0x5B70,0x5C70,0x5D70,0x5E70,0x5F70,0x6050,0x6150,
	0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x6A70,0x6B70,0x6C70,0x6D70,0x6E70,0x6F70,0x7050,0x7150,
	0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x7A70,0x7B70,0x7C70,0x7D70,0x7E70,0x7F70,0x8050,0x8150,
	0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x8A70,0x8B70,0x8C70,0x8D70,0x8E70,0x8F70,0x9050,0x9150,
	0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
	0x9A70,0x9B70,0x9C70,0x9D70,0x9E70,0x9F70,0xA050,0xA150,
	0xA250,0xA350,0xA450,0xA550,0xA650,0xA750,0xA850,0xA950,
	0xAA70,0xAB70,0xAC70,0xAD70,0xAE70,0xAF70,0xB050,0xB150,
	0xB250,0xB350,0xB450,0xB550,0xB650,0xB750,0xB850,0xB950,
	0xBA70,0xBB70,0xBC70,0xBD70,0xBE70,0xBF70,0xC050,0xC150,
	0xC250,0xC350,0xC450,0xC550,0xC650,0xC750,0xC850,0xC950,
	0xCA70,0xCB70,0xCC70,0xCD70,0xCE70,0xCF70,0xD050,0xD150,
	0xD250,0xD350,0xD450,0xD550,0xD650,0xD750,0xD850,0xD950,
	0xDA70,0xDB70,0xDC70,0xDD70,0xDE70,0xDF70,0xE050,0xE150,
	0xE250,0xE350,0xE450,0xE550,0xE650,0xE750,0xE850,0xE950,
	0xEA70,0xEB70,0xEC70,0xED70,0xEE70,0xEF70,0xF050,0xF150,
	0xF250,0xF350,0xF450,0xF550,0xF650,0xF750,0xF850,0xF950,
	0xFA70,0xFB70,0xFC70,0xFD70,0xFE70,0xFF70,0x00D0,0x0150,
	0x0250,0x0350,0x0450,0x0550,0x0650,0x0750,0x0850,0x0950,
	0x0A70,0x0B70,0x0C70,0x0D70,0x0E70,0x0F70,0x1050,0x1150,
	0x1250,0x1350,0x1450,0x1550,0x1650,0x1750,0x1850,0x1950,
	0x1A70,0x1B70,0x1C70,0x1D70,0x1E70,0x1F70,0x2050,0x2150,
	0x2250,0x2350,0x2450,0x2550,0x2650,0x2750,0x2850,0x2950,
	0x2A70,0x2B70,0x2C70,0x2D70,0x2E70,0x2F70,0x3050,0x3150,
	0x3250,0x3350,0x3450,0x3550,0x3650,0x3750,0x3850,0x3950,
	0x3A70,0x3B70,0x3C70,0x3D70,0x3E70,0x3F70,0x4050,0x4150,
	0x4250,0x4350,0x4450,0x4550,0x4650,0x4750,0x4850,0x4950,
	0x4A70,0x4B70,0x4C70,0x4D70,0x4E70,0x4F70,0x5050,0x5150,
	0x5250,0x5350,0x5450,0x5550,0x5650,0x5750,0x5850,0x5950,
	0x5A70,0x5B70,0x5C70,0x5D70,0x5E70,0x5F70,0x6050,0x6150,
	0x6250,0x6350,0x6450,0x6550,0x6650,0x6750,0x6850,0x6950,
	0x6A70,0x6B70,0x6C70,0x6D70,0x6E70,0x6F70,0x7050,0x7150,
	0x7250,0x7350,0x7450,0x7550,0x7650,0x7750,0x7850,0x7950,
	0x7A70,0x7B70,0x7C70,0x7D70,0x7E70,0x7F70,0x8050,0x8150,
	0x8250,0x8350,0x8450,0x8550,0x8650,0x8750,0x8850,0x8950,
	0x8A70,0x8B70,0x8C70,0x8D70,0x8E70,0x8F70,0x9050,0x9150,
	0x9250,0x9350,0x9450,0x9550,0x9650,0x9750,0x9850,0x9950,
};


static s32 timer_counter_table[] = { 1024, 16, 64, 256 };

//TEMPORARY
bool output_opcodes = false;
bool fullspeed = false;

/* internal functions */
static int execute();
static void interrupt( InterruptType type );
static void update_lcd_status();
static void update_lcd( u32 current_cycles );
static void update_timer( u32 current_cycles );
static void update_divider( u32 current_cycles );
static void service_interrupts();
static void sync_refresh_rate( u32 *total_cycles );

static void lock_cpu();
static void unlock_cpu();

static void lock_cpu()
{
/*
#ifdef THREADSAFE_CPU
	assert( cpu_mutex != NULL );
	if( SDL_mutexP( cpu_mutex ) < 0 ) {
		fprintf( stderr, "unable to lock mutex: %s\n", SDL_GetError() );
		return;
	}
#endif
*/
}

static void unlock_cpu()
{
/*
#ifdef THREADSAFE_CPU
	assert( cpu_mutex != NULL );
	if( SDL_mutexV( cpu_mutex ) < 0 ) {
		fprintf( stderr, "unable to unlock mutex:  %s\n", SDL_GetError() );
		return;
	}
#endif
*/
}

void change_emulator_state( EmulatorState new_state ) {
    emulator_state = new_state;
    
    char* emu_str;
    switch( emulator_state ) {
        case PAUSED: emu_str = "PAUSED"; break;
        case STARTED: emu_str = "STARTED"; break;
        case STOPPED: emu_str = "STOPPED"; break;
        default: emu_str = "INVALID"; break;
    }
    //printf( "Emulator state changed to %s\n", emu_str );
    
	/*
    assert( SetEvent( emulator_state_changed ) != 0 );
	*/
}

void print_bios_checksum()
{
	int i;
	int checksum = 0;
	for(i = 0; i < BIOS_SIZE; ++i) {
		checksum += bios[i];
	}
	printf("checksum = %X\n", checksum);
}

void initialize_cpu()
{
	assert( cpu_initialized == false );

	print_bios_checksum();
	
    /*
	if( QueryPerformanceFrequency( (LARGE_INTEGER*)&freq ) == FALSE ) {
		printf("WARNING!! system does not support high-res performance counter!\n"); 
	}
	QueryPerformanceCounter( (LARGE_INTEGER*)&last_refresh );
	*/
	
    
	/*
    emulator_state_changed = CreateEvent( NULL, FALSE, FALSE, NULL );
    assert( emulator_state_changed != NULL );
	*/
	
    change_emulator_state( PAUSED );
	
	/*
	cpu_mutex = SDL_CreateMutex();
	if( cpu_mutex == NULL ) {
		fprintf( stderr, "unable to create mutex: %s\n", SDL_GetError() );
		return;
	}
	*/
	
	lock_cpu();
	
	PC.W = 0;
    //PC.W = 0x100;
    //fprintf(stderr, "skipping bios\n");
	SP.W = 0;
	BC.W = 0;
	DE.W = 0;
	HL.W = 0;
	AF.W = 0;
	IR.W = 0;
	
	waiting_for_interrupt = false;
	
	timer_counter = INITIAL_TIMER_COUNTER;
	scanline_counter = INITIAL_SCANLINE_COUNTER;	

	cycles = 0;
	mode_cycles = 0;
	divider_counter = 0;
	
	cpu_initialized = true;
	
	unlock_cpu();
}

void reinitialize_cpu()
{
    printf("reinitialize_cpu()\n");
	assert( cpu_initialized == true );
	
	if( emulator_state == STARTED ) {
		stop_cpu();
	}
	
	/*
	SDL_DestroyMutex( cpu_mutex );
	*/
	
	cpu_initialized = false;
	initialize_cpu();
}

void start_cpu()
{
	lock_cpu();
	
	assert( cpu_initialized == true );
	assert( emulator_state == STOPPED || emulator_state == PAUSED );

    change_emulator_state( STARTED );
    
	unlock_cpu();
}

void pause_cpu()
{
	lock_cpu();
	
	assert( cpu_initialized == true );
	assert( emulator_state == STARTED );

    change_emulator_state( PAUSED );
    
	unlock_cpu();
}

void stop_cpu()
{
	lock_cpu();
	
	assert( cpu_initialized == true );
	assert( emulator_state == STARTED );

    change_emulator_state( STOPPED );
    
	unlock_cpu();
}

void emulate_cpu() {

	lock_cpu();

	//int z = 0;
	//const int limit = 1000;
	
	while( emulator_state != STOPPED ) {
	
		/* ANDROID TESTING */
		/*
		if(++z > limit) {
			stop_cpu();
			continue;
		}
		*/
	
		if( emulator_state == PAUSED ) {
			/*
            while( emulator_state == PAUSED ) {
                unlock_cpu();
                printf("Waiting for emulator to resume\n");
                DWORD result = WaitForSingleObject( emulator_state_changed, INFINITE );
                assert( result == WAIT_OBJECT_0 );
                lock_cpu();
            }
			continue;
			*/
		}

        /*
        if( cpu_debugger_enabled ) {
            debug_emulate_cpu( &cpu_state );
        }
        */
		
		/*
        if( debugger != NULL ) {
            debugger->debug_cpu( &cpu_state );
        }
		*/
        
		//printf("before execute()");
		u32 current_cycles = execute();
		//printf("after execute()");
		
		cycles += current_cycles;
		mode_cycles += current_cycles;

		update_divider( current_cycles );
		update_lcd_status();
		update_lcd( current_cycles );
		update_timer( current_cycles );

        service_interrupts();
        
        sync_refresh_rate( &cycles );
	}
	
	unlock_cpu();

}

/**
 *  
 */
static void sync_refresh_rate( u32 *total_cycles ) 
{
	if(*total_cycles < SCREEN_REFRESH_CYCLES) return;
	*total_cycles = 0;

    /* TODO this caused an assertion fail, not sure why... */
	//interrupt( VBLANK );
	
	/*
    if( *total_cycles < SCREEN_REFRESH_CYCLES || fullspeed ) {
        return;
    }
    
    unlock_cpu();
    
    u64 now;
    *total_cycles = 0;
    
    QueryPerformanceCounter((LARGE_INTEGER*)&now);
    
    u64 elapsed_usec = ((now - last_refresh) / (double)freq) * 1000000.0;
    usleep((int)((1000000 - elapsed_usec) / GB_V_SYNC_HZ));
    
    QueryPerformanceCounter((LARGE_INTEGER*)&last_refresh);
    
    lock_cpu();
	*/
}

static void interrupt( InterruptType type ) {
    waiting_for_interrupt = false;
    switch( type ) {
        case VBLANK:
            hardware_registers[IF] |= 0x01;
			stop_cpu();
            break;
        case LCD_STATUS:
            hardware_registers[IF] |= 0x02;
            break;
        case TIMER:
            hardware_registers[IF] |= 0x04;
            break;
        case SERIAL:
            hardware_registers[IF] |= 0x08;
            break;
        case JOYPAD:
			hardware_registers[IF] |= 0x10;
            break;
    }
}

static void update_lcd_status()
{
	
	u8 lcd_status = hardware_registers[STAT] & 0xFC; 	/* clear 2 LSB */
	if((hardware_registers[LCDC] & LCD_DISPLAY_ENABLE) == 0) {
		/* LCD is disabled */
		scanline_counter = 456;
		hardware_registers[LY] = 0;
		lcd_status |= 1;
	} else {
		/* LCD is enabled */
		u8 lcd_mode = hardware_registers[STAT] & 0x3;
		u8 new_lcd_mode = 0;
		bool lcd_interrupt = false;
		
		if(hardware_registers[LY] > 144) {
			/* LCD is in vertical blank, set mode to 1 */
			new_lcd_mode = 1;
			lcd_status |= 1;
		} else {
			if(scanline_counter >= (456 - 80)) {
				/* mode 2 */
				new_lcd_mode = 2;
				lcd_status |= 2;
				if(lcd_status & 0x20) {
					lcd_interrupt = true;
				}
			} else if(scanline_counter >= ((456 - 80) - 172)) {
				/* mode 3 */
				new_lcd_mode = 3;
				lcd_status |= 3;
			} else {
				/* mode 4 */
				new_lcd_mode = 0;
				if(lcd_status & 0x8) {
					lcd_interrupt = true;
				}
			}
		}
		
		if(lcd_interrupt && (lcd_mode != new_lcd_mode)) {
			interrupt( LCD_STATUS );
		}
	}
	
	hardware_registers[STAT] = lcd_status;
}



static void update_lcd( u32 current_cycles )
{
	if(hardware_registers[LCDC] & LCD_DISPLAY_ENABLE) {

		scanline_counter -= current_cycles;
		
		if(scanline_counter <= 0) {
			/* increment the current scanline */
			
			++hardware_registers[LY];
			
			/* reset the scanline counter */
			scanline_counter = 456;
			
			
			
			if(hardware_registers[LY] == 144) {
				/* enter vertical blank period */
				interrupt( VBLANK );
				/*
				update_screen();
				*/
				
	
			} else if(hardware_registers[LY] > 153) {
				/* scanline back to 0 (end of vertical blank) */
				
				/* TODO figure out a cleaner way to do this
				 * previously, hardware_registers[LY] was set to 0, but then the 
				 * first line wouldn't be drawn because the scanline register is incremented 
				 * before a scanline is rendered
				 */
				hardware_registers[LY] = -1;
				
			} else if(hardware_registers[LY] < 144) {
				render_scanline();
			}
		}
	}
}

static void update_timer( u32 current_cycles )
{
    if((hardware_registers[TAC] & 0x4) == 0) {
        /* timer is stopped */
        return;
    }
    
    timer_counter -= current_cycles;
    
    if(timer_counter <= 0) {
        /* reset the counter */
        int index = hardware_registers[TAC] & 0x3;
        timer_counter = timer_counter_table[index];
        
        if(hardware_registers[TIMA] == 0xFF) {
            hardware_registers[TIMA] = hardware_registers[TMA];
            interrupt( TIMER );
        } else {
            ++hardware_registers[TIMA];
        }
    }
}

static void update_divider( u32 current_cycles )
{
	divider_counter += current_cycles;
	
	if(divider_counter >= 255) {
		/* increment divider register */
		divider_counter = 0;
		++hardware_registers[DIV];
	}
}

static void service_interrupts()
{
	if( IME && interrupt_enable && hardware_registers[IF] ) {
	
		u8 interrupt = interrupt_enable & hardware_registers[IF];
		
		bool serviced_interrupt = false;
		u16 interrupt_address = 0;
		
        /* order of bits represents interrupt priority */
		if( interrupt & 0x1 ) { 
			/* vblank interrupt */
			serviced_interrupt = true;

			hardware_registers[IF] &= ~0x1;
			interrupt_address = 0x40;
			
			/* ANDROID TESTING */
			//stop_cpu();
		} 
		else if( interrupt & 0x2 ) {
			/* lcd interrupt */
			serviced_interrupt = true;
			
			hardware_registers[IF] &= ~0x2;
			interrupt_address = 0x48;

		} 
		else if( interrupt & 0x4 ) {
			/* timer interrupt */
			serviced_interrupt = true;
			
			hardware_registers[IF] &= ~0x4;
			interrupt_address = 0x50;
			
		} 
        else if( interrupt & 0x08 ) 
        {
            /* serial interrupt */
            serviced_interrupt = true;
            
            hardware_registers[IF] &= ~0x8;
			interrupt_address = 0x58;
        }
		else if( interrupt & 0x10 ) {
			/* joypad Interrupt */
			serviced_interrupt = true;
			
			hardware_registers[IF] &= ~0x10;
			interrupt_address = 0x60;
		}
		
		if( serviced_interrupt == true ) {
			assert( interrupt_address != 0 );
			
			MBC_write( --SP.W, PC.B.H );
			MBC_write( --SP.W, PC.B.L );
			
			IME = 0;
			PC.W = interrupt_address;
			cycles += 16;
		}
	}
}

void print_cpu_state()
{
	
	printf("AF=%04X\t\nBC=%04X\t\nDE=%04X\t\nHL=%04X\t\nSP=%04X\t\nPC=%04X\t(%s)\n",\
	AF.W, BC.W, DE.W, HL.W, SP.W, PC.W, opcode[MBC_read(PC.W)]);
\
	//TEMPORARY
	/*
	fprintf(opcodes_file, "AF=%04X\tBC=%04X\tDE=%04X\tHL=%04X\tSP=%04X\tPC=%04X\t\n",\
	AF.W, BC.W, DE.W, HL.W, SP.W, PC.W);
	*/
}

//temp
//static bool debug_daa = false;

static int execute() {

	//print_cpu_state();
	//getchar();

	/* variables that may be used by certain instructions */
	static Z80Register TR;
    static s8 offset;

	/* load the next intruction */
    if(waiting_for_interrupt == true) {
		/* execute NOP instructions while waiting for interrupt */
        IR.W = 0x00;
    } else {
        IR.W = MBC_read(PC.W++);	
    }

	/* execute the instruction and return cpu cycles */
	switch(IR.W) {
		#include "opcodes.h"
	}
    
	/* program will not reach this point */
    assert( false );
	return -1;
}

void cpu_set_timer_countr( int timer_countr )
{
	timer_counter = timer_countr;
}

CpuState get_cpu_state()
{
	lock_cpu();
	CpuState current_cpu_state = cpu_state;
	unlock_cpu();
	return current_cpu_state;
}

void set_cpu_state( CpuState new_cpu_state )
{
	lock_cpu();
	cpu_state = new_cpu_state;
	unlock_cpu();
}

/*
void set_cpu_debugger_enabled( bool value ) {
#ifdef DEBUG
    char *val_str = (value ? "TRUE" : "FALSE");
    printf("cpu_debugger_enabled set to %s\n", val_str);
#endif
    cpu_debugger_enabled = value;
}
*/

/*
void cpu_attach_debugger( Debugger *_debugger)
{
   assert( debugger == NULL ); 
   assert( _debugger != NULL );

#ifdef DEBUG
    printf("attaching CPU debugger\n");
#endif

   //TODO check if it is actually necessary to lock the cpu
    lock_cpu();
    debugger = _debugger;
    unlock_cpu();
}
*/

/*
Debugger *cpu_detach_debugger( void )
{
    assert( debugger != NULL );
    Debugger *_debugger = debugger;

#ifdef DEBUG
    printf("detaching CPU debugger\n");    
#endif

    lock_cpu(); //refer to attach_debuggerj
    debugger = NULL;
    unlock_cpu();
    return _debugger;
}
*/


