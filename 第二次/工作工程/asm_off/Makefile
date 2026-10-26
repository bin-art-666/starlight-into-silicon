led.bin:led.s
	arm-linux-gnueabihf-gcc -mcpu=cortex-a7 -marm -fno-pie -ffreestanding -g -c led.s -o led.o
	arm-linux-gnueabihf-ld -e _start -Ttext 0X87800000 led.o -o led.elf
	arm-linux-gnueabihf-objcopy -O binary -S -g led.elf led.bin
	arm-linux-gnueabihf-objdump -D led.elf > led.dis
clean:
	rm -rf *.o led.bin led.elf led.dis
	
	
