/* TODO: fill in ORIGIN/LENGTH for FLASH and RAM.
   You confirmed via CubeProgrammer: STM32F401xD/E, 384KB flash.
   Check RM0368 "Memory map" for:
     - flash base address (hint: all STM32 parts start flash at the
       same conventional address - check the datasheet to confirm)
     - RAM base address and size for this specific variant (xD vs xE
       have different RAM sizes - check which one you actually have) */

ENTRY(_reset);
MEMORY {
    FLASH(rx)  : ORIGIN = 0x08000000, LENGTH = 384k
    RAM(rwx)   : ORIGIN = 0x20000000, LENGTH = 96k
}

_estack = ORIGIN(RAM) + LENGTH(RAM);

SECTIONS {
    .vectors : {
        KEEP(*(.vectors))
    } > FLASH

    .text : {
        *(.text*)
        *(.rodata*)
    } > FLASH

    _sidata = LOADADDR(.data);

    .data : {
        _sdata = .;
        *(.data*)
        _edata = .;
    } > RAM AT > FLASH

    .bss : {
        _sbss = .;
        *(.bss*)
        *(COMMON)
        _ebss = .;
    } > RAM

    . = ALIGN(8);
    _end = .;
}
