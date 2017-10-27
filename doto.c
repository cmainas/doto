#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <fcntl.h>
#include <string.h>

struct entry {
	int year;
	int month;
	int day;
	struct entry *next;
	char descr[50];
};

struct entry *new_entry()
{
	struct entry  *new;
	new = malloc(sizeof(struct entry));
	if(!new) {
		fprintf(stderr, "Out of memory\n");
		exit(1);
	}
	new->next = NULL;
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
	printf("\t3: Print all stuff \n");
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
	strcat(memfile, "/.doto_file");
	fprintf(stderr, "Hi: %s\n", memfile);
	fd = open(memfile, O_RDWR | O_TRUNC | O_CREAT, 0644);
	if(fd == -1) {
		perror("memory file");
		exit(1);
	}
	return fd;
}

void read_nbytes(int fd, void *buf, ssize_t n)
{
	ssize_t bts_read = 0;
	int tries = 0;
	while(n) {
		bts_read = read(fd, buf + bts_read, n);
		if(bts_read == -1) {
			perror("read memory file");
			exit(1);
		}
		if(!bts_read) {
			tries++;
			if(tries == 3) {
				tries -= 3;
				memcpy(buf, &tries, sizeof(int));
				break;
			}
		} else {
			n -= bts_read;
			tries = 0;
		}
	}
	return;
}

void write_nbytes(int fd, void *buf, ssize_t n)
{
	ssize_t bts_written = 0;
	while(n) {
		bts_written = write(fd, buf + bts_written, n);
		if(bts_written == -1) {
			perror("read memory file");
			exit(1);
		}
		n -= bts_written;
	}
	return;
}

struct entry *read_entry(int fd)
{
	struct entry *n_entry = new_entry();
	ssize_t read_bts;
	read_nbytes(fd, &(n_entry->year), sizeof(int));
	read_nbytes(fd, &(n_entry->month), sizeof(int));
	read_nbytes(fd, &(n_entry->day), sizeof(int));
	read_nbytes(fd, &(n_entry->descr), 50*sizeof(char));
	return n_entry;
}

struct entry *read_all_entries(int fd, int cnt)
{
	if(!cnt)
		return NULL;
	struct entry *first, **dfirst;
	int i;
	first = read_entry(fd);
	dfirst = &(first->next);
	for(i = 0; i < cnt - 1; i++) {
		*dfirst = read_entry(fd);
		dfirst = &((*dfirst)->next);
	}
	*dfirst = NULL;
	return first;
}

void print_today(struct entry *ens)
{
	struct entry *tmp = ens;
	time_t t = time(NULL);
	struct tm time = *localtime(&t);
	printf("For today you have to:\n");
	while(tmp) {
		if(tmp->year > time.tm_year + 1900)
			break;
		if(tmp->month > time.tm_mon + 1)
			break;
		if(tmp->day > time.tm_mday)
			break;
		printf("\t %s\n", tmp->descr);
		tmp = tmp->next;
	}
	printf("---------------------------\n End of tasks for today\n");
	return;
}

void print_week(struct entry *ens)
{
	struct entry *tmp = ens;
	time_t t = time(NULL);
	struct tm time = *localtime(&t);
	printf("For this week you have to:\n");
	while(tmp) {
		if(tmp->year > time.tm_year + 1900)
			break;
		if(tmp->month > time.tm_mon + 1)
			break;
		if(tmp->day > time.tm_mday)
			break;
		printf("\t %s\n", tmp->descr);
		tmp = tmp->next;
	}
	printf("---------------------------\n End of tasks for this week\n");
	return;
}

void print_all(struct entry *ens)
{
	struct entry *tmp = ens;
	printf("All tasks:\n");
	while(tmp) {
		printf("---:\t %s\n", tmp->descr);
		tmp = tmp->next;
	}
	printf("---------------------------\n End of tasks\n");
	return;
}

struct entry *add(struct entry *ens)
{
	struct entry *tmp = ens, **dtmp = &ens, *nentry = new_entry();
	printf("Which year the task ends: ");
	scanf("%d", &(nentry->year));
	getchar();
	printf("Which month the task ends: ");
	scanf("%d", &(nentry->month));
	getchar();
	printf("Which day the task ends: ");
	scanf("%d", &(nentry->day));
	getchar();
	printf("Add a description (50 chars max): ");
	fgets(nentry->descr, sizeof(nentry->descr), stdin);
	//scanf("%s", nentry->descr);
	if(!tmp)
		return nentry;
	while(tmp) {
		if(nentry->year < tmp->year) {
			break;
		} else if (nentry->year > tmp->year) {
			dtmp = &(tmp->next);
			tmp = tmp->next;
		}else {
			if(nentry->month < tmp->month) {
				break;
			} else if (nentry->month > tmp->month) {
				dtmp = &(tmp->next);
				tmp = tmp->next;
			}else {
				if(nentry->day <= tmp->day) {
					break;
				} else if (nentry->day > tmp->day) {
					dtmp = &(tmp->next);
					tmp = tmp->next;
				}
			}
		}
	}
	nentry->next = tmp;
	*dtmp = nentry;
	return ens;
}

void close_mem_file(struct entry *ens, int f)
{

	struct entry *tmp = ens;
	while(tmp) {
		write_nbytes(f, &(tmp->year), sizeof(int));
		write_nbytes(f, &(tmp->month), sizeof(int));
		write_nbytes(f, &(tmp->day), sizeof(int));
		write_nbytes(f, &(tmp->descr), sizeof(tmp->descr));
		tmp = tmp->next;
	}
	if(close(f) == -1) {
		perror("close memory file:");
		exit(1);
	}
	return;
}

int main()
{
	char choice;
	int fd, entry_cnt = 0;
	struct entry *entries;
	fd = open_mem_file();
	read_nbytes(fd, &entry_cnt, sizeof(entry_cnt));
	entries = read_all_entries(fd, entry_cnt);
	while(1) {
		choice = display_menu();
		switch(choice) {
			case 0 : 
				printf("bye\n");
				write_nbytes(fd, &entry_cnt, sizeof(entry_cnt));
				close_mem_file(entries, fd);
				return 0;
			case 1:
				print_today(entries);
				//printf("print today\n");
				break;
			case 2:
				print_week(entries);
				//printf("print week\n");
				break;
			case 3:
				print_all(entries);
				//printf("print month\n");
				break;
			case 4:
				//edit();
				printf("edit\n");
				break;
			case 5:
				entries = add(entries);
				entry_cnt++;
				//printf("add\n");
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
