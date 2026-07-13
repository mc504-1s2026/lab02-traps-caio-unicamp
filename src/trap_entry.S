# this is what will be loaded into the stvec CSR; the
# trap handler address must be 4-byte aligned:
# https://docs.riscv.org/reference/isa/priv/supervisor.html#12-1-1-2-supervisor-trap-vector-base-address-stvec-register
.globl trap_entry
.align 4
trap_entry:

    # your code here; save registers to the stack
    addi sp, sp, -80
    
    sw ra, 0(sp)    # Salvando endereço de retorno
    # Registradores temporários
    sw t0, 4(sp)
    sw t1, 8(sp)
    sw t2, 16(sp)
    sw t3, 20(sp)
    sw t4, 24(sp)
    sw t5, 28(sp)
    sw t6, 32(sp)
    # Argumentos e retornos
    sw a0, 36(sp)
    sw a1, 40(sp)
    sw a2, 44(sp)
    sw a3, 48(sp)
    sw a4, 52(sp)
    sw a5, 56(sp)
    sw a6, 60(sp)
    sw a7, 64(sp)
    
    
    call handle_trap

    # your code here; restore registers
    
    lw ra, 0(sp)    # Recupera endereço de retorno
    # Registradores temporários
    lw t0, 4(sp)
    lw t1, 8(sp)
    lw t2, 16(sp)
    lw t3, 20(sp)
    lw t4, 24(sp)
    lw t5, 28(sp)
    lw t6, 32(sp)
    # Argumentos e retornos
    lw a0, 36(sp)
    lw a1, 40(sp)
    lw a2, 44(sp)
    lw a3, 48(sp)
    lw a4, 52(sp)
    lw a5, 56(sp)
    lw a6, 60(sp)
    lw a7, 64(sp)

    addi sp, sp, 80

    sret
