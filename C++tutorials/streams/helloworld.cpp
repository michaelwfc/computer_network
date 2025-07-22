#include <iostream>
// C++ way
int hello_cpp() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}

#include <stdio.h>
#include <stdlib.h>
// C way
int hello_c() {
    printf("%s","Hello, World!\n");
    return 0;
}

int hello_asm() {
	asm(".LC0:\n\t"
			".string \"Hello, world!\"\n\t"
		"main:\n\t"
			"push rbp\n\t"
			"mov rbp, rsp\n\t"
			"sub rsp, 16\n\t"
			"mov DWORD PTR [rbp-4], edi\n\t"
			"mov QWORD PTR [rbp-16], rsi\n\t"
			"mov edi, OFFSET FLAT:.LC0\n\t"
			"call puts\n\t");
	return EXIT_SUCCESS;
}


int main() {
    hello_cpp();
    hello_c();
    hello_asm();
    return 0;
}