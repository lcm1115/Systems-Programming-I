void clock_isr(int vector, int code);

void serial_isr(int vector, int code);

void serial_putchar(char ch);

void serial_puts(char* buffer);

int serial_getchar(void);

void serial_putd(int d);

void serial_putx(double x);

void init(void);
