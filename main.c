#include <stdint.h>  // For uint32_t, uint16_t, etc
#include <stdbool.h>  // For bool, true, false



// Step 1: Defining Macros
#define BIT(x) (1UL << (x)) // Macro to create a bitmask for a given bit position
#define PIN(bank, num) ((((uint32_t)(bank) - 'A') << 8) | (uint32_t)(num)) // Macro to encode bank and pin number into a single value
#define PINNO(pin) (pin & 255) // Macro to extract the pin number from the encoded value
#define PINBANK(pin) (pin >> 8) // Macro to extract the bank letter from the encoded value


#define DELAY_1S  4000000

// Step 2: Creating the GPIO Peripheral Structure
typedef struct gpio {
    volatile uint32_t MODER;    // 0x00: mode register
    volatile uint32_t OTYPER;   // 0x04: output type register
    volatile uint32_t OSPEEDR;  // 0x08: output speed register
    volatile uint32_t PUPDR;    // 0x0C: pull-up/pull-down register
    volatile uint32_t IDR;      // 0x10: input data register
    volatile uint32_t ODR;      // 0x14: output data register  <-- was missing
    volatile uint32_t BSRR;     // 0x18: bit set/reset register
    volatile uint32_t LCKR;     // 0x1C: config lock register
    volatile uint32_t AFRL;     // 0x20: alternate function low
    volatile uint32_t AFRH;     // 0x24: alternate function high
} gpio;

// GPIO(bank) macro -> pointer to the correct gpio struct for a bank
#define GPIO(bank) ((gpio *) (0x40020000 + 0x400 * (bank)))

// Step 3: Defining GPIO States
// enum values per datasheet
typedef enum { 
    GPIO_MODE_INPUT = 0, 
    GPIO_MODE_OUTPUT, 
    GPIO_MODE_AF, 
    GPIO_MODE_ANALOG 
} gpio_mode;


// Step 4: Setting the GPIO Mode
static inline void gpio_set_mode(uint16_t pin, uint8_t mode) {
    gpio * gpioPort = GPIO(PINBANK(pin));   // gpio bank
    int n = PINNO(pin);
    gpioPort->MODER &= ~(3U << (n * 2));  // Clear the 2 bits for this pin
    gpioPort->MODER |= (mode & 3U) << (n * 2);  // Set new mode
}


// Step 5: Enabling the GPIO peripheral
typedef struct rcc {
    volatile uint32_t CR;            // 0x00
    volatile uint32_t PLLCFGR;       // 0x04
    volatile uint32_t CFGR;          // 0x08
    volatile uint32_t CIR;           // 0x0C
    volatile uint32_t AHB1RSTR;      // 0x10
    volatile uint32_t AHB2RSTR;      // 0x14
    volatile uint32_t AHB3RSTR;      // 0x18   
    volatile uint32_t RESERVED0;     // 0x1C
    volatile uint32_t APB1RSTR;      // 0x20   
    volatile uint32_t APB2RSTR;      // 0x24
    volatile uint32_t RESERVED1[2];  // 0x28, 0x2C
    volatile uint32_t AHB1ENR;       // 0x30
    volatile uint32_t AHB2ENR;       // 0x34
    volatile uint32_t AHB3ENR;       // 0x38   
    volatile uint32_t RESERVED2;     // 0x3C
    volatile uint32_t APB1ENR;       // 0x40
    volatile uint32_t APB2ENR;       // 0x44
    volatile uint32_t RESERVED3[2];  // 0x48, 0x4C
    volatile uint32_t AHB1LPENR;     // 0x50
    volatile uint32_t AHB2LPENR;     // 0x54
    volatile uint32_t AHB3LPENR;     // 0x58
    volatile uint32_t RESERVED4;    // 0x5C, 
    volatile uint32_t APB1LPENR;    //0x60
    volatile uint32_t APB2LPENR;    // 0x64  
    volatile uint32_t RESERVED5[2];    // 0x68, 0x6C
    volatile uint32_t BDCR;  // 0x70
    volatile uint32_t CSR;   // 0x74
    volatile uint32_t RESERVED6[2]; // 0x78, 0x7C
    volatile uint32_t SSCGR;   // 0x80
    volatile uint32_t PLLI2SCFGR;  // 0x84
    volatile uint32_t RESERVED7;  // 0x88
    volatile uint32_t DCKCFGR;     // 0x8C
} rcc;

// RCC macro -> pointer to rcc struct at its base address
#define RCC ((rcc *) (0x40023800) )




// Step 7: Writing to GPIO Pin
static inline void gpio_write(uint16_t pin, bool val) {
    gpio *gpioPin = GPIO(PINBANK(pin));   // GPIO Bank
    // BSRR is a "set/reset" register - writing a 1 to the lower 16 bits
    // sets the pin, writing a 1 to the upper 16 bits (bit+16) clears it.
    gpioPin->BSRR = (1U << PINNO(pin)) << (val ? 0: 16);
}

// read IDR, shift right by pin number, mask with 1.
static inline bool gpio_read(uint16_t pin) {
    gpio *gpioPin = GPIO(PINBANK(pin));
    uint8_t pinNum = PINNO(pin);

    // Read IDR, shift to pin position, and mask to get 0 or 1
    return (gpioPin->IDR >> pinNum) & 1U; 
    // return (gpioPin->IDR & BIT(pinNum)) != 0;
}

static inline void delay(volatile uint32_t counter) {
    while(counter--) asm("nop");
}


// Step 8: Writing the main function
int main() {
    // encode the LED pin. Black Pill onboard LED is PC13.
    uint16_t led = PIN('C', 13); // blue LED
    RCC->AHB1ENR |= BIT(PINBANK(led));  // Enable GPIO clock for LED
    gpio_set_mode(led, GPIO_MODE_OUTPUT);  // set blue LED to output mode

    for(;;) {
        gpio_write(led, true);
        delay(DELAY_1S);
        gpio_write(led, false);
        delay(DELAY_1S);
    }

}


// Step 9: Modifying the Startup code

// startup code
// Attribute to mark the reset handler
// ---- Reset handler ----
// Runs before main(). Must zero .bss and copy .data from flash to RAM.
__attribute__((naked, noreturn, used)) 
void _reset(void) {
    // External symbols defined in linker script
    extern long _sbss;    // Start of BSS
    extern long _ebss;    // End of BSS
    extern long _sdata;   // Start of DATA (RAM)
    extern long _edata;   // End of DATA (RAM)
    extern long _sidata;  // Start of DATA (Flash - initial values)
    
    // STEP 1: Zero-initialize .bss (uninitialized globals)
    for (long *dst = &_sbss; dst < &_ebss; dst++) {
        *dst = 0;
    }
    
    // STEP 2: Copy .data from Flash to RAM (initialized globals)
    for (long *dst = &_sdata, *src = &_sidata; dst < &_edata; ) {
        *dst++ = *src++;  // Copy from Flash to RAM
    }
    
    // STEP 3: Call main() - NEVER RETURNS
    main();
    
    // STEP 4: If main returns, hang forever
    for (;;) { 
        asm("wfi");  // Wait for interrupt (saves power)
    }
}

