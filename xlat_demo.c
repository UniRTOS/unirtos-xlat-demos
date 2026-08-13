/*****************************************************************/ /**
* @file xlat_demo.c
* @brief
* @author elmer.tang@quectel.com
* @date 2025-12-12
*
* @copyright Copyright (c) 2023 Quectel Wireless Solution, Co., Ltd.
* All Rights Reserved. Quectel Wireless Solution Proprietary and Confidential.
*
* @par EDIT HISTORY FOR MODULE
* <table>
* <tr><th>Date <th>Version <th>Author <th>Description
* <tr><td>2025-12-12 <td>1.0 <td>elmer.tang <td> Init
* </table>
**********************************************************************/
#include "qosa_def.h"
#include "qosa_log.h"
#include "qosa_sim.h"
#include "qosa_dev1.h"
#include "qosa_network.h"
#include "qosa_clat.h"
#include "qcm_ping_app.h"
#include "unirtos_app_init_registry.h"

#define QOS_LOG_TAG LOG_TAG_DEMO

#define QOSA_CLAT_DEFAULT_PROFILE_ID 1
// PING server address
#define UNIR_PING_DEMO_SERVER          "8.8.8.8"

// PING configuration parameters
#define UNIR_PING_DEMO_SIMID           0
#define UNIR_PING_DEMO_PDPID           1

/** xlat demo task handle */
qosa_task_t g_xlat_task = QOSA_NULL;

static qosa_msgq_t g_ping_demo_msg = QOSA_NULL;

/**
 * @brief Ping callback response data structure
 *
 * This structure is used to store ping operation response information,
 * including event type, error code, response data, and user-defined parameters
 */
typedef struct
{
    qcm_ping_event_type event_id;   /**< ping event type */
    qcm_ping_error_e    evt_code;   /**< ping event error code */
    qcm_ping_resp_t     resp_ptr;   /**< ping response data pointer */
    void               *user_param; /**< user-defined parameter pointer */
} ping_demo_resp_t;

/**
 * @brief Ping result callback function
 *
 * This function handles ping operation result callbacks, encapsulates ping response data,
 * and sends it through the message queue
 *
 * @param event_id ping event type
 * @param evt_code ping error code
 * @param resp_ptr ping response data pointer
 * @param user_param user-defined parameter pointer
 */
static void unir_ping_result_cb(qcm_ping_event_type event_id, qcm_ping_error_e evt_code, qcm_ping_resp_t *resp_ptr, void *user_param)
{
    ping_demo_resp_t ping_rsp = {0};

    ping_rsp.event_id = event_id;
    ping_rsp.evt_code = evt_code;
    ping_rsp.user_param = user_param;

    /* Copy corresponding response data based on event type */
    if (ping_rsp.event_id == QCM_PING_STATS)
    {
        qosa_memcpy(&ping_rsp.resp_ptr.type.status, &resp_ptr->type.status, sizeof(qcm_ping_stats_type));
    }
    else
    {
        qosa_memcpy(&ping_rsp.resp_ptr.type.summary, &resp_ptr->type.summary, sizeof(qcm_ping_summary_type));
    }

    /* Send message */
    qosa_msgq_release(g_ping_demo_msg, sizeof(ping_demo_resp_t), (qosa_uint8_t *)&ping_rsp, QOSA_NO_WAIT);
}

/**
 * @brief Process PING operation result callback function, parse and record PING statistics or summary information based on event type.
 *
 * This function handles response events from the PING module, including PING status updates and final summary information.
 * Distinguishes between single PING result (QCM_PING_STATS) and final summary (QCM_PING_SUMMARY) based on event ID,
 * formats the relevant information and outputs it through logs. Marks processing as complete when summary event is received.
 *
 * @param[in] ping_ptr Pointer to PING response data structure, containing event ID, event code and response content
 * @return Flag indicating whether processing is complete, QOSA_TRUE means processing ended, QOSA_FALSE means not ended
 */
static qosa_bool_t unir_ping_result_handler(ping_demo_resp_t *ping_ptr)
{
    char         resp_buf[256] = {0};            // Buffer for storing formatted response string
    qosa_int32_t resp_len = 0;                   // Response string length (currently not actually used)
    qosa_int32_t evt_code = ping_ptr->evt_code;  // Extract event code
    qosa_bool_t  finish = QOSA_FALSE;            // Completion flag, default to not completed

    // Print event ID and event code for debugging and tracking
    QLOGD("event_id=%d,%x", ping_ptr->event_id, evt_code);

    // Process based on different event IDs
    switch (ping_ptr->event_id)
    {
        // Handle single PING statistics information event
        case QCM_PING_STATS: {
            // Only process when event code indicates success
            if (evt_code == QCM_PING_OK)
            {
                // Get PING status information structure pointer
                qcm_ping_stats_type *stats = &ping_ptr->resp_ptr.type.status;
                if (stats != QOSA_NULL)
                {
                    // Format PING status information
                    resp_len += qosa_snprintf(resp_buf, 256, "\"%s\",%ld,%ld,%ld", stats->resolved_ip_addr, stats->ping_size, stats->ping_rtt, stats->ping_ttl);
                    // Print PING status information
                    QLOGV("PING: [%s]", resp_buf);
                }
            }
        }
        break;
        // Handle PING summary information event at completion
        case QCM_PING_SUMMARY: {
            // Only process when event code indicates success
            if (evt_code == QCM_PING_OK)
            {
                // Get PING summary information structure pointer
                qcm_ping_summary_type *summary = &ping_ptr->resp_ptr.type.summary;
                if (summary != QOSA_NULL)
                {
                    // Format PING summary information
                    resp_len += qosa_snprintf(
                        resp_buf,
                        256,
                        "%ld,%ld,%ld,%ld,%ld,%ld",
                        summary->num_pkts_sent,
                        summary->num_pkts_recvd,
                        summary->num_pkts_lost,
                        summary->min_rtt,
                        summary->max_rtt,
                        summary->avg_rtt
                    );
                    // Print PING completion summary information
                    QLOGD("PING_END: [%s]", resp_buf);
                }
            }
            // Mark processing as complete
            finish = QOSA_TRUE;
        }
        break;
        default:
            break;
    }
    // Return completion flag
    return finish;
}

 /**
 * @brief xlat demo task main function
 *
 * This function serves as the main entry of the xlat demo task, implementing the following functions:
 * - Get and print the current CLAT switch status
 * - Enable CLAT function (set clat_switch to 1)
 * - Start a PING test to the IPv4 server (8.8.8.8) over the CLAT-enabled datacall
 * - Wait for PING completion and print the result statistics
 * - Get CLAT configuration again and print all CLAT parameters (switch, DNS64 mode, destination prefix, FQDN, TOS, etc.)
 *
 * @param[in] arg
 *          - Task parameter
 */
static void xlat_demo_task(void *arg)
{
    int                ret = 0;
    char               ip_str[64] = {0};
    qosa_bool_t        xlat_state = 0;
    qosa_bool_t        clat_status = 1;
    qosa_clat_config_t cfg = {0};

    qcm_ping_config_type ping_options = {0};
    ping_demo_resp_t     rsp = {0};
    qosa_bool_t          is_finish = QOSA_FALSE;

    QOSA_UNUSED(arg);

    /* xlat initialization has been completed at the platform level, so no additional initialization is required */ 
    QLOGI("XLAT DEMO !!!");

    qosa_task_sleep_sec(3);

    /* Get the CLAT status */
    ret = qosa_clat_get_cfg(QOSA_CLAT_DEFAULT_PROFILE_ID, &cfg);
    xlat_state = cfg.clat_switch;
    if (ret != QOSA_CLAT_OK)
    {
        QLOGI("qosa_clat_get_cfg fail");
    }
	QLOGI("xlat_state = %d",xlat_state);

    /* Set the CLAT status */
    cfg.clat_switch = clat_status;
    qosa_clat_set_cfg(QOSA_CLAT_DEFAULT_PROFILE_ID, &cfg);

	qosa_task_sleep_sec(3);

    /* Start the ping service. Refer to the ping demos. */
    // Assign values for ping operation data size, count, timeout; use default values if no parameters
    ping_options.num_data_bytes = 64;
    ping_options.num_pings = 4;
    ping_options.ping_response_time_out = 4;
    ping_options.ttl = 255;

    // Start ping operation, specifying PDPID, SIM card ID, server address and other parameters
    ret = qcm_ping_start(UNIR_PING_DEMO_PDPID, UNIR_PING_DEMO_SIMID, UNIR_PING_DEMO_SERVER, &ping_options, unir_ping_result_cb, QOSA_NULL);
    if (ret != QCM_PING_OK)
    {
        QLOGE("ping start err =%x", ret);
        qosa_msgq_delete(g_ping_demo_msg);
        return;
    }

    // Loop waiting for and processing ping result messages
    while (1)
    {
        // Wait for synchronous result message and process it
        qosa_msgq_wait(g_ping_demo_msg, (qosa_uint8_t *)&rsp, sizeof(ping_demo_resp_t), QOSA_WAIT_FOREVER);
        is_finish = unir_ping_result_handler(&rsp);
        if (is_finish)
        {
            // ping processing ended
            break;
        }
    }
    qosa_msgq_delete(g_ping_demo_msg);
    QLOGV("PING END");

    /* Get the CLAT status End*/
    ret = qosa_clat_get_cfg(QOSA_CLAT_DEFAULT_PROFILE_ID, &cfg);

    qosa_ip_addr_inet_ntop(cfg.dst_prefix.ip_vsn, &cfg.dst_prefix.addr.ipv6_addr, ip_str, sizeof(ip_str));
    xlat_state = cfg.clat_switch;
	QLOGI("clat ,%d,%d,\"%s\",%d,\"%s\",%d,%d,%d,%d,%d,%d",
           xlat_state,
           cfg.dns64_mode,
           ip_str,
           cfg.dest_prefix_len,
           cfg.fqdn,
           cfg.ul_ignore_v4_tos,
           cfg.dl_v4_tos,
           cfg.no_df,
           cfg.udp_zero_cksum,
           cfg.icmpv4_err_mode,
           cfg.icmpv6_err_mode);

	QLOGI("end");
}

/**
 * @brief xlat demo initialization function
 *
 * This function is used to initialize xlat demo functionality, create xlat demo task
 *
 */
void unir_xlat_demo_init(void)
{
    int err = 0;

    if (g_ping_demo_msg == QOSA_NULL)
    {
        QLOGD("g_ping_demo_msg create");
        qosa_msgq_create(&g_ping_demo_msg, sizeof(ping_demo_resp_t), 10);
    }
    // Create xlat demo task
    err = qosa_task_create(&g_xlat_task, 1024 * 4, QOSA_PRIORITY_NORMAL, "QXLATDEMO", xlat_demo_task, QOSA_NULL);
    if (err != QOSA_OK)
    {
        QLOGD("xlat demo task create error");
        return;
    }
}
UNIRTOS_APP_EXPORT(328, "xlat_demo", unir_xlat_demo_init);
