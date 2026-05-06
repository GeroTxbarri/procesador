#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// 1. MEMORIAS SEPARADAS (Arquitectura Harvard)
#define MEM_SIZE 256
uint32_t program_memory[MEM_SIZE];
uint32_t data_memory[MEM_SIZE];

// 2. ESTADO DEL PROCESADOR (Según pizarrón)
uint32_t pc = 0;        // Program Counter
uint32_t psw = 0;       // Program Status Word (Banderas, no usado en este subset pero declarado)
uint32_t regs[16] = {0};// Banco de 16 registros

// Nombres de los registros para debugear fácil
const char* reg_names[16] = {
    "t0", "t1", "t2", "t3", 
    "s0", "s1", "s2", "s3", "s4", "s5", 
    "ra", "gp", "sp", "fp", "a0", "a1"
};

// 3. OPCODES (Deducidos del pizarrón)
#define OP_ADD 0x0   // R-Type
#define OP_LW  0x2   // I-Type
#define OP_SW  0x3   // I-Type
#define OP_BEQ 0x4   // I-Type
#define OP_J   0xE   // J-Type (1110 en binario)

// --- MACROS PARA ENSAMBLAR INSTRUCCIONES ---
// (Facilita cargar el programa en memoria)
#define MAKE_R(op, rd, rs, rt)   (((op) << 28) | ((rd) << 24) | ((rs) << 20) | ((rt) << 16))
#define MAKE_I(op, rd, rs, imm)  (((op) << 28) | ((rd) << 24) | ((rs) << 20) | ((imm) & 0xFFFFF))
#define MAKE_J(op, addr)         (((op) << 28) | ((addr) & 0xFFFFFFF))

int main() {
    // ---------------------------------------------------------
    // PROGRAMA DE PRUEBA (Cargado en program_memory)
    // ---------------------------------------------------------
    // Vamos a simular:
    // 1. Cargar datos de memoria.
    // 2. Sumarlos.
    // 3. Guardar el resultado.
    // 4. Saltar y terminar.
    
    data_memory[10] = 15; // Valor en dirección 10
    data_memory[11] = 25; // Valor en dirección 11

    // Inst 0: lw t0, s0(offset 10) -> Asumimos s0=0 por ahora. t0 = mem[0 + 10] (Carga 15)
    program_memory[0] = MAKE_I(OP_LW, 0, 4, 10); 
    
    // Inst 1: lw t1, s0(offset 11) -> t1 = mem[0 + 11] (Carga 25)
    program_memory[1] = MAKE_I(OP_LW, 1, 4, 11);
    
    // Inst 2: add t2, t0, t1 -> t2 = t0 + t1 (15 + 25 = 40)
    program_memory[2] = MAKE_R(OP_ADD, 2, 0, 1);
    
    // Inst 3: sw t2, s0(offset 12) -> mem[0 + 12] = t2 (Guarda 40 en mem[12])
    program_memory[3] = MAKE_I(OP_SW, 2, 4, 12);

    // Inst 4: beq t0, t1, offset -> Si t0 == t1 salta. Como 15 != 25, no salta.
    program_memory[4] = MAKE_I(OP_BEQ, 0, 1, 100); 

    // Inst 5: j 100 -> Salto incondicional al final del programa para evitar ejecutar basura.
    program_memory[5] = MAKE_J(OP_J, 100);

    // ---------------------------------------------------------
    // CICLO DE INSTRUCCIÓN (FETCH, DECODE, EXECUTE)
    // ---------------------------------------------------------
    bool running = true;
    while (running && pc < MEM_SIZE) {
        
        // FETCH
        uint32_t instr = program_memory[pc];
        
        // Si encontramos una instrucción vacía (0) que no sea ADD, cortamos por seguridad en la simulación.
        if (instr == 0 && pc >= 6) break; 
        
        uint32_t current_pc = pc; // Guardar PC actual para debug
        pc++; // Avanzar PC por defecto

        // DECODE
        // Extraemos los campos según el ancho de bits que inferimos de las cajas
        uint32_t opcode = (instr >> 28) & 0xF;    // 4 bits
        uint32_t rd     = (instr >> 24) & 0xF;    // 4 bits
        uint32_t rs     = (instr >> 20) & 0xF;    // 4 bits
        uint32_t rt     = (instr >> 16) & 0xF;    // 4 bits
        
        // Extraemos el inmediato de 20 bits y extendemos el signo por si es un offset negativo
        int32_t imm_I   = instr & 0xFFFFF;
        if (imm_I & 0x80000) imm_I |= 0xFFF00000; 
        
        uint32_t addr_J = instr & 0xFFFFFFF;      // 28 bits

        // EXECUTE
        switch (opcode) {
            case OP_ADD:
                regs[rd] = regs[rs] + regs[rt];
                printf("PC:%02d | ADD  %s, %s, %s \t-> %s = %d\n", current_pc, reg_names[rd], reg_names[rs], reg_names[rt], reg_names[rd], regs[rd]);
                break;
                
            case OP_LW:
                regs[rd] = data_memory[regs[rs] + imm_I];
                printf("PC:%02d | LW   %s, %d(%s) \t-> %s = %d\n", current_pc, reg_names[rd], imm_I, reg_names[rs], reg_names[rd], regs[rd]);
                break;
                
            case OP_SW:
                data_memory[regs[rs] + imm_I] = regs[rd];
                printf("PC:%02d | SW   %s, %d(%s) \t-> M[%d] = %d\n", current_pc, reg_names[rd], imm_I, reg_names[rs], regs[rs] + imm_I, regs[rd]);
                break;
                
            case OP_BEQ:
                printf("PC:%02d | BEQ  %s, %s, %d\n", current_pc, reg_names[rd], reg_names[rs], imm_I);
                if (regs[rd] == regs[rs]) {
                    pc = pc + imm_I - 1; // Salto relativo al PC actual
                }
                break;
                
            case OP_J:
                printf("PC:%02d | J    %d\n", current_pc, addr_J);
                pc = addr_J; 
                if (pc == 100) running = false; // Halt casero para nuestro script
                break;
                
            default:
                printf("Error: Opcode desconocido 0x%X en PC %d\n", opcode, current_pc);
                running = false;
                break;
        }
    }

    // VERIFICACIÓN
    printf("\n--- RESULTADO FINAL ---\n");
    printf("Registro t2 (Suma total): %d\n", regs[2]);
    printf("Memoria de datos [12]: %d\n", data_memory[12]);
    
    return 0;
}