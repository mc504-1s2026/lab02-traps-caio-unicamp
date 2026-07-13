#include <kernel/trap.h>
#include <kernel/panic.h>
#include <include/arch/csr.h>
#include <include/arch/plic.h>

#define TRAP_IRQ_BIT			(1ULL << 63)
#define TRAP_CODE_MASK			(TRAP_IRQ_BIT - 1)
#define TRAP_INST_ACCESS_FAULT		1
#define TRAP_LOAD_ACCESS_FAULT		5
#define TRAP_STORE_ACCESS_FAULT		7
#define TRAP_INST_PAGE_FAULT		12
#define TRAP_LOAD_PAGE_FAULT		13
#define TRAP_STORE_PAGE_FAULT		15

// Códigos de interrupção (IRQ) do RISC-V em Modo Supervisor
#define IRQ_S_TIMER             5
#define IRQ_S_EXT               9

// O IRQ da UART no QEMU
#define UART_IRQ                10

// Declarações das funções que iremos implementar nos outros arquivos
extern void timer_irq();
extern void serial_irq();

/* defined in src/trap_entry.S */
extern void trap_entry();

static bool ktest_fault_occurred = false;
static bool ktest_fault_expected = false;
static u64 ktest_fault_return_addr = 0;

void handle_irq()
{
    u64 scause = csr_read(CSR_SCAUSE);
    u64 irq_code = scause & TRAP_CODE_MASK; // Remove o bit 63 para pegar o código da irq

    switch (irq_code) {
        case IRQ_S_TIMER:
            // Interrupção de timer
            timer_irq();
            break;

        case IRQ_S_EXT: {
            // Interrupção externa
            u32 irq = plic_hart_claim_irq(0);	// Está sempre no hart 0
            
            if (irq != 0) {
                if (irq == UART_IRQ) {
                    serial_irq(); // Processa os dados da porta serial
                } else {
                    error("unexpected external irq: %d\n", irq);
                }
                // Avisa ao PLIC que terminou de processar essa IRQ
                plic_hart_complete_irq(0, irq);
            }
            break;
        }

        default:
            error("uncaught interrupt! cause: %llu\n", irq_code);
            panic("unexpected interrupt!\n");
    }
}

void handle_exception()
{
	static u64 scause, stval, sepc;
	
	scause = csr_read(CSR_SCAUSE);
	stval = csr_read(CSR_STVAL);
	sepc = csr_read(CSR_SEPC);

	u64 exception_code = scause & TRAP_CODE_MASK;	// Pegando apenas o código da exceção
	
	switch (exception_code) {
		case TRAP_INST_ACCESS_FAULT:
			error("instruction access fault at address 0x%x, sepc = 0x%x\n", stval, sepc);
			break;
		case TRAP_LOAD_ACCESS_FAULT:
			error("load access fault at address 0x%x, sepc = 0x%x\n", stval, sepc);
			break;
		case TRAP_STORE_ACCESS_FAULT:
			error("store access fault at address 0x%x, sepc = 0x%x\n", stval, sepc);
			break;
		case TRAP_INST_PAGE_FAULT:
			error("instruction page fault at address 0x%x, sepc = 0x%x\n", stval, sepc);
			break;
		case TRAP_LOAD_PAGE_FAULT:
			error("load page fault at address 0x%x, sepc = 0x%x\n", stval, sepc);
			break;
		case TRAP_STORE_PAGE_FAULT:
			error("store page fault at address 0x%x, sepc = 0x%x\n", stval, sepc);
			break;
		default:
			error("uncaught exception! cause: 0x%x, sepc = \n", scause, sepc);
	}	

	panic("unexpected fault!\n");
}

void trap_setup()
{
    // Grava o endereço do handler de assembly no CSR stvec
    csr_write(CSR_STVEC, trap_entry);
}

void handle_trap()
{
	u64 scause = csr_read(CSR_SCAUSE);

	if (scause & TRAP_IRQ_BIT){	// Analisa o bit 63 pra decidir ver se é uma irq ou uma exceção
		handle_irq();
	}else{
		handle_exception();
	}
}

void hart_irq_enable()
{
    // Seta o bit SIE em sstatus para habilitar interrupções
    csr_set(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

u64 hart_irq_save()
{
    // Lê o valor atual de sstatus, limpa o bit SIE atomicamente, e retorna o valor antigo
    return csr_read_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
}

void hart_irq_restore(u64 flags)
{
    // Restaura o estado das interrupções baseado no valor salvo de sstatus
    if (flags & CSR_SSTATUS_SIE) {
        hart_irq_enable();
    } else {
        hart_irq_disable();
    }
}

void hart_irq_disable()
{
    // Limpa o bit SIE em sstatus para desabilitar interrupções
    csr_clear(CSR_SSTATUS, CSR_SSTATUS_SIE);
}
