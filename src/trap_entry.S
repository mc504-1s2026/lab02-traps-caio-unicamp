# this is what will be loaded into the stvec CSR; the
# trap handler address must be 4-byte aligned:
# https://docs.riscv.org/reference/isa/priv/supervisor.html#12-1-1-2-supervisor-trap-vector-base-address-stvec-register
.text
.globl trap_entry
.align 4
trap_entry:
    # Aloca 160 bytes na pilha (múltiplo de 16)
    addi sp, sp, -160
    # your code here; save registers to the stack

    # Salva os registradores com offsets estritamente múltiplos de 8
    sd ra, 0(sp)
    sd sp, 8(sp)
    sd gp, 16(sp)
    sd tp, 24(sp)
    sd t0, 32(sp)
    sd t1, 40(sp)
    sd t2, 48(sp)
    sd a0, 56(sp)
    sd a1, 64(sp)
    sd a2, 72(sp)
    sd a3, 80(sp)
    sd a4, 88(sp)
    sd a5, 96(sp)
    sd a6, 104(sp)
    sd a7, 112(sp)
    sd t3, 120(sp)
    sd t4, 128(sp)
    sd t5, 136(sp)
    sd t6, 144(sp)
    
    call handle_trap
    # your code here; restore registers

    # Restaura os registradores na mesma ordem
    ld ra, 0(sp)
    ld sp, 8(sp)
    ld gp, 16(sp)
    ld tp, 24(sp)
    ld t0, 32(sp)
    ld t1, 40(sp)
    ld t2, 48(sp)
    ld a0, 56(sp)
    ld a1, 64(sp)
    ld a2, 72(sp)
    ld a3, 80(sp)
    ld a4, 88(sp)
    ld a5, 96(sp)
    ld a6, 104(sp)
    ld a7, 112(sp)
    ld t3, 120(sp)
    ld t4, 128(sp)
    ld t5, 136(sp)
    ld t6, 144(sp)

    # Desaloca a pilha
    addi sp, sp, 160
    sret