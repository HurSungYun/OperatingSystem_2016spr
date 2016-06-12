#include <stdio.h>
#include <linux/gps.h>

#define SET_GPS_LOCATION 384

int main()
{
	struct gps_location loc;

	printf("Input lat, long, acc\n");
	scanf("%lf %lf %f", &loc.latitude, &loc.longitude, &loc.accuracy);

	printf("scanf done\n");

	printf("%.6lf %.6lf %.4f\n",loc.latitude, loc.longitude, loc.accuracy);

	printf("(%f, %f, %.2f) : %d\n", loc.latitude, loc.longitude, loc.accuracy, syscall(SET_GPS_LOCATION, &loc));
	return 0;
}
