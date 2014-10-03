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
#include <errno.h>
#include <mntent.h>
#include <time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/fanotify.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>

#define APP_NAME		"fnotifystat"
#define TABLE_SIZE		(1997)
#define OPT_VERBOSE		(0x00000001)
#define OPT_DIRNAME_STRIP 	(0x00000002)
#define OPT_PID			(0x00000004)
#define OPT_SORT_BY_PID		(0x00000008)
#define OPT_CUMULATIVE		(0x00000010)

/* fnotify file activity stats */
typedef struct file_stat {
	char 		*path;	/* Pathname of file */
	pid_t		pid;	/* PID of process touching file */
	uint64_t	open;	/* Count of opens */
	uint64_t	close;	/* Count of closes */
	uint64_t	read;	/* Count of reads */
	uint64_t	write;	/* Count of writes */
	uint64_t	total;	/* Total count */
	struct file_stat *next;	/* Next item in hash list */
} file_stat_t;

/* process info */
typedef struct proc_info {
	char 		*cmdline; /* cmdline of process */
	pid_t		pid;	/* pid of process */
	struct proc_info *next;	/* Next item in hash list */
} proc_info_t;

static file_stat_t *file_stats[TABLE_SIZE];	/* hash table of file stats */
static proc_info_t *proc_infos[TABLE_SIZE];	/* hash table of proc infos */
static size_t file_stats_size;			/* number of items in hash table */
static unsigned int opt_flags;			/* option flags */
static volatile bool stop_fnotifystat = false;	/* true -> stop fnotifystat */
static pid_t opt_pid;				/* just watch files touched by a process */
static pid_t my_pid;				/* pid of this programme */

/*
 *  get_pid_cmdline
 * 	get process's /proc/pid/cmdline
 */
static char *get_pid_cmdline(const pid_t id)
{
	char buffer[4096];
	char *ptr;
	int fd;
	ssize_t ret;

	snprintf(buffer, sizeof(buffer), "/proc/%d/cmdline", id);

	if ((fd = open(buffer, O_RDONLY)) < 0)
		return strdup("<unknown>");

	if ((ret = read(fd, buffer, sizeof(buffer))) <= 0) {
		close(fd);
		return strdup("<unknown>");
	}
	close(fd);

	buffer[sizeof(buffer)-1] = '\0';
	for (ptr = buffer; *ptr && (ptr < buffer + ret); ptr++) {
		if (*ptr == ' ')
			*ptr = '\0';
	}

	return strdup(basename(buffer));
}

/*
 *  handle_sigint()
 *      catch SIGINT and flag a stop
 */
static void handle_sigint(int dummy)
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

static proc_info_t *proc_info_get(const pid_t pid)
{
	unsigned long h = pid % TABLE_SIZE;
	proc_info_t *pi = proc_infos[h];

	while (pi) {
		if (pi->pid == pid)
			return pi;
		pi = pi->next;
	}
	if ((pi = calloc(1, sizeof(*pi))) == NULL) {
		fprintf(stderr, "calloc: out of memory\n");
		exit(EXIT_FAILURE);
	}
	pi->pid = pid;
	if ((pi->cmdline = get_pid_cmdline(pid)) == NULL) {
		fprintf(stderr, "calloc: out of memory\n");
		exit(EXIT_FAILURE);
	}
	pi->next = proc_infos[h];
	proc_infos[h] = pi;

	return pi;
}

static void proc_info_free(void)
{
	int i;

	for (i = 0; i < TABLE_SIZE; i++) {
		proc_info_t *pi = proc_infos[i];

		while (pi) {
			proc_info_t *next = pi->next;

			free(pi->cmdline);
			free(pi);

			pi = next;
		}
		proc_infos[i] = NULL;
	}
}

/*
 *  file_stat_get()
 *	get file stats on a file touched by a given process by PID.
 *	existing stats are returned, new stats are allocated and returned.
 */
static file_stat_t *file_stat_get(const char *str, const pid_t pid)
{
	unsigned long h = hash_pjw(str, pid);
	file_stat_t *fs = file_stats[h];

	while (fs) {
		if (!strcmp(str, fs->path) && (pid == fs->pid))
			return fs;
		fs = fs->next;
	}
	if ((fs = calloc(1, sizeof(*fs))) == NULL) {
		fprintf(stderr, "calloc: out of memory\n");
		exit(EXIT_FAILURE);
	}
	if ((fs->path = strdup(str)) == NULL) {
		free(fs);
		fprintf(stderr, "calloc: out of memory\n");
		exit(EXIT_FAILURE);
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
	int fan_fd;
	int ret;
	FILE* mounts;
	struct mntent* mount;

	if ((fan_fd = fanotify_init (0, 0)) < 0) {
		fprintf(stderr, "Cannot initialize fanotify: %s.\n",
			strerror(errno));
		return -1;
	}

	ret = fanotify_mark(fan_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
		FAN_ACCESS| FAN_MODIFY | FAN_OPEN | FAN_CLOSE |
		FAN_ONDIR | FAN_EVENT_ON_CHILD, AT_FDCWD, "/");
	if (ret < 0) {
		fprintf(stderr, "Cannot add fanotify watch on /: %s.\n",
			strerror(errno));
	}

	if ((mounts = setmntent("/proc/self/mounts", "r")) == NULL) {
		fprintf(stderr, "Cannot get mount points.\n");
		return -1;
	}

	while ((mount = getmntent (mounts)) != NULL) {
		/*
		if (access (mount->mnt_fsname, F_OK) != 0)
			continue;
		*/

		ret = fanotify_mark(fan_fd, FAN_MARK_ADD | FAN_MARK_MOUNT,
			FAN_ACCESS| FAN_MODIFY | FAN_OPEN | FAN_CLOSE |
			FAN_ONDIR | FAN_EVENT_ON_CHILD, AT_FDCWD,
			mount->mnt_dir);
		if ((ret < 0) && (errno != ENOENT)) {
			continue;
		}
	}
	endmntent (mounts);

	/* Track /sys/power ops for wakealarm analysis */
	(void)fanotify_mark(fan_fd, FAN_MARK_ADD,
		FAN_ACCESS | FAN_MODIFY, AT_FDCWD,
		"/sys/power/wake_lock");
	(void)fanotify_mark(fan_fd, FAN_MARK_ADD,
		FAN_ACCESS | FAN_MODIFY, AT_FDCWD,
		"/sys/power/wake_unlock");

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
	int i = 0;

	if (mask & FAN_OPEN)
		modes[i++] = 'O';
	if (mask & (FAN_CLOSE_WRITE | FAN_CLOSE_NOWRITE))
		modes[i++] = 'C';
	if (mask & FAN_ACCESS)
		modes[i++] = 'R';
	if (mask & (FAN_MODIFY | FAN_CLOSE_WRITE))
		modes[i++] = 'W';
	modes[i] = '\0';

	return modes;
}

/*
 *  fnotify_event_add()
 *	add a new fnotify event
 */
static int fnotify_event_add(const struct fanotify_event_metadata *metadata)
{
	char 	*filename;
	time_t now;
	struct tm tm;
	file_stat_t *fs;

	if ((metadata->fd == FAN_NOFD) && (metadata->fd < 0))
		return 0;
	if (metadata->pid == my_pid)
		return 0;
	if ((opt_flags & OPT_PID) && (metadata->pid != opt_pid))
		return 0;

 	filename = fnotify_get_filename(-1, metadata->fd);
	if (filename == NULL) {
		fprintf(stderr, "Out of memory: allocating fnotify filename");
		close(metadata->fd);
		return -1;
	}

	time(&now);
	localtime_r(&now, &tm);

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
	close(metadata->fd);

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

	if (!file_stats_size)
		return;
	
	sorted = calloc(file_stats_size, sizeof(file_stat_t *));
	if (sorted == NULL) {
		fprintf(stderr, "calloc failed\n");	
		exit(EXIT_FAILURE);
	}

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

	printf(" Total   Open  Close   Read  Write  PID  Process         Pathname\n");
	for (j = 0; j < file_stats_size; j++) {
		if (top && j <= top) {
			printf("%6.2f %6.2f %6.2f %6.2f %6.2f %5d %-15.15s %s\n",
				(double)sorted[j]->total / duration,
				(double)sorted[j]->open / duration,
				(double)sorted[j]->close / duration,
				(double)sorted[j]->read / duration,
				(double)sorted[j]->write / duration,
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

/*
 *  show_usage()
 *	how to use
 */
void show_usage(void)
{
	printf("%s, version %s\n\n", APP_NAME, VERSION);
	printf("Options are:\n"
		"  -c     cumulative totals over time\n"
		"  -d     strip directory off the filenames\n"
		"  -h     show this help\n"
		"  -p PID collect stats for just process with pid PID\n"
		"  -P     sort stats by process ID\n"
		"  -t N   show just the busiest N files\n"
		"  -v     verbose mode, dump out all file activity\n");
}

int main(int argc, char **argv)
{
	int fan_fd;
	ssize_t len;
	int ret;
	void *buffer;
	struct timeval tv1, tv2;
	float duration_secs = 1.0;
	bool forever = true;
	unsigned long count = 0, top = -1;

	for (;;) {
		int c = getopt(argc, argv, "hvdt:p:Pc");
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
		case 'p':
			errno = 0;
			opt_pid = (pid_t)strtol(optarg, NULL, 10);
			if (errno) {
				fprintf(stderr, "Invalid value for -t option.\n");
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
		fprintf(stderr, "this program requires root privileges to run.\n");
		exit(EXIT_FAILURE);
	}

	my_pid = getpid();

	ret = posix_memalign(&buffer, 4096, 4096);
	if (ret != 0 || buffer == NULL) {
		fprintf(stderr,"cannot allocate 4K aligned buffer");
		exit(EXIT_FAILURE);
	}
	fan_fd = fnotify_event_init();
	if (fan_fd < 0) {
		fprintf(stderr,"cannot init fnotify");
		exit(EXIT_FAILURE);
	}

	signal(SIGINT, &handle_sigint);
	while (!stop_fnotifystat && (forever || count--)) {
		double duration;
		gettimeofday(&tv1, NULL);
		for (;;) {
			fd_set rfds;
			int ret;
			double remaining;
	
			gettimeofday(&tv2, NULL);
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
				fprintf(stderr, "select error: %d\n", errno);
				exit(EXIT_FAILURE);
			}
			if (ret == 0)
				break;
			if ((len = read(fan_fd, (void *)buffer, 4096)) > 0) {
				const struct fanotify_event_metadata *metadata;
				metadata = (struct fanotify_event_metadata *)buffer;

				while (FAN_EVENT_OK(metadata, len)) {
					if (fnotify_event_add(metadata) < 0)
						break;
					metadata = FAN_EVENT_NEXT(metadata, len);
				}
			}
		}
		gettimeofday(&tv2, NULL);
		duration = timeval_double(&tv2) - timeval_double(&tv1);
		file_stat_dump(duration, top);
	}

	close(fan_fd);
	free(buffer);
	proc_info_free();

	exit(EXIT_SUCCESS);
}
