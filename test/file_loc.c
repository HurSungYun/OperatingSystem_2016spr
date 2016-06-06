#include <stdio.h>
#include <stdlib.h>
#include <linux/gps.h>

#define GET_GPS_LOCATION 385

int main(int argc, char *argv[])
{
	if (argc != 2)
		return -1;

	struct gps_location *loc = malloc(sizeof(struct gps_location));
	syscall(GET_GPS_LOCATION, argv[1], loc);

	printf("latitude: %lf, longitude: %lf, accuracy: %f\n",
			loc->latitude, loc->longitude, loc->accuracy);

	return 0;
}
