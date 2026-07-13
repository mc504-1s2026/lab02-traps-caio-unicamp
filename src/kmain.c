#include <kernel/printf.h>
#include <kernel/mm.h>
#include <arch/timer.h>
#include <kernel/trap.h>
#include <kernel/serial.h>
#include <kernel/string.h> 

extern int _hartid[];
void kmain()
{
    printk_set_level(LOG_DEBUG);
    info("entered S-mode\n");
    info("booting on hart %d\n", _hartid[0]);
    info("setting up virtual memory...\n");
    vm_init();

    info("enabling traps...\n");
    trap_setup();
    info("enabling timer...\n");
    timer_irq_enable();
    info("enabling serial...\n");
    serial_init();
    serial_irq_enable();
	
    // Habilita as interrupções globalmente no processador (sstatus.SIE = 1)
    hart_irq_enable();
	
	/* implement your shell here */
    
    char cmd_buf[256];
    int cmd_idx = 0;
    
    // Imprime ">" no começo
    serial_puts("> ");

    while (1) {
        char rx_buf[32];
        
        // Puxa o que tiver no buffer assíncrono do driver serial
        size_t n = serial_read(rx_buf);

        // Processa cada caractere recebido
        for (size_t i = 0; i < n; i++) {
            char c = rx_buf[i];

            // Faz o "echo" para o usuário ver o que está digitando
            serial_putc(c);

            // Verifica se é o Carriage Return (Enter = '\r')
            if (c == '\r') {
                // Coloca '\n' logo após o '\r' para quebrar a linha visualmente
                serial_putc('\n');

                // Adiciona o terminador nulo para transformar o array de chars em uma String de C
                cmd_buf[cmd_idx] = '\0';

                // Se o usuário digitou alguma coisa, ignorando Enters vazios
                if (cmd_idx > 0) {
                    
                    // Comando: uptime
                    if (strcmp(cmd_buf, "uptime") == 0) {
                        u64 ticks = timer_read();
                        u64 seconds = ticks / 10000000ULL; // Divide pela frequência (10MHz)
                        printk(LOG_INFO, "%llus\n", seconds);
                    } 
                    // Comando: echo [str]
                    else if (strncmp(cmd_buf, "echo ", 5) == 0) {
                        // Imprime a string ignorando os 5 primeiros caracteres ("echo ")
                        printk(LOG_INFO, "%s\n", cmd_buf + 5);
                    }
                    // Comando: alarm [time]
                    else if (strncmp(cmd_buf, "alarm ", 6) == 0) {
                        // Converte o tempo passado em string para inteiro manualmente
                        u64 secs = 0;
                        for (int j = 6; cmd_buf[j] >= '0' && cmd_buf[j] <= '9'; j++) {
                            secs = (secs * 10) + (cmd_buf[j] - '0');
                        }
                        timer_set_alarm(secs);
                    }
                    // Comando inválido
                    else {
                        printk(LOG_INFO, "Comando nao encontrado: %s\n", cmd_buf);
                    }
                }

                // Prepara o buffer para o próximo comando e imprime o prompt novamente
                cmd_idx = 0;
                print("> ");
            } 
            // Tratamento bônus/ux: Backspace (0x08) ou DEL (0x7F)
            else if (c == '\b' || c == 0x7F) {
                if (cmd_idx > 0) {
                    cmd_idx--;
                    // Apaga o caractere da tela escrevendo espaço por cima
                    serial_putc('\b');
                    serial_putc(' ');
                    serial_putc('\b');
                }
            }
            // Qualquer outro caractere
            else {
                // Guarda no buffer do comando (prevenindo overflow)
                if (cmd_idx < 255) {
                    cmd_buf[cmd_idx++] = c;
                }
            }
        }
    }
}