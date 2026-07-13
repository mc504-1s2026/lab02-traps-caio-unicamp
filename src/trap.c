#include <kernel/trap.h>
#include <kernel/panic.h>
#include <include/arch/csr.h>


#define TRAP_IRQ_BIT			(1ULL << 63)
#define TRAP_CODE_MASK			(TRAP_IRQ_BIT - 1)
#define TRAP_INST_ACCESS_FAULT		1
#define TRAP_LOAD_ACCESS_FAULT		5
#define TRAP_STORE_ACCESS_FAULT		7
#define TRAP_INST_PAGE_FAULT		12
#define TRAP_LOAD_PAGE_FAULT		13
#define TRAP_STORE_PAGE_FAULT		15

/* defined in src/trap_entry.S */
extern void trap_entry();
static bool ktest_fault_occurred = false;
static bool ktest_fault_expected = false;
static u64 ktest_fault_return_addr = 0;

void handle_irq()
{
	/* not implemented */
	BUG();
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
