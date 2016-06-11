#include <stdio.h>
#include <stdlib.h>
#include <linux/gps.h>

#define GET_GPS_LOCATION 385
#define SET_GPS_LOCATION 384

int main(int argc, char *argv[])
{
	if (argc != 2)
		return -1;

	struct gps_location *loc = malloc(sizeof(struct gps_location));
	syscall(GET_GPS_LOCATION, argv[1], loc);

	printf("reset (%lf, %lf, %f): %d\n",
			loc->latitude, loc->longitude, loc->accuracy, 
			syscall(SET_GPS_LOCATION, loc));

	free(loc);
	return 0;
}
