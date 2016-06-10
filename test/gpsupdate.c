#include <stdio.h>
#include <linux/gps.h>

#define SET_GPS_LOCATION 384

int main()
{
	int i;
	struct gps_location loc[100];

	/* An array of gps_location with arbitrary values in it */
	for (i = 0 ; i < 100 ; i ++) {
		loc[i].latitude = i * 5 + 1;
		loc[i].longitude = i * 3 + 1;
		loc[i].accuracy = 1;
	}

	/* Round-robin through the array to test the system call */
	for (i = 0 ; ; i = (i + 1) % 100) {
		printf("%d\n", syscall(SET_GPS_LOCATION, loc+i));
		sleep(5);
	}
	return 0;
}
