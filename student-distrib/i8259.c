/* i8259.c - Functions to interact with the 8259 interrupt controller
 * vim:ts=4 noexpandtab
 */

#include "i8259.h"

/* Interrupt masks to determine which interrupts are enabled and disabled */
uint8_t master_mask; /* IRQs 0-7  */
uint8_t slave_mask;  /* IRQs 8-15 */

/* i8259_init(void(
        INPUTS: none
        OUTPUTS: none
        SIDE EFFECTS: initializes the master and slave i8259 PICs. enables interrupts from keyboard and RTC
*/
void i8259_init(void) {
    int i;

    // disables all PIC interrupts
    for(i = 0; i < SLAVE_PORT_MAX; i++)         disable_irq(i);
    
    // alternate way to disable all interrupts
    // outb(0xFF, MASTER_8259_DATA_PORT);
    // outb(0xFF, SLAVE_8259_DATA_PORT);

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
    
    /* turns on PIC interrupts for slave IRQ, RTC, keyboard*/
    for(i = 0; i < SLAVE_PORT_MAX; i++)         disable_irq(i);
    enable_irq(IRQ_SLAVE);          // enables slave IRQ
    enable_irq(IRQ_RTC);            // enables RTC IRQ
    enable_irq(IRQ_KEYBOARD);       // enables keyboard IRQ

}

/* enable_irq(irq_num)
        INPUTS: IRQ line that we want to enable
        OUTPUTS: none
        SIDE EFFECTS: enables interrupts on either the master or slave PIC specified from the parameter
*/
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

    value = inb(port) & ~(1 << irq_num);    // turning the bit corresponding to the IR port off 
    outb(value, port);                      // write back to the port
}

/* disable_irq(irq_num)
        INPUTS: IRQ line that we want to disable
        OUTPUTS: none
        SIDE EFFECTS: disables interrupts on either the master or slave PIC specified from the parameter
*/
void disable_irq(uint32_t irq_num) {
    uint16_t    port;
    uint8_t     value;

    // handles incorrect calls
    if (irq_num >= SLAVE_PORT_MAX || irq_num < 0)  return; 
    // first 8 IRQs correspond to master port
    if(irq_num < MASTER_PORT_MAX)     port = MASTER_8259_DATA_PORT;
    // second 8 ports correspond to slave: so if irq_num > 8, we want to make it the corresponding slave port
    else{
        irq_num -= MASTER_PORT_MAX; 
        port = SLAVE_8259_DATA_PORT;
    }

    value = inb(port) | (1 << irq_num);     // turning the bit corresponding to the IR port on
    outb(value, port);                      // write back to the port
}

/* send_eoi(irq_num)
        INPUTS: IRQ line we want to send an EOI to
        OUTPUTS: none
        SIDE EFFECTS: sends EOI signal to master or master and slave PIC at the specified IRQ line
*/
void send_eoi(uint32_t irq_num) {
    /* when sending EOI to slave, we send to IRQ line on slave PIC 
    and slave IRQ line on master PIC */
    if (irq_num >= MASTER_PORT_MAX){
        outb((EOI | (irq_num-MASTER_PORT_MAX)), SLAVE_8259_CMD_PORT);
        outb((EOI | IRQ_SLAVE) , MASTER_8259_CMD_PORT);  
    } 
    /* when sending EOI to master PIC, we send it directly to the IRQ line */
    else        outb((EOI | irq_num), MASTER_8259_CMD_PORT);
}
