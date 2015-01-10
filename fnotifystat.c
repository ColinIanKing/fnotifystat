/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * Author: Colin Ian King <colin.king@canonical.com>
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>
#include <errno.h>
#include <mntent.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/fanotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>


#define TABLE_SIZE		(17627)		/* Best if prime */
#define BUFFER_SIZE		(4096)

#define OPT_VERBOSE		(0x00000001)	/* Verbose mode */
#define OPT_DIRNAME_STRIP 	(0x00000002)	/* Remove leading path */
#define OPT_PID			(0x00000004)	/* Select by PID */
#define OPT_SORT_BY_PID		(0x00000008)	/* Sort by PID */
#define OPT_CUMULATIVE		(0x00000010)	/* Gather cumulative stats */
#define OPT_TIMESTAMP		(0x00000020)	/* Show timestamp */
#define OPT_SCALE		(0x00000040)	/* scale data */

#define PROC_CACHE_LIFE		(120)		/* Refresh cached pid info timeout */


/* fnotify file activity stats */
typedef struct file_stat {
	uint64_t	open;			/* Count of opens */
	uint64_t	close;			/* Count of closes */
	uint64_t	read;			/* Count of reads */
	uint64_t	write;			/* Count of writes */
	uint64_t	total;			/* Total count */
	char 		*path;			/* Pathname of file */
	struct file_stat *next;			/* Next item in hash list */
	pid_t		pid;			/* PID of process touching file */
} file_stat_t;

/* process info */
typedef struct proc_info {
	char 		*cmdline;		/* cmdline of process */
	struct proc_info *next;			/* Next item in hash list */
	double		whence;			/* When data acquired */
	pid_t		pid;			/* pid of process */
} proc_info_t;

static const char *app_name = "fnotifystat";	/* name of this tool */
static file_stat_t *file_stats[TABLE_SIZE];	/* hash table of file stats */
static proc_info_t *proc_infos[TABLE_SIZE];	/* hash table of proc infos */
static proc_info_t *proc_list;			/* processes to monitor */
static size_t file_stats_size;			/* number of items in hash table */
static unsigned int opt_flags = OPT_SCALE;	/* option flags */
static volatile bool stop_fnotifystat = false;	/* true -> stop fnotifystat */
static pid_t my_pid;				/* pid of this programme */
static bool named_procs = false;

/*
 *  Attempt to catch a range of signals so
 *  we can clean
 */
static const int signals[] = {
	/* POSIX.1-1990 */
#ifdef SIGHUP
	SIGHUP,
#endif
#ifdef SIGINT
	SIGINT,
#endif
#ifdef SIGQUIT
	SIGQUIT,
#endif
#ifdef SIGILL
	SIGILL,
#endif
#ifdef SIGABRT
	SIGABRT,
#endif
#ifdef SIGFPE
	SIGFPE,
#endif
#ifdef SIGSEGV
	SIGSEGV,
#endif
#ifdef SIGTERM
	SIGTERM,
#endif
#ifdef SIGUSR1
	SIGUSR1,
#endif
#ifdef SIGUSR2
	SIGUSR2,
	/* POSIX.1-2001 */
#endif
#ifdef SIGBUS
	SIGBUS,
#endif
#ifdef SIGXCPU
	SIGXCPU,
#endif
#ifdef SIGXFSZ
	SIGXFSZ,
#endif
	/* Linux various */
#ifdef SIGIOT
	SIGIOT,
#endif
#ifdef SIGSTKFLT
	SIGSTKFLT,
#endif
#ifdef SIGPWR
	SIGPWR,
#endif
#ifdef SIGINFO
	SIGINFO,
#endif
#ifdef SIGVTALRM
	SIGVTALRM,
#endif
	-1,
};

static void pr_error(const char *msg) __attribute__ ((noreturn));
static void pr_nomem(const char *msg) __attribute__ ((noreturn));

static char *count_to_str(
	const double val,
	char *const buf,
	const size_t buflen)
{
	double v = val;
	size_t i;
	static const char scales[] = " KMB";

	if (opt_flags & OPT_SCALE) {
		for (i = 0; i < sizeof(scales) - 1; i++, v /= 1000) {
			if (v <= 500)
				break;
		}
		snprintf(buf, buflen, "%5.1f%c", v, scales[i]);
	} else {
		snprintf(buf, buflen, "%6.1f", v);
	}

	return buf;
}

/*
 *  timeval_to_double()
 *	convert timeval to seconds as a double
 */
static double timeval_to_double(void)
{
	struct timeval tv;

redo:
	errno = 0;			/* clear to be safe */
	if (gettimeofday(&tv, NULL) < 0) {
		if (errno == EINTR)	/* should not occur */
			goto redo;

		/* Silently fail */
		return -1.0;
	}
	return (double)tv.tv_sec + ((double)tv.tv_usec / 1000000.0);
}

/*
 *  get_tm()
 *	fetch tm, will set fields to zero if can't get
 */
static void get_tm(struct tm *tm)
{
	time_t now = time(NULL);

	if (now == ((time_t) -1)) {
		memset(tm, 0, sizeof(struct tm));
	} else {
		(void)localtime_r(&now, tm);
	}
}

/*
 *  pr_error()
 *	print error message and exit fatally
 */
static void pr_error(const char *msg)
{
	fprintf(stderr, "Fatal error: %s: errno=%d (%s)\n",
		msg, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

/*
 *  pr_nomem()
 *	print out of memory error and exit fatally
 */
static void pr_nomem(const char *msg)
{
	fprintf(stderr, "Fatal error: out of memory: %s\n", msg);
	exit(EXIT_FAILURE);
}

/*
 *  get_pid_cmdline
 * 	get process's /proc/pid/cmdline
 */
static char *get_pid_cmdline(const pid_t id)
{
	char buffer[BUFFER_SIZE];
	char *ptr;
	int fd;
	ssize_t ret;

	snprintf(buffer, sizeof(buffer), "/proc/%d/cmdline", id);

	if ((fd = open(buffer, O_RDONLY)) < 0)
		return strdup("<unknown>");

	if ((ret = read(fd, buffer, sizeof(buffer))) <= 0) {
		(void)close(fd);
		return strdup("<unknown>");
	}
	(void)close(fd);

	buffer[sizeof(buffer)-1] = '\0';
	for (ptr = buffer; *ptr && (ptr < buffer + ret); ptr++) {
		if (*ptr == ' ')
			*ptr = '\0';
	}

	return strdup(basename(buffer));
}

/*
 *  handle_sig()
 *      catch signals and flag a stop
 */
static void handle_sig(int dummy)
{
	(void)dummy;	/* Stop unused parameter warning with -Wextra */

	stop_fnotifystat = true;
}

/*
 *  timeval_double()
 *      timeval to a double
 */
static inline double timeval_double(const struct timeval *tv)
{
	return (double)tv->tv_sec + ((double)tv->tv_usec / 1000000.0);
}

/*
 *  hash_pjw()
 *	Hash a string, from Aho, Sethi, Ullman, Compiling Techniques.
 */
static unsigned long hash_pjw(const char *str, const pid_t pid)
{
  	unsigned long h = pid;

	while (*str) {
		unsigned long g;

		h = (h << 4) + (*str);
		if (0 != (g = h & 0xf0000000)) {
			h = h ^ (g >> 24);
			h = h ^ g;
		}
		str++;
	}

  	return h % TABLE_SIZE;
}

/*
 *  proc_info_get()
 *	get info about a specific process
 */
static proc_info_t *proc_info_get(const pid_t pid)
{
	const unsigned long h = pid % TABLE_SIZE;
	proc_info_t *pi = proc_infos[h];

	while (pi) {
		if (pi->pid == pid) {
			double now = timeval_to_double();
			if (now < pi->whence + PROC_CACHE_LIFE)
				return pi;

			/* Deemed "stale", so refresh stats */
			free(pi->cmdline);
			goto update;
		}
		pi = pi->next;
	}
	if ((pi = calloc(1, sizeof(*pi))) == NULL)
		pr_nomem("allocating process information");

	pi->next = proc_infos[h];
	proc_infos[h] = pi;
	pi->pid = pid;

update:
	if ((pi->cmdline = get_pid_cmdline(pid)) == NULL)
		pr_nomem("allocating process command line");
	pi->whence = timeval_to_double();

	return pi;
}

static void proc_list_free(proc_info_t **list)
{
	proc_info_t *pi = *list;

	while (pi) {
		proc_info_t *next = pi->next;

		free(pi->cmdline);
		free(pi);

		pi = next;
	}
	*list = NULL;
}

/*
 *  proc_info_free()
 *	free process info
 */
static void proc_info_free(void)
{
	int i;

	for (i = 0; i < TABLE_SIZE; i++)
		proc_list_free(&proc_infos[i]);
}

/*
 *  file_stat_get()
 *	get file stats on a file touched by a given process by PID.
 *	existing stats are returned, new stats are allocated and returned.
 */
static file_stat_t *file_stat_get(const char *str, const pid_t pid)
{
	const unsigned long h = hash_pjw(str, pid);
	file_stat_t *fs = file_stats[h];

	while (fs) {
		if (!strcmp(str, fs->path) && (pid == fs->pid))
			return fs;
		fs = fs->next;
	}
	if ((fs = calloc(1, sizeof(*fs))) == NULL)
		pr_nomem("allocating file stats");

	if ((fs->path = strdup(str)) == NULL) {
		free(fs);
		pr_nomem("allocating file stats pathname");
	}
	fs->next = file_stats[h];
	fs->pid = pid;
	file_stats[h] = fs;

	file_stats_size++;

	return fs;
}

/*
 *  fnotify_event_init()
 *	initialize fnotify
 */
static int fnotify_event_init(void)
{
	int fan_fd, ret, count = 0;
	FILE* mounts;
	struct mntent* mount;

	if ((fan_fd = fanotify_init(0, 0)) < 0)
		pr_error("cannot initialize fanotify");

	if ((mounts = setmntent("/proc/self/mounts", "r")) == NULL) {
		(void)close(fan_fd);
		pr_error("setmntent cannot get mount points from /proc/self/mounts");
	}

	/*
	 *  Gather all mounted file systems and monitor them
	 */
	while ((mount = getmntent(mounts)) != NULL) {
		ret = fanotify_mark(fan_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
			FAN_ACCESS| FAN_MODIFY | FAN_OPEN | FAN_CLOSE |
			FAN_ONDIR | FAN_EVENT_ON_CHILD, AT_FDCWD,
			mount->mnt_dir);
		if (ret >= 0)
			count++;
	}
	if (endmntent(mounts) < 0) {
		(void)close(fan_fd);
		pr_error("endmntent failed");
	}

	/* This really should not happen, / is always mounted */
	if (!count) {
		fprintf(stderr, "no mount points could be monitored\n");
		(void)close(fan_fd);
		exit(EXIT_FAILURE);
	}

	return fan_fd;
}

/*
 *  fnotify_get_filename()
 *	look up a in-use file descriptor from a given pid
 *	and find the associated filename
 */
static char *fnotify_get_filename(const pid_t pid, const int fd)
{
	char 	buf[256];
	char 	path[PATH_MAX];
	ssize_t len;
	char 	*filename;

	/*
	 * With fnotifies, fd of the file is added to the process
	 * fd array, so we just pick them up from /proc/self. Use
	 * a pid of -1 for self
	 */
	if (pid == -1)
		snprintf(buf, sizeof(buf), "/proc/self/fd/%d", fd);
	else
		snprintf(buf, sizeof(buf), "/proc/%d/fd/%d", pid, fd);

	len = readlink(buf, path, sizeof(path));
	if (len < 0) {
		struct stat statbuf;
		if (fstat(fd, &statbuf) < 0)
			filename = strdup("(unknown)");
		else {
			snprintf(buf, sizeof(buf), "dev: %i:%i inode %ld",
				major(statbuf.st_dev), minor(statbuf.st_dev), statbuf.st_ino);
			filename = strdup(buf);
		}
	} else {
		/*
		 *  In an ideal world we should allocate the path
		 *  based on a lstat'd size, but because this can be
		 *  racey on has to re-check, which involves
		 *  re-allocing the buffer.  Since we need to be
		 *  fast let's just fetch up to PATH_MAX-1 of data.
		 */
		path[len >= PATH_MAX ? PATH_MAX - 1 : len] = '\0';
		filename = strdup(path);
	}
	return filename;
}

/*
 *  fnotify_mask_to_str()
 *	convert fnotify mask to readable string
 */
static const char *fnotify_mask_to_str(const int mask)
{
	static char modes[5];

	modes[0] = (mask & FAN_OPEN) ? 'O' : '-';
	modes[1] = (mask & (FAN_CLOSE_WRITE | FAN_CLOSE_NOWRITE)) ? 'C' : '-';
	modes[2] = (mask & FAN_ACCESS) ? 'R' : '-';
	modes[3] = (mask & (FAN_MODIFY | FAN_CLOSE_WRITE)) ? 'W' : '-';
	modes[4] = '\0';

	return modes;
}

/*
 *  fnotify_event_add()
 *	add a new fnotify event
 */
static int fnotify_event_add(const struct fanotify_event_metadata *metadata)
{
	char 	*filename;
	struct tm tm;
	file_stat_t *fs;

	if ((metadata->fd == FAN_NOFD) && (metadata->fd < 0))
		return 0;
	if (metadata->pid == my_pid)
		goto tidy;
	if (opt_flags & OPT_PID) {
		proc_info_t *p;
		bool found = false;

		for (p = proc_list; p; p = p->next) {
			if (p->pid && p->pid == metadata->pid) {
				found = true;
				break;
			}
		}
		if (!found && named_procs) {
			proc_info_t *cached_p = proc_info_get(metadata->pid);

			for (p = proc_list; p; p = p->next) {
				if (p->cmdline && strstr(p->cmdline, cached_p->cmdline)) {
					found = true;
					break;
				}
			}
		}
		if (!found)
			goto tidy;
	}

 	filename = fnotify_get_filename(-1, metadata->fd);
	if (filename == NULL) {
		(void)close(metadata->fd);
		pr_error("allocating fnotify filename");
	}

	get_tm(&tm);

	fs = file_stat_get(filename, metadata->pid);
	if (metadata->mask & FAN_OPEN) {
		fs->open++;
		fs->total++;
	}
	if (metadata->mask & (FAN_CLOSE_WRITE | FAN_CLOSE_NOWRITE)) {
		fs->close++;
		fs->total++;
	}
	if (metadata->mask & FAN_ACCESS) {
		fs->read++;
		fs->total++;
	}
	if (metadata->mask & (FAN_MODIFY | FAN_CLOSE_WRITE)) {
		fs->write++;
		fs->total++;
	}

	if (opt_flags & OPT_VERBOSE) {
		printf("%2.2d/%2.2d/%-2.2d %2.2d:%2.2d:%2.2d (%4.4s) %5d %s %s\n",
			tm.tm_mday, tm.tm_mon + 1, (tm.tm_year+1900) % 100,
			tm.tm_hour, tm.tm_min, tm.tm_sec,
			fnotify_mask_to_str(metadata->mask),
			metadata->pid, proc_info_get(metadata->pid)->cmdline,
			(opt_flags & OPT_DIRNAME_STRIP) ?
				basename(filename) : filename);
	}
	free(filename);
tidy:
	(void)close(metadata->fd);

	return 0;
}

/*
 *  file_stat_cmp()
 *	compare file stats, sort by total and if they are
 *	the same, sort by pathname
 */
int file_stat_cmp(const void *p1, const void *p2)
{
	file_stat_t **fs1 = (file_stat_t **)p1;
	file_stat_t **fs2 = (file_stat_t **)p2;

	if (opt_flags & OPT_SORT_BY_PID) {
		if ((*fs1)->pid < (*fs2)->pid)
			return -1;
		if ((*fs1)->pid > (*fs2)->pid)
			return 1;
		/* Fall through if pids equal */
	}

	if ((*fs1)->total == (*fs2)->total)
		return strcmp((*fs1)->path, (*fs2)->path);

	if ((*fs1)->total > (*fs2)->total)
		return -1;
	else
		return 1;
}

/*
 *  file_stat_dump()
 *	dump file stats and free hash table for next iteration
 */
static void file_stat_dump(const double duration, const unsigned long top)
{
	file_stat_t **sorted;
	uint64_t i, j;
	char ts[32];

	if (!file_stats_size)
		return;

	sorted = calloc(file_stats_size, sizeof(file_stat_t *));
	if (sorted == NULL)
		pr_error("allocating file stats");

	for (j = 0, i = 0; i < TABLE_SIZE; i++) {
		file_stat_t *fs = file_stats[i];

		while (fs) {
			sorted[j++] = fs;
			fs = fs->next;
		}
		if (!(opt_flags & OPT_CUMULATIVE))
			file_stats[i] = NULL;
	}

	qsort(sorted, file_stats_size, sizeof(file_stat_t *), file_stat_cmp);

	if (opt_flags & OPT_TIMESTAMP) {
		struct tm tm;

		get_tm(&tm);
		snprintf(ts, sizeof(ts), " [%2.2d/%2.2d/%-2.2d %2.2d:%2.2d:%2.2d]\n",
			tm.tm_mday, tm.tm_mon + 1, (tm.tm_year+1900) % 100,
			tm.tm_hour, tm.tm_min, tm.tm_sec);
	} else {
		*ts = '\0';
	}
	printf(" Total   Open  Close   Read  Write  PID  Process         Pathname%s\n", ts);
	for (j = 0; j < file_stats_size; j++) {
		if (top && j <= top) {
			char buf[5][32];

			printf("%s %s %s %s %s %5d %-15.15s %s\n",
				count_to_str(sorted[j]->total / duration, buf[0], sizeof(buf[0])),
				count_to_str(sorted[j]->open / duration, buf[1], sizeof(buf[1])),
				count_to_str(sorted[j]->close / duration, buf[2], sizeof(buf[2])),
				count_to_str(sorted[j]->read/ duration, buf[3], sizeof(buf[3])),
				count_to_str(sorted[j]->write / duration, buf[4], sizeof(buf[4])),
				sorted[j]->pid,
				proc_info_get(sorted[j]->pid)->cmdline,
				(opt_flags & OPT_DIRNAME_STRIP) ?
					basename(sorted[j]->path) : sorted[j]->path);
		}
		if (!(opt_flags & OPT_CUMULATIVE)) {
			free(sorted[j]->path);
			free(sorted[j]);
		}
	}
	free(sorted);
	if (!(opt_flags & OPT_CUMULATIVE))
		file_stats_size = 0;

	printf("\n");
}

static int parse_pid_list(char *arg)
{
	char *str, *token, *saveptr = NULL;

	for (str = arg; (token = strtok_r(str, ",", &saveptr)) != NULL; str = NULL) {
		pid_t pid = 0;
		char *name = NULL;
		proc_info_t *p;

		/* Hit next option, which means no args given */
		if (token[0] == '-')
			break;

		if (isdigit(token[0])) {
			errno = 0;
			pid = strtol(token, NULL, 10);
			if (errno) {
				fprintf(stderr, "Invalid PID specified.\n");
				return -1;
			}
		} else {
			name = strdup(token);
			if (!name) {
				fprintf(stderr, "Out of memory allocating process name info.\n");
				return -1;
			}
			named_procs = true;
		}
		p = calloc(1, sizeof(proc_info_t));
		if (!p) {
			fprintf(stderr, "Out of memory allocating process info.\n");
			free(name);
			return -1;
		}
		p->pid = pid;
		p->cmdline = name;
		p->whence = 0.0;	/* Not used */
		p->next = proc_list;
		proc_list = p;
	}
	if (proc_list == NULL) {
		fprintf(stderr, "No valid process ID or names given.\n");
		return -1;
	}
	return 0;
}

/*
 *  show_usage()
 *	how to use
 */
void show_usage(void)
{
	printf("%s, version %s\n\n", app_name, VERSION);
	printf("Options are:\n"
		"  -c     cumulative totals over time\n"
		"  -d     strip directory off the filenames\n"
		"  -h     show this help\n"
		"  -p PID collect stats for just process with pid PID\n"
		"  -P     sort stats by process ID\n"
		"  -t N   show just the busiest N files\n"
		"  -T     show timestamp\n"
		"  -v     verbose mode, dump out all file activity\n");
}

int main(int argc, char **argv)
{
	unsigned long count = 0, top = -1;
	void *buffer;
	ssize_t len;
	int fan_fd, ret, i;
	float duration_secs = 1.0;
	bool forever = true;
	struct sigaction new_action;
	struct timeval tv1, tv2;

	for (;;) {
		int c = getopt(argc, argv, "hvdt:p:PcTs");
		if (c == -1)
			break;
		switch (c) {
		case 'c':
			opt_flags |= OPT_CUMULATIVE;
			break;
		case 'h':
			show_usage();
			exit(EXIT_SUCCESS);
		case 'v':
			opt_flags |= OPT_VERBOSE;
			break;
		case 'd':
			opt_flags |= OPT_DIRNAME_STRIP;
			break;
		case 's':
			opt_flags &= ~OPT_SCALE;
			break;
		case 't':
			errno = 0;
			top = strtol(optarg, NULL, 10);
			if (errno) {
				fprintf(stderr, "Invalid value for -t option.\n");
				exit(EXIT_FAILURE);
			}
			if (top < 1) {
				fprintf(stderr, "Value for -t option must be 1 or more.\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'T':
			opt_flags |= OPT_TIMESTAMP;
			break;
		case 'p':
			if (parse_pid_list(optarg) < 0) {
				fprintf(stderr, "Invalid value for -p option.\n");
				exit(EXIT_FAILURE);
			}
			opt_flags |= OPT_PID;
			break;
		case 'P':
			opt_flags |= OPT_SORT_BY_PID;
			break;
		default:
			show_usage();
			exit(EXIT_FAILURE);
		}
	}
	if (optind < argc) {
		duration_secs = atof(argv[optind++]);
		if (duration_secs < 0.5) {
			fprintf(stderr, "Duration must be 0.5 or more.\n");
			exit(EXIT_FAILURE);
		}
	}
	if (optind < argc) {
		forever = false;
		errno = 0;
		count = strtol(argv[optind++], NULL, 10);
		if (errno) {
			fprintf(stderr, "Invalid count value\n");
			exit(EXIT_FAILURE);
		}
		if (count < 1) {
			fprintf(stderr, "Count must be > 0\n");
			exit(EXIT_FAILURE);
		}
	}

	if ((getuid() != 0) || (geteuid() != 0)) {
		fprintf(stderr, "%s requires root privileges to run.\n", app_name);
		exit(EXIT_FAILURE);
	}

	memset(&new_action, 0, sizeof(new_action));
	for (i = 0; signals[i] != -1; i++) {
		new_action.sa_handler = handle_sig;
		sigemptyset(&new_action.sa_mask);
		new_action.sa_flags = 0;

		if (sigaction(signals[i], &new_action, NULL) < 0) {
			fprintf(stderr, "sigaction failed: errno=%d (%s)\n",
				errno, strerror(errno));
			exit(EXIT_FAILURE);
		}
	}

	my_pid = getpid();

	ret = posix_memalign(&buffer, BUFFER_SIZE, BUFFER_SIZE);
	if (ret != 0 || buffer == NULL) {
		fprintf(stderr,"cannot allocate 4K aligned buffer");
		exit(EXIT_FAILURE);
	}
	fan_fd = fnotify_event_init();
	if (fan_fd < 0) {
		fprintf(stderr, "cannot init fnotify");
		exit(EXIT_FAILURE);
	}

	while (!stop_fnotifystat && (forever || count--)) {
		double duration;
		if (gettimeofday(&tv1, NULL) < 0)
			pr_error("gettimeofday failed");

		for (;;) {
			fd_set rfds;
			int ret;
			double remaining;

			if (gettimeofday(&tv2, NULL) < 0)
				pr_error("gettimeofday failed");

			remaining = duration_secs + timeval_double(&tv1) - timeval_double(&tv2);
			if (remaining < 0.0)
				break;

			FD_ZERO(&rfds);
			FD_SET(fan_fd, &rfds);
			tv2.tv_sec = remaining;
			tv2.tv_usec = (remaining - (int)remaining) * 1000000.0;

			ret = select(fan_fd + 1, &rfds, NULL, NULL, &tv2);
			if (ret == -1) {
				if (errno == EINTR) {
					stop_fnotifystat = true;
					break;
				}
				pr_error("select failed");
			}
			if (ret == 0)
				break;
			if ((len = read(fan_fd, (void *)buffer, BUFFER_SIZE)) > 0) {
				const struct fanotify_event_metadata *metadata;
				metadata = (struct fanotify_event_metadata *)buffer;

				while (FAN_EVENT_OK(metadata, len)) {
					if (fnotify_event_add(metadata) < 0)
						break;
					metadata = FAN_EVENT_NEXT(metadata, len);
				}
			}
		}
		if (gettimeofday(&tv2, NULL) < 0)
			pr_error("gettimeofday failed");

		duration = timeval_double(&tv2) - timeval_double(&tv1);
		file_stat_dump(duration, top);
	}

	(void)close(fan_fd);
	free(buffer);
	proc_info_free();
	proc_list_free(&proc_list);

	exit(EXIT_SUCCESS);
}
