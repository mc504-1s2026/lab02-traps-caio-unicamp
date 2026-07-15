#include <arch/timer.h>
#include <kernel/panic.h>
#include <kernel/printf.h>
#include <arch/csr.h>

u64 timer_read()
{
    // Lê o registrador de tempo absoluto
    return csr_read(CSR_TIME);
}

void timer_irq_enable()
{
    // Seta o bit STIE no registrador sie
    csr_set(CSR_SIE, CSR_SIE_STIE);

	csr_write(CSR_STIMECMP, csr_read(CSR_TIME) + TIMER_FREQ);
}

void timer_irq_disable()
{
    // Limpa o bit STIE no registrador sie
    csr_clear(CSR_SIE, CSR_SIE_STIE);
}

void timer_set_alarm(u64 secs)
{
    u64 now = timer_read();
    u64 tick_in_future = now + (secs * TIMER_FREQ);
    
    // Escreve no comparador. Quando CSR_TIME atingir esse valor, a IRQ dispara.
    csr_write(CSR_STIMECMP, tick_in_future);
    
    // Garante que a interrupção do timer está ativada no hart
    timer_irq_enable();
}

void timer_irq()
{    
    // imprime "alarm" quando o timer estourar
    print("alarm\n");
    csr_write(CSR_STIMECMP, -1ULL); // Taca o tempo pra infinito
}


