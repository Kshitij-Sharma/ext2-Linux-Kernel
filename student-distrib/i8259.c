/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* Initialize the 8259 PIC */
void i8259_init(void) {
    int i;

    // disables processor interrupts 
    // cli();

    // disables all PIC interrupts
    for(i = 0; i < SLAVE_PORT_MAX; i++)         disable_irq(i);
    
    /* initialize master with 4 ICWs */
    outb(ICW1, MASTER_8259_CMD_PORT );          // ICW1: tells the PIC it's being initalized, operating in cascade mode, etc.
    outb(ICW2_MASTER, MASTER_8259_DATA_PORT);   // ICW2: provides PIC with high bits of interrupt vector numbers (0x20-0x27)
    outb(ICW3_MASTER, MASTER_8259_DATA_PORT);   // ICW3: tells PIC the specific IR pin used in master/slave relationship
    outb(ICW4, MASTER_8259_DATA_PORT);          // ICW4: specifies 8086 protocol, normal EOI signaling, etc. 

    /* initialize slave with 4 ICWs */
    outb(ICW1, SLAVE_8259_CMD_PORT);            // ICW1: tells the PIC it's being initalized, operating in cascade mode, etc.
    outb(ICW2_SLAVE, SLAVE_8259_DATA_PORT);     // ICW2: provides PIC with high bits of interrupt vector numbers (0x28-0x2F)
    outb(ICW3_SLAVE, SLAVE_8259_DATA_PORT);     // ICW3: tells PIC the specific IR pin used in master/slave relationship
    outb(ICW4, SLAVE_8259_DATA_PORT);           // ICW4: specifies 8086 protocol, normal EOI signaling, etc. 
    
    // re-enables all PIC interrupts
    for(i = 0; i < SLAVE_PORT_MAX; i++)         enable_irq(i);

    // printf("%x\n", inb(MASTER_8259_DATA_PORT));
    // printf("%x\n", inb(SLAVE_8259_DATA_PORT));

    
    // re-enables processor interrupts
    // sti();
}

/* Enable (unmask) the specified IRQ */
void enable_irq(uint32_t irq_num) {
    uint16_t    port;
    uint8_t     value;

    // handles incorrect calls
    if (irq_num >= SLAVE_PORT_MAX || irq_num < 0)   return; 
    // first 8 IRQs correspond to master port
    if(irq_num < MASTER_PORT_MAX)                   port = MASTER_8259_DATA_PORT;
    // second 8 ports correspond to slave: so if irq_num > 8, we want to make it the corresponding slave port
    else{
        irq_num -= MASTER_PORT_MAX;
        port = SLAVE_8259_DATA_PORT;
    }

    value = inb(port) & ~(1 << irq_num); // turning the bit corresponding to the IR port off 
    outb(value, port); // write back to the port
}

/* Disable (mask) the specified IRQ */
void disable_irq(uint32_t irq_num) {
    uint16_t    port;
    uint8_t     value;

    // handles incorrect calls
    if (irq_num >= SLAVE_PORT_MAX || irq_num < 0)  return; 
    // first 8 IRQs correspond to master port
    if(irq_num < MASTER_PORT_MAX)     port = MASTER_8259_DATA_PORT;
    // second 8 ports correspond to slave: so if irq_num > 8, we want to make it the corresponding slave port
    else{
        irq_num -= MASTER_PORT_MAX; // make the IRQ number a value between 0 and 7 
        port = SLAVE_8259_DATA_PORT;
    }

    value = inb(port) | (1 << irq_num); // turning the bit corresponding to the IR port on
    outb(value, port); // write back to the port
}

/* Send end-of-interrupt signal for the specified IRQ */
void send_eoi(uint32_t irq_num) {
    if (irq_num > 7)
        outb(EOI, SLAVE_8259_CMD_PORT);
    outb(EOI, MASTER_8259_CMD_PORT);
}
