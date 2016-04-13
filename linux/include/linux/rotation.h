struct dev_rotation{
	int degree;
};

struct rotation_range{
	struct dev_rotation rot;
	unsigned int degree_range;
};

void exit_rotlock(void);
