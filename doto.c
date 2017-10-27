#include <stdio.h>
#include <time.h>

char display_menu()
{
	time_t t = time(NULL);
	struct tm *time = localtime(&t);
	char action = -1;
	printf("Hello to doto menu\n");
	printf("Today is %s\n", asctime(time));
//	printf("Today is: %d-%d-%d %d:%d:%d\n", time.tm_mday, 
//			time.tm_mon++, time.tm_year + 1900, time.tm_hour,
//		       	time.tm_min, time.tm_sec);
	printf("Menu: \n");
	printf("\t0: Quit doto\n");
	printf("\t1: Print today stuff \n");
	printf("\t2: Print week stuff \n");
	printf("\t3: Print month stuff \n");
	printf("\t4: Edit a todo\n");
	printf("\t5: Add a todo\n");
	printf("\t6: Mark a todo as done\n");
	printf("Choose between 0-6: ");
	action = getchar();
	getchar();
	return action - '0';
}

int main()
{
	char choice;
	while(1) {
		choice = display_menu();
		switch(choice) {
			case 0 : 
				printf("bye\n");
				return 0;
			case 1:
				//print_today();
				printf("print today\n");
				break;
			case 2:
				//print_week();
				printf("print week\n");
				break;
			case 3:
				//print_month();
				printf("print month\n");
				break;
			case 4:
				//edit();
				printf("edit\n");
				break;
			case 5:
				//add();
				printf("add\n");
				break;
			case 6:
				//done();
				printf("done\n");
				break;
			default:
				printf("\n%c is not between 0-6\n", choice+'0');
		}
	}
}
