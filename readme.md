# Documentación Técnica: Emulador de Procesador RISC de 16 bits

## 1. Visión General
Este código implementa un simulador a nivel de conjunto de instrucciones (ISA) de un procesador RISC de 16 bits escrito en C. Modela el comportamiento fundamental del hardware digital mediante la ejecución del ciclo clásico de Fetch-Decode-Execute. Está diseñado con fines educativos e ilustra conceptos clave como la manipulación de bits, la separación de memoria y el direccionamiento.

## 2. Arquitectura del Sistema
El procesador virtual está modelado a través de la estructura CPU y presenta las siguientes características arquitectónicas:

* **Modelo de Memoria (Arquitectura Harvard):** Posee memorias separadas para datos (`data_memory`) y programas (`program_memory`), ambas con un tamaño definido por `MEM_SIZE` (1024 palabras de 16 bits por defecto). Esto permite leer instrucciones y acceder a datos en el mismo ciclo simulado sin colisiones.
* **Registros de Propósito General:** Un banco de 16 registros de 16 bits (`regs[0]` a `regs[15]`).
* **Registros de Estado y Control:**
    * `pc` (Program Counter): Apunta a la dirección de la próxima instrucción a ejecutar en la memoria de programa.
    * `psw` (Program Status Word): Reservado para banderas de estado (Zero, Carry, Overflow), aunque no se explota activamente en el conjunto de instrucciones básico actual.

## 3. Formato y Conjunto de Instrucciones (ISA)
Las instrucciones tienen una longitud fija de 16 bits. La arquitectura decodifica los primeros 4 bits (MSB) como el Opcode y divide el resto dependiendo del tipo de instrucción.

### Formatos de Instrucción
* **Tipo R (Registro):** `[ Opcode: 4b ] [ rd: 4b ] [ rs: 4b ] [ rt: 4b ]`
* **Tipo I (Inmediato):** `[ Opcode: 4b ] [ rd: 4b ] [ rs: 4b ] [ imm: 4b ]`
* **Tipo J (Salto):** `[ Opcode: 4b ] [ addr: 12b ]`

### Tabla de Opcodes Soportados

| Instrucción | Nemónico | Opcode (Hex) | Tipo | Operación |
| :--- | :--- | :---: | :---: | :--- |
| **Suma** | `ADD` | `0x0` | R | `rd = rs + rt` |
| **Cargar Palabra** | `LW` | `0x2` | I | `rd = MemData[rs + imm]` |
| **Guardar Palabra**| `SW` | `0x3` | I | `MemData[rs + imm] = rd` |
| **Salto Condicional**| `BEQ` | `0x4` | I | Si (`rd == rs`) -> `pc = pc + imm` |
| **Salto Incondicional**| `J` | `0xE` | J | `pc = addr` |

> **Nota sobre los inmediatos:** El campo `imm` de 4 bits en las instrucciones Tipo I incluye una extensión de signo (`signed_imm`), permitiendo valores negativos para realizar saltos hacia atrás con `BEQ` o aplicar offsets negativos en memoria.

## 4. Referencia de Funciones

### `void init_cpu(CPU *cpu)`
Inicializa el estado del hardware. Recibe un puntero a la estructura CPU y establece a 0 el Program Counter (`pc`), el Program Status Word (`psw`), todos los registros y limpia tanto la memoria de datos como la de programa.

### `void clock_cycle(CPU *cpu)`
Es el núcleo del emulador. Simula un ciclo de reloj completo ejecutando tres fases secuenciales:
1. **Fetch (Búsqueda):** Lee la instrucción de `program_memory` apuntada por el `pc` y luego incrementa el `pc`.
2. **Decode (Decodificación):** Utiliza desplazamientos de bits (`>>`) y máscaras lógicas (`& 0x000F`, etc.) para aislar el opcode, los registros origen/destino y los valores inmediatos o direcciones.
3. **Execute (Ejecución):** A través de un bloque `switch(opcode)`, ejecuta la lógica matemática, de memoria o de control de flujo correspondiente a la instrucción, modificando el estado del CPU e imprimiendo el resultado en la consola.

## 5. Escenario de Prueba (Testbench)
La función `main` actúa como el banco de pruebas inicial (Testbench):
1. **Configuración del Entorno:** Se inyectan valores arbitrarios en los registros `r1` y `r2`, y en la posición 15 de la memoria de datos.
2. **Ensamblado Manual:** Se construyen tres instrucciones binarias utilizando operadores bit a bit (`<<`, `|`) y se cargan directamente en `program_memory`:
    * `ADD r3, r1, r2`
    * `SW r3, r0, 5`
    * `J 10`
3. **Simulación:** Se ejecuta un bucle de 3 iteraciones llamando a `clock_cycle()`, lo que procesa secuencialmente el programa cargado.
4. **Volcado de Estado:** Imprime los valores finales de registros clave, ubicaciones de memoria modificadas y el `pc` final para verificar el correcto funcionamiento de la ALU y la memoria.