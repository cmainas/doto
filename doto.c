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
	char month;
	char day;
	char done;
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

char display_menu(int ecnt)
{
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	char action = -1;
	printf("Hello to doto menu\n");
	printf("Total jobs: %d\n ", ecnt);
	printf("Today is %s\n", asctime(tm));
//	printf("Today is: %d-%d-%d %d:%d:%d\n", tm.tm_mday, 
//			tm.tm_mon++, tm.tm_year + 1900, tm.tm_hour,
//		       	tm.tm_min, tm.tm_sec);
	printf("Menu: \n");
	printf("\t0: Quit doto\n");
	printf("\t1: Print today stuff \n");
	printf("\t2: Print week stuff \n");
	printf("\t3: Print all stuff \n");
	printf("\t4: Add a todo\n");
	printf("\t5: Delete a todo\n");
	printf("\t6: Mark a todo as done\n");
	printf("Choose between 0-6: ");
	action = getchar();
	getchar();
	return action - '0';
}

int open_mem_file(int flags)
{
	const char *homedir = getenv("HOME");
	char memfile[50];
	int fd;
	if(!homedir) 
		homedir = getpwuid(getuid())->pw_dir;
	strcpy(memfile, homedir);
	strcat(memfile, "/.doto_file");
	//fprintf(stderr, "Hi: %s\n", memfile);
	fd = open(memfile, flags, 0644);
	if(fd == -1) {
		perror("memory file");
		exit(1);
	}
	//printf("fl: %d\n", lseek(fd, 0, SEEK_END));
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
			//printf("mpou\n");
			tries++;
			if(tries == 3) {
				tries -= 3;
				memcpy(buf, &tries, sizeof(int));
				//printf("hi---------------\n");
				return;
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
	//int *k=buf;
	//printf("brrrrrrrr: %d\n", (int)*k);
	while(n) {
		bts_written = write(fd, buf + bts_written, n);
		if(bts_written == -1) {
			perror("write memory file");
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
	read_nbytes(fd, &(n_entry->month), sizeof(char));
	read_nbytes(fd, &(n_entry->day), sizeof(char));
	read_nbytes(fd, &(n_entry->done), sizeof(char));
	read_nbytes(fd, &(n_entry->descr), 50*sizeof(char));
	return n_entry;
}

struct entry *read_all_entries(int fd, int *cnt)
{
	if(!(*cnt))
		return NULL;
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	struct entry *first, **dfirst;
	int i;
	char ok = 1;
	while(ok){
		first = read_entry(fd);
		if(first->year < tm->tm_year + 1900 || 
				first->month < tm->tm_mon + 1 ||
				first->day < tm->tm_mday)
			(*cnt)--;
		else 
			ok = 0;
	}
	dfirst = &(first->next);
	for(i = 0; i < *cnt - 1; i++) {
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
	struct tm *tm = localtime(&t);
	int cnt=0;
	printf("Today tasks:\n");
	printf("-------------------------------------\n"
		"|id	|Day	|Month	|Year	|Done?	|Description|\n"
		"-------------------------------------\n");
	while(tmp) {
		if(tmp->year > (tm->tm_year + 1900))
			break;
		if(tmp->month > (tm->tm_mon + 1))
			break;
		if(tmp->day > tm->tm_mday)
			break;
		printf("|%d\t|%d\t|%d\t|%d\t|%d\t|%s", cnt++, tmp->day, 
				tmp->month, tmp->year, tmp->done, tmp->descr);
		printf("-------------------------------------\n");
		tmp = tmp->next;
	}
	return;
}

void one_week_from_now(int *y, char *m, char *d)
{
	if(*d < 23){
		*d += 6;
		return;
	}
	if(*m == 4 | *m == 6 | *m == 9 | *m == 11) {
		if(*d < 25) {
			(*d) += 6;
			return;
		}
		(*m)++;
		(*d) = (*d) - 24;
		return;
	} else if (*m == 2) {
		(*m)++;
		(*d) = (*d) - 23;
		return;
	} else {
		if(*d < 26) {
			(*d) += 6;
			return;
		}
		if(*m == 12){
			(*m) = 1;
			(*y)++;
		} else {
			(*m)++;
		}
		(*d) = (*d) - 25;
	}
	return;
}

void print_week(struct entry *ens)
{
	int cnt = 0;
	struct entry *tmp = ens;
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	printf("For this week you have to:\n");
	printf("-------------------------------------\n"
		"|id	|Day	|Month	|Year	|Done?	|Description|\n"
		"-------------------------------------\n");
	int yr = tm->tm_year + 1900;
	char mn = tm->tm_mon + 2, d = tm->tm_mday+17;
	printf("%d-%d-%d\n", yr, mn, d);
	one_week_from_now(&yr, &mn, &d);
	//d += 6;
	printf("%d-%d-%d\n", yr, mn, d);
	while(tmp) {
		if(tmp->year > yr)
			break;
		if(tmp->month > mn && tmp->year == yr)
			break;
		if((tmp->day > d) && (tmp->month == mn))
			break;
		printf("|%d\t|%d\t|%d\t|%d\t|%d\t|%s", cnt++, tmp->day, 
				tmp->month, tmp->year, tmp->done, tmp->descr);
		printf("-------------------------------------\n");
		tmp = tmp->next;
	}
	return;
}

void print_all(struct entry *ens)
{
	int cnt = 0;
	struct entry *tmp = ens;
	printf("All tasks:\n");
	printf("-------------------------------------\n"
		"|Id	|Day	|Month	|Year	|Done?	|Description|\n"
		"-------------------------------------\n");
	while(tmp) {
		printf("|%d\t|%d\t|%d\t|%d\t|%d\t|%s", cnt++, tmp->day, 
				tmp->month, tmp->year, tmp->done, tmp->descr);
		printf("-------------------------------------\n");
		tmp = tmp->next;
	}
	return;
}

struct entry *get_entry()
{
	struct entry *nentry = new_entry();
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
	nentry->done = 0;
	return nentry;
}

struct entry *add(struct entry *ens)
{
	struct entry *tmp = ens, **dtmp = &ens, *nentry = get_entry();
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

void close_mem_file(int f)
{

	if(close(f) == -1) {
		perror("close memory file:");
		exit(1);
	}
	return;
}

void update_mem_file(int cnt, struct entry *ens)
{
	int f = open_mem_file(O_WRONLY);
	struct entry *tmp = ens;
	write_nbytes(f, &cnt, sizeof(int));
	while(tmp) {
		write_nbytes(f, &(tmp->year), sizeof(int));
		write_nbytes(f, &(tmp->month), sizeof(char));
		write_nbytes(f, &(tmp->day), sizeof(char));
		write_nbytes(f, &(tmp->done), sizeof(char));
		write_nbytes(f, &(tmp->descr), sizeof(tmp->descr));
		tmp = tmp->next;
	}
	close_mem_file(f);
	return;
}

void del(struct entry **ens)
{
	int i, id;
	struct entry **tmp = ens, *t = *ens;
	printf("Add id of entry you want to delete:");
	scanf("%d", &id);
	getchar();
	for(i=0; i<id; i++){
		tmp = &(t->next);
		t = t->next;
	}
	*tmp = t->next;
	free(t);
	return;
}

void done(struct entry *ens)
{
	struct entry *tmp = ens;
	int i, id;
	printf("Add id of entry you want to mark as done:");
	scanf("%d", &id);
	getchar();
	for(i=0; i<id; i++)
		tmp = tmp->next;
	tmp->done = 1;
	return;
}

int main()
{
	char choice;
	int fd, entry_cnt = 0;
	struct entry *entries;
	fd = open_mem_file(O_RDONLY | O_CREAT);
	read_nbytes(fd, &entry_cnt, sizeof(entry_cnt));
	entries = read_all_entries(fd, &entry_cnt);
	close_mem_file(fd);
	while(1) {
		choice = display_menu(entry_cnt);
		switch(choice) {
			case 0 : 
				printf("bye: %d\n", entry_cnt);
				update_mem_file(entry_cnt, entries);
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
				entries = add(entries);
				entry_cnt++;
				//printf("add\n");
				break;
			case 5:
				del(&entries);
				entry_cnt--;
				break;
			case 6:
				done(entries);
				//printf("done\n");
				break;
			default:
				printf("\n%c is not between 0-6\n", choice+'0');
		}
	}
}
