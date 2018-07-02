#include "main.h"
#include "system_init.h"
#include "spi_prog.h"
int main(void)
{
	init_system();
    while(1)
    {
    	process_program();
    }
}
