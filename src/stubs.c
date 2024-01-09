/* stubs.c
 This file written 2015 by Axel Isaksson
 This copyright notice added 2015 by F Lundevall
 Latest update 2019-10-14 by E All

 For copyright and licensing, see file LICENSE
*/

/* Non-Maskable Interrupt; something bad likely happened, so hang */
void _nmi_handler() {
	for(;;);
}

/* This function is called upon reset, before .data and .bss is set up */
void _on_reset() {
}

/* This function is called before main() is called, you can do setup here */
void _on_bootstrap() {
}

/* sleep:
   A simple function to create a small delay.
   Very inefficient use of computing resources,
   but very handy in some special cases. */
void sleep(int cyc)
{
	int i;
	for(i = cyc; i > 0; --i);
}
