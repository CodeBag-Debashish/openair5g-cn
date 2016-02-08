/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under 
 * the Apache License, Version 2.0  (the "License"); you may not use this file
 * except in compliance with the License.  
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "intertask_interface.h"
#include "mme_config.h"
#include "nas_defs.h"
#include "msc.h"

#include "nas_network.h"
#include "nas_proc.h"
#include "emm_main.h"
#include "log.h"
#include "nas_timer.h"


static void                            *
nas_intertask_interface (
  void *args_p)
{
  itti_mark_task_ready (TASK_NAS_MME);
  LOG_START_USE ();
  MSC_START_USE ();

  while (1) {
    MessageDef                             *received_message_p = NULL;

    itti_receive_msg (TASK_NAS_MME, &received_message_p);

    switch (ITTI_MSG_ID (received_message_p)) {
    case NAS_CONNECTION_ESTABLISHMENT_IND:{
        nas_establish_ind_t                    *nas_est_ind_p;

        nas_est_ind_p = &received_message_p->ittiMsg.nas_conn_est_ind.nas;
        nas_proc_establish_ind (nas_est_ind_p->UEid, nas_est_ind_p->plmn, nas_est_ind_p->tac, nas_est_ind_p->initialNasMsg.data, nas_est_ind_p->initialNasMsg.length);
      }
      break;

    case NAS_UPLINK_DATA_IND:{
        nas_proc_ul_transfer_ind (NAS_UL_DATA_IND (received_message_p).UEid, NAS_UL_DATA_IND (received_message_p).nasMsg.data, NAS_UL_DATA_IND (received_message_p).nasMsg.length);
      }
      break;

    case NAS_DOWNLINK_DATA_CNF:{
        nas_proc_dl_transfer_cnf (NAS_DL_DATA_CNF (received_message_p).UEid);
      }
      break;

    case NAS_DOWNLINK_DATA_REJ:{
        nas_proc_dl_transfer_rej (NAS_DL_DATA_REJ (received_message_p).UEid);
      }
      break;

    case NAS_AUTHENTICATION_PARAM_RSP:{
        nas_proc_auth_param_res (&NAS_AUTHENTICATION_PARAM_RSP (received_message_p));
      }
      break;

    case NAS_AUTHENTICATION_PARAM_FAIL:{
        nas_proc_auth_param_fail (&NAS_AUTHENTICATION_PARAM_FAIL (received_message_p));
      }
      break;

    case NAS_PDN_CONNECTIVITY_RSP:{
        nas_proc_pdn_connectivity_res (&NAS_PDN_CONNECTIVITY_RSP (received_message_p));
      }
      break;

    case NAS_PDN_CONNECTIVITY_FAIL:{
        nas_proc_pdn_connectivity_fail (&NAS_PDN_CONNECTIVITY_FAIL (received_message_p));
      }
      break;

    case TIMER_HAS_EXPIRED:{
        /*
         * Call the NAS timer api
         */
        nas_timer_handle_signal_expiry (TIMER_HAS_EXPIRED (received_message_p).timer_id, TIMER_HAS_EXPIRED (received_message_p).arg);
      }
      break;

    case S1AP_ENB_DEREGISTERED_IND:{
      /*
        int                                     i;

        for (i = 0; i < S1AP_ENB_DEREGISTERED_IND (received_message_p).nb_ue_to_deregister; i++) {
          nas_proc_deregister_ue (S1AP_ENB_DEREGISTERED_IND (received_message_p).mme_ue_s1ap_id[i]);
        }
*/
      }
      break;

    case S1AP_DEREGISTER_UE_REQ:{
        nas_proc_deregister_ue (S1AP_DEREGISTER_UE_REQ (received_message_p).mme_ue_s1ap_id);
      }
      break;

    case TERMINATE_MESSAGE:{
        itti_exit_task ();
      }
      break;

    case MESSAGE_TEST:
      LOG_DEBUG (LOG_NAS, "Received MESSAGE_TEST\n");
      break;

    default:{
        LOG_DEBUG (LOG_NAS, "Unkwnon message ID %d:%s from %s\n", ITTI_MSG_ID (received_message_p), ITTI_MSG_NAME (received_message_p), ITTI_MSG_ORIGIN_NAME (received_message_p));
      }
      break;
    }

    itti_free (ITTI_MSG_ORIGIN_ID (received_message_p), received_message_p);
    received_message_p = NULL;
  }

  return NULL;
}

int
nas_init (
  mme_config_t * mme_config_p)
{
  LOG_DEBUG (LOG_NAS, "Initializing NAS task interface\n");
  nas_network_initialize (mme_config_p);

  if (itti_create_task (TASK_NAS_MME, &nas_intertask_interface, NULL) < 0) {
    LOG_ERROR (LOG_NAS, "Create task failed");
    LOG_DEBUG (LOG_NAS, "Initializing NAS task interface: FAILED\n");
    return -1;
  }

  LOG_DEBUG (LOG_NAS, "Initializing NAS task interface: DONE\n");
  return 0;
}
