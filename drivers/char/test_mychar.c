#ifndef QUERY_IOCTL_H
#define QUERYt_IOCTL_H
#include <linux/ioctl.h>

typedef struct {
	int id,
	int age,
} student;

#define MYCHAR_IOCTL_R _IOR('k', 1, student) 
#define MYCHAR_IOCTL_W _IOW('k', 1, student) 

void get_vars(int fd){
	student s1;
	ioctl(fd, MYCHAR_IOCTL_R, &s1);
	printf("\n id = %d\n age = %d",s1.id,s1.age);

}
void set_vars(int fd){
	student s1;
	scanf("%d %d",&(s1.id),&(s1.age));
	ioctl(fd, MYCHAR_IOCTL_W, &s1);
	printf("vals set");
}

int main(int argc, char* argv[])
{
	char* name = "/dev/sirius_dev";
	int fd = open(name, O_RDWR);
	switch (argv[1]){
	case 1:
		get_vars(fd);
		break;
	case 2:
		set_vars(fd);
		break;
	}
	close(fd);
	return 0;
}
