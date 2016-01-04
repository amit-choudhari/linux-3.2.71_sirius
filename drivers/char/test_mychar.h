#include <linux/ioctl.h>

typedef struct{
	int id;
	int age;
} student;

#define MYCHAR_IOCTL_R _IOR('k', 1, student) 
#define MYCHAR_IOCTL_W _IOW('k', 1, student) 
