#include <stdio.h>
#include <stdlib.h>
#include <linux/gps.h>

#define GET_GPS_LOCATION 385

int main(int argc, char *argv[])
{
	if (argc != 2)
		return -1;

	struct gps_location *loc = malloc(sizeof(struct gps_location));
	if (syscall(GET_GPS_LOCATION, argv[1], loc) < 0) {
		printf("no such file\n");
		return -1;
	}

	printf("latitude: %lf, longitude: %lf, accuracy: %f\n",
			loc->latitude, loc->longitude, loc->accuracy);

	printf("http://www.google.com/maps/place/%.5lf,%.5lf\n",loc->latitude, loc->longitude);

	return 0;
}
