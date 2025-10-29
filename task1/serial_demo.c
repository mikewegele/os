#include <stdarg.h>
#include <stdint.h>

/**
 * Debug Unit (DBGU) – AT91RM9200
 *
 * The DBGU is a simple UART dedicated for debugging output.
 * It is always present in the chip and does not require enabling
 * via the peripheral clock controller (PMC).
 *
 * It starts at base address 0xFFFFF200 in the memory map.
 * All registers are located at fixed offsets from this base.
 */
#define DBGU_BASE 0xFFFFF200u

/**
 * DBGU registers (offsets from DBGU_BASE).
 *
 * - CR   (Control Register): start/stop/reset receiver/transmitter
 * - MR   (Mode Register):    configure data bits, parity, stop bits
 * - SR   (Status Register):  check if data available or transmitter ready
 * - RHR  (Receive Holding Register): contains received character
 * - THR  (Transmit Holding Register): write here to send a character
 * - BRGR (Baud Rate Generator Register): divisor for baud rate
 */
#define DBGU_CR   (*(volatile uint32_t*)(DBGU_BASE + 0x00))
#define DBGU_MR   (*(volatile uint32_t*)(DBGU_BASE + 0x04))
#define DBGU_SR   (*(volatile uint32_t*)(DBGU_BASE + 0x14))
#define DBGU_RHR  (*(volatile uint32_t*)(DBGU_BASE + 0x18))
#define DBGU_THR  (*(volatile uint32_t*)(DBGU_BASE + 0x1C))
#define DBGU_BRGR (*(volatile uint32_t*)(DBGU_BASE + 0x20))

/**
 * Bit masks for Control, Status, and Mode registers.
 *
 * CR (Control Register):
 * - RSTRX (bit 2): reset receiver
 * - RSTTX (bit 3): reset transmitter
 * - RXEN  (bit 4): enable receiver
 * - TXEN  (bit 6): enable transmitter
 *
 * SR (Status Register):
 * - RXRDY (bit 0): a character has been received
 * - TXRDY (bit 1): transmitter is ready for a new character
 *
 * MR (Mode Register):
 * - PAR_NONE: no parity, 8 data bits, 1 stop bit (8N1)
 */
#define CR_RSTRX  (1<<2)
#define CR_RSTTX  (1<<3)
#define CR_RXEN   (1<<4)
#define CR_TXEN   (1<<6)

#define SR_RXRDY  (1<<0)
#define SR_TXRDY  (1<<1)

#define MR_PAR_NONE (4<<9)

/**
 * Clock and baud rate settings.
 * - MCK  = Master Clock (e.g. 60 MHz)
 * - BAUD = UART baud rate (commonly 115200)
 */
#ifndef MCK
#define MCK 60000000u
#endif
#ifndef BAUD
#define BAUD 115200u
#endif

/**
 * Initialize the DBGU.
 * - reset RX and TX
 * - configure 8N1 mode (8 data bits, no parity, 1 stop bit)
 * - set baud rate divisor
 * - enable receiver and transmitter
 */
static void dbgu_init(void){
    DBGU_CR = CR_RSTRX | CR_RSTTX;
    DBGU_MR = MR_PAR_NONE;
    DBGU_BRGR = (MCK / (16 * BAUD));
    DBGU_CR = CR_RXEN | CR_TXEN;
}

/**
 * Send a single character via DBGU.
 * Waits until transmitter is ready.
 * Inserts '\r' before '\n' for proper terminal output.
 */
static void dbgu_putc(unsigned char c){
    if (c == '\n'){
        while(!(DBGU_SR & SR_TXRDY)){}
        DBGU_THR = '\r';
    }
    while(!(DBGU_SR & SR_TXRDY)){}
    DBGU_THR = c;
}

/**
 * Receive a single character from DBGU.
 * Waits until a character is available and returns it.
 */
static unsigned char dbgu_getc(void){
    while(!(DBGU_SR & SR_RXRDY)){}
    return (unsigned char)DBGU_RHR;
}

/**
 * Send a null-terminated string via DBGU.
 */
static void dbgu_puts(const char* s){
    if (!s) return;
    while(*s) dbgu_putc((unsigned char)*s++);
}

/**
 * Helper function: convert an unsigned long into a hex string.
 */
static void utoa_hex(unsigned long v, char* buf){
    static const char* d="0123456789abcdef";
    char tmp[2*sizeof(unsigned long)];
    int i=0,j=0;
    if(!v){ buf[0]='0'; buf[1]='\0'; return; }
    while(v){ tmp[i++] = d[v & 0xFu]; v >>= 4; }
    for(j=0;j<i;++j) buf[j] = tmp[i-1-j];
    buf[i]='\0';
}

/**
 * Minimal printf-like function for debug output.
 * Supported formats:
 *   %c – single character
 *   %s – string
 *   %x – unsigned int in hex
 *   %p – pointer in hex (with 0x prefix)
 *   %% – literal percent sign
 */
static int tiny_printf(const char* fmt, ...){
    va_list ap; va_start(ap, fmt);
    for(const char* p=fmt; *p; ++p){
        if(*p!='%'){ dbgu_putc((unsigned char)*p); continue; }
        ++p; if(!*p){ dbgu_putc('%'); break; }
        if(*p=='%'){ dbgu_putc('%'); }
        else if(*p=='c'){ dbgu_putc((unsigned char)va_arg(ap,int)); }
        else if(*p=='s'){ const char* s=va_arg(ap,const char*); if(!s)s="(null)"; dbgu_puts(s); }
        else if(*p=='x'){ unsigned int v=va_arg(ap,unsigned int); char b[9]; utoa_hex(v,b); dbgu_puts(b); }
        else if(*p=='p'){ void* pv=va_arg(ap,void*); unsigned long v=(unsigned long)(uintptr_t)pv; char b[2*sizeof(unsigned long)+1]; dbgu_puts("0x"); utoa_hex(v,b); dbgu_puts(b); }
        else { dbgu_putc('%'); dbgu_putc((unsigned char)*p); }
    }
    va_end(ap);
    return 0;
}

/**
 * Main program:
 * - initialize DBGU
 * - print "Ready"
 * - infinite loop: wait for keypress, acknowledge with formatted output
 */
int main(void){
    dbgu_init();
    dbgu_puts("Ready\r\n");

    for(;;){
        unsigned char c = dbgu_getc();
        tiny_printf("Received character: '%c'\r\n", (int)c);
        tiny_printf("%%c='%c'  %%s=\"%s\"  %%x=%x  %%p=%p\r\n",
                    (int)c,
                    (char[]){ (char)c, 0 },
                    (unsigned int)c,
                    (void*)(uintptr_t)c);
    }
}
