/* 
**  mod_detect_overload.c -- Apache module detect_overload 
*
*/ 

#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "ap_config.h"
#include "scoreboard.h"
#include "ap_mpm.h"
#include "http_log.h"
#include "http_config.h"
#include "http_request.h"

int server_limit, thread_limit;
static int percentage_limit = 90;

module AP_MODULE_DECLARE_DATA detect_overload_module;

typedef struct {
    int overload_detection_enabled;
} detect_overload_config;



static void detect_overload_register_hooks(apr_pool_t *p);

static void *create_detect_overload_server_config(apr_pool_t *p, server_rec *s)
{
    detect_overload_config *conf = apr_pcalloc(p, sizeof *conf);

    conf->overload_detection_enabled = 0;

    return conf;
}

static const char *detect_overload_on(cmd_parms *cmd, void *dummy, int arg)
{
    detect_overload_config *conf = ap_get_module_config(cmd->server->module_config,
                                                &detect_overload_module);
    conf->overload_detection_enabled = arg;

    return NULL;
}

static const command_rec detect_overload_cmds[] =
{
    AP_INIT_FLAG("EnableOverloadDetection", detect_overload_on, NULL, RSRC_CONF,
                 "Activate overload detection"),
//   AP_INIT_FLAG("BusyPercentageLimit", detect_overload_on, NULL, RSRC_CONF,
//                 "Limit of Busy/Total workers in percent"),
    { NULL }
};

static int detect_overload_init(apr_pool_t *p, apr_pool_t *plog, apr_pool_t *ptemp,
                       server_rec *s)
{
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_THREADS, &thread_limit);
    ap_mpm_query(AP_MPMQ_HARD_LIMIT_DAEMONS, &server_limit);
    return OK;
}

/*
 * Calculates the percentage
 *
 * Inspired by http://svn.apache.org/repos/asf/httpd/httpd/branches/2.4.x/modules/generators/mod_status.c
 */

static int calculate_overload_handler(request_rec *r){

   /*
    detect_overload_config *conf = ap_get_module_config(r->base_server->module_config,
                                               &detect_overload_module);
    if (!conf->overload_detection_enabled) {
       return OK;
    }
    */

    apr_table_t *env = r->subprocess_env;

    apr_time_t nowtime;
    int j, i, res;
    int ready;
    int busy;
    unsigned long count;
    worker_score *ws_record;
    process_score *ps_record;
    char *stat_buffer;
    ap_generation_t mpm_generation, worker_generation;

    ready = 0;
    busy = 0;
    count = 0;

    ap_mpm_query(AP_MPMQ_GENERATION, &mpm_generation);

    if (!ap_exists_scoreboard_image()) {
        ap_log_rerror(APLOG_MARK, APLOG_ERR, 0, r,
                      "Server status unavailable in inetd mode");
        return HTTP_INTERNAL_SERVER_ERROR;
    }

    r->allowed = (AP_METHOD_BIT << M_GET);
    if (r->method_number != M_GET)
        return DECLINED;

    for (i = 0; i < server_limit; ++i) {
       
        ps_record = ap_get_scoreboard_process(i);
        for (j = 0; j < thread_limit; ++j) {
            int indx = (i * thread_limit) + j;

            ws_record = ap_get_scoreboard_worker_from_indexes(i, j);

            res = ws_record->status;

            if (!ps_record->quiescing
                && ps_record->pid) {
                if (res == SERVER_READY
                    && ps_record->generation == mpm_generation)
                    ready++;
                else if (res != SERVER_DEAD &&
                         res != SERVER_STARTING &&
                         res != SERVER_IDLE_KILL)
                    busy++;
            }
         }
    }


    int total = busy+ready;
    double in_use_percent = ((double)busy/(double)total)*100;

//    apr_table_setn(env, "OVERLOAD_PERCENTAGE", (char *) in_use_percent);

    if (in_use_percent > percentage_limit){
      apr_table_setn(env, "OVERLOAD", "yes");
    }else{
      apr_table_setn(env, "OVERLOAD", "no");
    }

    return OK;
}


/* The sample content handler */
static int detect_overload_status_handler(request_rec *r)
{
    if (strcmp(r->handler, "detect_overload")) {
        return DECLINED;
    }
    r->content_type = "text/html";      

    apr_table_t *env = r->subprocess_env;


    if (!r->header_only){
        ap_rputs("The status page from mod_detect_overload.c\n", r);
    }

    ap_rprintf(r, "TODO");

    return OK;
}

static void detect_overload_register_hooks(apr_pool_t *p)
{
    ap_hook_handler(detect_overload_status_handler, NULL, NULL, APR_HOOK_MIDDLE);
    ap_hook_post_config(detect_overload_init, NULL, NULL, APR_HOOK_MIDDLE);

    ap_hook_fixups(calculate_overload_handler, NULL, NULL, APR_HOOK_MIDDLE);
}

module AP_MODULE_DECLARE_DATA detect_overload_module = {
    STANDARD20_MODULE_STUFF,
    NULL,                       /* Konfiguration pro Verzeichnis */
    NULL,                       /* Verknüpfung (Merger) der Verzeichnis-Konfigurationen */
    create_detect_overload_server_config,  /* Konfiguration pro Server */
    NULL,                       /* Verknüpfung (Merger) der Server-Konfigurationen */
    detect_overload_cmds,                  /* Befehlstabelle (Direktiven) */
    detect_overload_register_hooks              /* Registrierung der Hooks */
};


