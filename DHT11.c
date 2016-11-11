#include "DHT11.h"

int main(int argc, char* argv[])
{
	int* dhtData = NULL;

	/* Memory Mapping */
	setup_io();

	/* Try to get right data */
	while(!(dhtData = dht11()));

	/* Print Result */
	printf ("Humidity = %d.%d %%, Temperature = %d.%d C\n", dhtData[0], dhtData[1], dhtData[2], dhtData[3]);

	/* Free Data */
	free(dhtData);

	return 0;
}

int* dht11()
{
	bool checker;

	int* binData;
	int* decData;

	/* DHT11 initialization */
	initDHT11();

	/* Data Reading */
	binData = readData();

	/* Binary Data to Decimal Data */
	decData = bin2dec(binData);

	/* Parity Check */
	checker = chkPrt(decData);

	/* Free binData */
	free (binData);

	if (checker)
		return decData;
	return 0;
}

int bitTimer(bool* data)
{
	int counter = 0;
	bool buf = 0;

	do
	{
		myusleep(1);
		buf = GET_GPIO(DHTPIN);
		counter++;
	} while(!(*data ^ buf) && counter < 255);

	if (counter == 255)
		return -1;

	*data = buf;

	return counter;
}

void initDHT11()
{
	bool bit = 1;

	OUT_GPIO(DHTPIN);
	GPIO_CLR = 1 << DHTPIN;
	usleep(18 * 1000);
	GPIO_SET = 1 << DHTPIN;
	myusleep(40);
	INP_GPIO(DHTPIN);

	bitTimer(&bit);
	bitTimer(&bit);
	bitTimer(&bit);
}

int* readData()
{
	int i;
	bool bit = 0;
	int bittime[80] = {0,};
	int* binData = (int*) malloc (sizeof(int) * 40);

	for (i = 0; i < 40; i++)
		binData[i] = 0;

	for (i = 0; i < 80; i++)
		bittime[i] = bitTimer(&bit);

	for (i = 0; i < 80; i += 2)	
	{
		if (bittime[i] < bittime[i + 1])
			binData [i / 2] = 1;

		else if (bittime[i] < 0)
			continue;
	}

	return binData;
}

int* bin2dec(int* binData)
{
	int i, j;
	int* decData = (int*) malloc (sizeof(int) * 5);

	j = -1;

	for (i = 0; i < 4; i++)
		decData[i] = 0;

	for (i = 0; i < 40; i++)
	{
		if (!(i % 8))
			j++;

		if (binData[i])
		{
			decData[j] <<= 1;
			decData[j] |= 1;
		}

		else
			if (decData[j])
				decData[j] <<= 1;
	}

	return decData;
}

bool chkPrt(int* decData)
{
	int i, buf = 0;
	
	for (i = 0; i < 4; i++)
		buf += decData[i];

	if (buf ^ decData[4] || !(decData[0] + decData[2]))
		return 0;
	return 1;
}

void setup_io()
{
	int  mem_fd;
	void *gpio_map;

	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("can't open /dev/mem \n");
		exit(-1);
	}

	/* mmap GPIO */
	gpio_map = mmap(
			NULL,             //Any adddress in our space will do
			BLOCK_SIZE,       //Map length
			PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
			MAP_SHARED,       //Shared with other processes
			mem_fd,           //File to map
			GPIO_BASE         //Offset to GPIO peripheral
		       );

	close(mem_fd); //No need to keep mem_fd open after mmap

	if (gpio_map == MAP_FAILED) {
		printf("mmap error %d\n", (int)gpio_map);//errno also set!
		exit(-1);
	}

	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;


} // setup_io

void myusleep (unsigned int usec)
{
	struct timeval tNow, tLong, tEnd ;

	gettimeofday (&tNow, NULL) ;
	tLong.tv_sec  = usec / 1000000 ;
	tLong.tv_usec = usec % 1000000 ;
	timeradd (&tNow, &tLong, &tEnd) ;

	while (timercmp (&tNow, &tEnd, <))
		gettimeofday (&tNow, NULL) ;
}
