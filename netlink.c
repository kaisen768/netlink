#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>

#include "netlink.h"

#define NET_OPERSTATE_PATH    "/sys/class/net/eth0/operstate"

typedef enum nl_operstate_e {
    OPERSTATE_UP        = 0,
    OPERSTATE_DOWN      = 1,
    OPERSTATE_NOEXIST   = 2,
    OPERSTATE_UNKNOWN   = 3,
} nl_operstate_t;

static inline nl_operstate_t read_operstate(void)
{
    char buf[12] = {0};

    int operstate_fd = open(NET_OPERSTATE_PATH, O_RDONLY);
    if (operstate_fd < 0) {
        fprintf(stderr, "operstate file[%s] open fail, err : %d\n", \
            NET_OPERSTATE_PATH, operstate_fd);
        return OPERSTATE_NOEXIST;
    }

    if (read(operstate_fd, buf, sizeof(buf)) <= 0) {
        fprintf(stderr, "operstate file[%s] read fail\n", NET_OPERSTATE_PATH);
        return OPERSTATE_NOEXIST;
    }

    close(operstate_fd);

    // fprintf(stderr, "operstate : %s\n", buf);

    if (strstr(buf, "up"))
        return OPERSTATE_UP;
    else if (strstr(buf, "down"))
        return OPERSTATE_DOWN;
    else
        return OPERSTATE_UNKNOWN;
}

static void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

static void on_receive_callback(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
    struct nlmsghdr *nlmsg_h;
    struct ifinfomsg *ifinfo;

    nl_module_t *nl = handle->data;

    if (nread < 0) {
        fprintf(stderr, "failed to receive from udp server, err: %s\n", uv_strerror(nread));
        free(buf->base);
        return;
    } else if (nread == 0) {
        goto release;
    }

    for (nlmsg_h = (struct nlmsghdr *)buf->base; NLMSG_OK(nlmsg_h, nread); nlmsg_h = NLMSG_NEXT(nlmsg_h, nread)) {
        if (nlmsg_h->nlmsg_type == NLMSG_DONE)
            return;

        if (nlmsg_h->nlmsg_type == NLMSG_ERROR) {
            printf ("read_netlink: Message is an error - decode TBD\n");
            return;
        }

        if (nlmsg_h->nlmsg_type == RTM_NEWLINK) {
            ifinfo = NLMSG_DATA (nlmsg_h);
            fprintf (stderr, "NETLINK::%s\n", (ifinfo->ifi_flags & IFF_RUNNING) ? "Up" : "Down");
            if (ifinfo->ifi_flags & IFF_RUNNING)
                nl->connected = true;
            else
                nl->connected = false;
        }
    }

release:
    if (!(flags & UV_UDP_MMSG_CHUNK))
        free(buf->base);
}

static void nl_module_start(nl_module_t *nl)
{
    int nl_socket;
    struct sockaddr_nl addr;

    if (!nl || !nl->loop) {
        return;
    }

    /* Get current operstate */
    nl_operstate_t operstate = read_operstate();
    nl->connected = (operstate == OPERSTATE_UP) ? true : false;

    /* Register netlink notic */
    addr.nl_family = AF_NETLINK;
    addr.nl_pid = getpid();
    addr.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV6_IFADDR;

    nl_socket = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    if (nl_socket < 0) {
        fprintf(stderr, "socket create fail !!!\n");
        return;
    }

    nl->_nl_udp.data = nl;

    uv_udp_init_ex(nl->loop, &nl->_nl_udp, UV_UDP_RECVMMSG);
    if (bind(nl_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        fprintf(stderr, "Socket bind fail\n");
        return;
    }

    uv_udp_open(&nl->_nl_udp, nl_socket);
    uv_udp_recv_start(&nl->_nl_udp, alloc_buffer, on_receive_callback);

    return;
}

static void nl_module_exit(nl_module_t *nl)
{
    if (!nl)
        return;

    uv_udp_recv_stop(&nl->_nl_udp);
    uv_close((uv_handle_t*)&nl->_nl_udp, NULL);

    nl->connected = false;

    return;
}

int nl_module_init(nl_module_t *nl, uv_loop_t *loop)
{
    if (!nl || !loop) {
        return -1;
    }

    nl->loop = loop;
    nl->connected = false;
    nl->start = nl_module_start;
    nl->stop = nl_module_exit;

    return 0;
}
