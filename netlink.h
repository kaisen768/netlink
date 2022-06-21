#ifndef _NETLINK_H_
#define _NETLINK_H_

#include "uv.h"
#include <stdbool.h>

typedef struct nl_module_s {
    uv_loop_t *loop;

    /* wired connect status */
    bool connected;

    /* netlink udp handle */
    uv_udp_t _nl_udp;

    /**
     * @brief netlink module start working
     * @param nl netlink structure
     */
    void (*start)(struct nl_module_s *nl);

    /**
     * @brief netlink module stop work
     * @param nl netlink structure
     */
    void (*stop)(struct nl_module_s *nl);
} nl_module_t;

/**
 * @brief netlink module initalizal
 * 
 * @param nl netlink structure
 * @param loop uv loop context
 * @return 0 if success, else return -1 if an error occurred
 */
int nl_module_init(nl_module_t *nl, uv_loop_t *loop);

#endif
