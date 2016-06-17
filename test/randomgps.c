#include <stdio.h>
#include <stdlib.h>
#include <linux/gps.h>

#define SET_GPS_LOCATION 384

int main(){
	time_t t;
	struct gps_location loc;

	srand(time(&t));

	loc.latitude = ((double)rand())/((double)RAND_MAX) * 180 - 90.0;
	loc.longitude = ((double)rand())/((double)RAND_MAX) * 360 - 180.0;
	loc.accuracy = 1;

	printf("(%f, %f, %.2f) : %d\n", loc.latitude, loc.longitude, loc.accuracy, syscall(SET_GPS_LOCATION, &loc));

}
