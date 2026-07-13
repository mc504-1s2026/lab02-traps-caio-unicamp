#include <kernel/serial.h>
#include <kernel/panic.h>
#include <include/arch/spinlock.h>
#include <include/arch/plic.h>
#include <kernel/types.h>

// Mapeamento Direto para acessar a memória física na metade superior
#define KERNEL_DIRECT_MAP_START 0xFFFFFFC000000000ULL

// Macro para acesso MMIO: soma a base virtual com o offset do registrador
#define SERIAL_REG(offset) (*(volatile u8 *)(KERNEL_DIRECT_MAP_START + (u64)SERIAL_BASE + (offset)))

#define SERIAL_BUF_SIZE 256

// Estrutura de dados para o buffer assíncrono 
struct serialdev {
    char buf[SERIAL_BUF_SIZE];
    size_t len;
    struct spinlock lock;
} dev;

void serial_init()
{
    // Inicializa o spinlock e zera o buffer
    spin_init(&dev.lock);
    dev.len = 0;

    // Desabilita todas as interrupções seriais temporariamente
    SERIAL_REG(SERIAL_IER) = 0x00;

    // Habilita o DLAB no LCR para configurar o Baud Rate
	SERIAL_REG(SERIAL_LCR) = 0x80;

    // Configura o Baud Rate para 38.4K (Divisor = 3).
    // O LSB vai no offset 0 e o MSB no offset 1 (enquanto DLAB = 1)
	SERIAL_REG(SERIAL_RBR) = 0x03; // DLL
    SERIAL_REG(SERIAL_IER) = 0x00; // DLM

    // Desabilita o DLAB e configura para 8 bits de dados, 1 stop bit, sem paridade (8N1)
	SERIAL_REG(SERIAL_LCR) = 0x03;

    // Habilita e limpa os FIFOs de transmissão e recepção
	SERIAL_REG(SERIAL_FCR) = SERIAL_FCR_FIFO_ENABLE | SERIAL_FCR_RX_FIFO_CLEAR | SERIAL_FCR_TX_FIFO_CLEAR;
}

void serial_irq_enable()
{
    // Habilita a interrupção de "Received Data Available" na UART
	SERIAL_REG(SERIAL_IER) = SERIAL_IER_ERBFI;

    // Configura o PLIC para aceitar a interrupção da UART no hart 0
    plic_irq_set_priority(IRQ_SERIAL, 1);       // Prioridade 1 (qualquer coisa > 0 é válida)
    plic_hart_set_threshold(0, 0);            // Aceita prioridades maiores que 0
    plic_hart_enable_irq(0, IRQ_SERIAL);        // Habilita o IRQ 10
}

void serial_irq_disable()
{
    // Desabilita a interrupção de recepção na própria UART
	SERIAL_REG(SERIAL_IER) = 0x00;
}

void serial_irq()
{
    // Podem haver múltiplos bytes no FIFO, todos são lidos enquanto LSR apontar dado pronto
    while (SERIAL_REG(SERIAL_LSR) & SERIAL_LSR_DTR) {
        char c = SERIAL_REG(SERIAL_RBR);

        // Ao tratar a interrupção, as interrupções gerais do processador já estão desativadas 
        spin_lock(&dev.lock);
        
        if (dev.len < SERIAL_BUF_SIZE) {
            dev.buf[dev.len++] = c;
        }
        
        spin_unlock(&dev.lock);
    }
}

size_t serial_read(char *buf)
{
    u64 flags = spin_lock_irqsave(&dev.lock);

    size_t size = dev.len;
    for (size_t i = 0; i < size; i++) {
        buf[i] = dev.buf[i];
    }
    
    // Reseta o contador do buffer após a leitura
    dev.len = 0;

    spin_unlock_irqrestore(&dev.lock, flags);

    return size;
}

void serial_puts(char *str)
{
    // Itera e imprime caractere por caractere até achar caractere '\0'
    while (*str) {
        serial_putc(*str++);
    }
}

void serial_putc(char c)
{
    // Loop de espera ativa até que o Transmissor esteja livre (Bit 5 do LSR)
    while ((SERIAL_REG(SERIAL_LSR) & SERIAL_LSR_THRE) == 0) { }
    
    // Escreve o caractere no registrador de transmissão
    SERIAL_REG(SERIAL_THR) = c;
}
