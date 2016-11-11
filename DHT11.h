#define BCM2708_PERI_BASE        0x3F000000
#define GPIO_BASE                (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/time.h>

#define BLOCK_SIZE 4 * 1024

#define MAXTIMINGS	85
#define DHTPIN		4

#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0
#define GET_GPIO(g) (*(gpio+13)&(1<<g)) // 0 if LOW, (1<<g) if HIGH

volatile unsigned *gpio;

int* dht11();				// Body of DHT11
int bitTimer(bool* data);		// Timer for another bit signal
void initDHT11();			// DHT11 initialization
int* readData();			// Read Data from DHT11
int* bin2dec(int* binData);		// Change Binary Data to Decimal Data
bool chkPrt(int* decData);		// Check whether right data

void setup_io();			// Memory Mapping
void myusleep (unsigned int usec);	// Customized Microsecond Timer
