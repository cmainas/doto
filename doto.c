#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <strings.h>

struct entry {
	int year;
	int month;
	int day;
	struct *next;
	char descr[50];
}

struct new_entry() 
{
	struct entry  *new;
	new = malloc(sizeof(struct entry));
	if(!new) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	return new;
}

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

int open_mem_file()
{
	const char *homedir = getenv("HOME");
	char memfile[50];
	int fd;
	if(!homedir) 
		homedir = getpwuid(getuid())->pw_dir;
	strcpy(memfile, homedir);
	strcat(memfile, ".doto_file");
	fd = open(memfile, O_RDWR | O_TRUNC | O_CREAT);
	if(fd == -1) {
		perror("memory file");
		exit(1);
	}
	return fd;
}

void read_nbytes(int fd, void *buf, ssize_t n)
{
	ssize_t bts_read = 0;
	while(n) {
		bts_read = read(fd, buf + bts_read, n);
		if(bts_read == -1) {
			perror("read memory file");
			exit(1);
		}
		n -= bts_read;
	}
	return;
}

struct entry read_entry(int fd)
{
	struct entry *n_entry = new_entry();
	ssize_t read_bts;
	read_nbytes(fd, &(n_entry->year), sizeof(int));
	read_nbytes(fd, &(n_entry->month), sizeof(int));
	read_nbytes(fd, &(n_entry->day), sizeof(int));
	read_nbytes(fd, &(n_entry->descr), 50*sizeof(char));
	return n_entry;
}

struct entry read_all_entries(int fd, int cnt)
{
	if(!cnt)
		return NULL;
	struct entry *first, **dfirst;
	int i;
	first = read_entry(fd);
	**dfirst = &(first->next);
	for(i = 0; i < cnt - 1; i++) {
		*dfirst = read_entry(fd);
		**dfirst = &((*dfirst)->next)
	}
	*dfirst = NULL;
	return first;
}

int main()
{
	char choice;
	int fd, entry_cnt;
	struct entry *entries;
	read_nbytes(fd, &entry_cnt, sizeof(entry_cnt));
	entries = read_all_entries(fd, entry_cnt);
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
