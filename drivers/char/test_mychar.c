#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include "test_mychar.h" 

void get_vars(int fd){
	student s1;
	ioctl(fd, MYCHAR_IOCTL_R, &s1);
	printf("\n id = %d\n age = %d",s1.id,s1.age);

}
void set_vars(int fd){
	student s1;
	scanf("%d %d",&(s1.id),&(s1.age));
	ioctl(fd, MYCHAR_IOCTL_W, &s1);
	printf("\nid=%d age=%d",s1.id,s1.age);
	printf("\nvals set");
}

int main(int argc, char* argv[])
{
	char *name = "/dev/sirius_dev";

	int fd = open(name, O_RDWR);
	printf("opened");
	if(strcmp(argv[1], "1") == 0){
		get_vars(fd);
	}else if(strcmp(argv[1], "2") == 0){
		set_vars(fd);
	}
	close(fd);
	return 0;
}
