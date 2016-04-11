#include <assert.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>


/*
 * connect to netlink
 * returns netlink socket, or -1 on error
 */
static int nl_connect()
{
    int rc;
    int nl_sock;
    struct sockaddr_nl sa_nl;

    nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
    if (nl_sock == -1) {
        perror("socket");
        return -1;
    }

    sa_nl.nl_family = AF_NETLINK;
    sa_nl.nl_groups = CN_IDX_PROC;
    sa_nl.nl_pid = getpid();

    rc = bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl));
    if (rc == -1) {
        perror("bind");
        close(nl_sock);
        return -1;
    }

    return nl_sock;
}

/*
 * subscribe on proc events (process notifications)
 */
static int set_proc_ev_listen(int nl_sock, bool enable)
{
    int rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg;
            enum proc_cn_mcast_op cn_mcast;
        };
    } nlcn_msg;

    memset(&nlcn_msg, 0, sizeof(nlcn_msg));
    nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
    nlcn_msg.nl_hdr.nlmsg_pid = getpid();
    nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

    nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
    nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
    nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);

    nlcn_msg.cn_mcast = enable ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

    rc = send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
    if (rc == -1) {
        perror("netlink send");
        return -1;
    }

    return 0;
}

// FIXME we need to depend on log_10(PID_MAX) in /proc/sys/kernel/pid_max
pid_t get_pid_max() {
    return 32768;
}

// Return the length of a string necessary
size_t max_pid_length_base_10() {
    pid_t max_pid = get_pid_max();
    assert(max_pid > 0);
    return floor(log10(abs(max_pid))) + 1;
}

// Slow af
bool process_is_in_docker_ns(int argc, const char **docker_ids,
        pid_t child_pid) {
    int i;
    size_t max_pid_length = max_pid_length_base_10();
    // Build the path for the /proc/PID/cgroup directory
    char buff[13+max_pid_length+1];
    snprintf(buff, PATH_MAX, "/proc/%d/cgroup",
             child_pid);

    // Open the file and get its size
    FILE *proc_file = fopen(buff, "r");
    if (proc_file == NULL) {
        fprintf(stderr,
                "The entry under '%s' for the process %d isn't available\n",
                buff, child_pid);
        perror("Error opening file");
        return false;
    }
    // Create a buffer and slurp the whole file into it. The file probably
    // isn't too big.
    size_t slurp_size = 1024;
    char *proc_cgroup = malloc(slurp_size);
    while (true) {
        // Sluuuurp
        fread(proc_cgroup, slurp_size, 1, proc_file);
        // If there was an error, clean up
        if (ferror(proc_file)) {
            perror("Failed to read /proc/PID/cgroup file");
            fclose(proc_file);
            free(proc_cgroup);
            return false;
        }
        // If we're not at the end of the file try again, otherwise break.
        if (!feof(proc_file)) {
            slurp_size += 1024;
            proc_cgroup = realloc(proc_cgroup, slurp_size);
        } else {
            break;
        }
    }

    fclose(proc_file);

    // Iterate through the given ids and check to see if they're in the file.
    for (i = 0; i < argc; ++i) {
        fprintf(stderr, "cross");
        // If we find one...
        if (strstr(proc_cgroup, docker_ids[i]) != NULL) {
            // Clean up and return true
            free(proc_cgroup);
            return true;
        }
    }
    // If we didn't find any, clean up and return false
    free(proc_cgroup);
    return false;
}

/*
 * handle a single process event
 */
static volatile bool need_exit = false;
static int handle_proc_ev(int nl_sock, int argc, const char **docker_ids)
{
    int rc;
    struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
        struct nlmsghdr nl_hdr;
        struct __attribute__ ((__packed__)) {
            struct cn_msg cn_msg;
            struct proc_event proc_ev;
        };
    } nlcn_msg;

    while (!need_exit) {
        rc = recv(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
        if (rc == 0) {
            /* shutdown? */
            return 0;
        } else if (rc == -1) {
            if (errno == EINTR) continue;
            perror("netlink recv");
            return -1;
        }

        switch (nlcn_msg.proc_ev.what) {
            case PROC_EVENT_NONE:
                printf("set mcast listen ok\n");
                break;
            case PROC_EVENT_FORK:
                if (!process_is_in_docker_ns(argc, docker_ids,
                            nlcn_msg.proc_ev.event_data.fork.parent_pid)) {
                    continue;
                }
                printf("fork: parent tid=%d pid=%d -> child tid=%d pid=%d\n",
                        nlcn_msg.proc_ev.event_data.fork.parent_pid,
                        nlcn_msg.proc_ev.event_data.fork.parent_tgid,
                        nlcn_msg.proc_ev.event_data.fork.child_pid,
                        nlcn_msg.proc_ev.event_data.fork.child_tgid);
                break;
            case PROC_EVENT_EXEC:
                if (!process_is_in_docker_ns(argc, docker_ids,
                            nlcn_msg.proc_ev.event_data.exec.process_pid)) {
                    continue;
                }

                printf("exec: tid=%d pid=%d\n",
                        nlcn_msg.proc_ev.event_data.exec.process_pid,
                        nlcn_msg.proc_ev.event_data.exec.process_tgid);
                break;
            case PROC_EVENT_UID:
                if (!process_is_in_docker_ns(argc, docker_ids,
                            nlcn_msg.proc_ev.event_data.id.process_pid)) {
                    continue;
                }
                printf("uid change: tid=%d pid=%d from %d to %d\n",
                        nlcn_msg.proc_ev.event_data.id.process_pid,
                        nlcn_msg.proc_ev.event_data.id.process_tgid,
                        nlcn_msg.proc_ev.event_data.id.r.ruid,
                        nlcn_msg.proc_ev.event_data.id.e.euid);
                break;
            case PROC_EVENT_GID:
                if (!process_is_in_docker_ns(argc, docker_ids,
                            nlcn_msg.proc_ev.event_data.id.process_pid)) {
                    continue;
                }
                printf("gid change: tid=%d pid=%d from %d to %d\n",
                        nlcn_msg.proc_ev.event_data.id.process_pid,
                        nlcn_msg.proc_ev.event_data.id.process_tgid,
                        nlcn_msg.proc_ev.event_data.id.r.rgid,
                        nlcn_msg.proc_ev.event_data.id.e.egid);
                break;
            case PROC_EVENT_EXIT:
                if (!process_is_in_docker_ns(argc, docker_ids,
                            nlcn_msg.proc_ev.event_data.exit.process_pid)) {
                    continue;
                }
                printf("exit: tid=%d pid=%d exit_code=%d\n",
                        nlcn_msg.proc_ev.event_data.exit.process_pid,
                        nlcn_msg.proc_ev.event_data.exit.process_tgid,
                        nlcn_msg.proc_ev.event_data.exit.exit_code);
                break;
            default:
                printf("unhandled proc event\n");
                break;
        }
    }

    return 0;
}

static void on_sigint(int unused)
{
    need_exit = true;
}

int main(int argc, const char *argv[])
{
    // If we didn't get any docker ids as arguments, exit
    if (argc <= 1) {
        fprintf(stderr, "Usage: %s dockerids...\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    int nl_sock;
    int rc = EXIT_SUCCESS;

    signal(SIGINT, &on_sigint);
    siginterrupt(SIGINT, true);

    nl_sock = nl_connect();
    if (nl_sock == -1)
        exit(EXIT_FAILURE);

    rc = set_proc_ev_listen(nl_sock, true);
    if (rc == -1) {
        rc = EXIT_FAILURE;
        goto out;
    }

    // Send everything except the first argument
    rc = handle_proc_ev(nl_sock, argc-1, &argv[1]);
    if (rc == -1) {
        rc = EXIT_FAILURE;
        goto out;
    }

	set_proc_ev_listen(nl_sock, false);

out:
    close(nl_sock);
    exit(rc);
}

