/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
// IMPORTANT: DONOT CHANGE ANYTHING IN THIS FILE
#include "vsf.h"
#include "usrapp.h"

#if defined(APPCFG_NRT_QUEUE_LEN) && (APPCFG_NRT_QUEUE_LEN > 0)
#	define APPCFG_MAINQ_EN
#endif
#if defined(APPCFG_SRT0_QUEUE_LEN) && (APPCFG_SRT0_QUEUE_LEN > 0)
#	define APPCFG_SRT0_EN
#endif
#if defined(APPCFG_SRT1_QUEUE_LEN) && (APPCFG_SRT1_QUEUE_LEN > 0)
#	define APPCFG_SRT1_EN
#endif
#if defined(APPCFG_SRT2_QUEUE_LEN) && (APPCFG_SRT2_QUEUE_LEN > 0)
#	define APPCFG_SRT2_EN
#endif
#if defined(APPCFG_SRT3_QUEUE_LEN) && (APPCFG_SRT3_QUEUE_LEN > 0)
#	define APPCFG_SRT3_EN
#endif

#ifdef APPCFG_SRT0_EN
static void app_srt0_activate(struct vsfsm_evtq_t *q);
#endif
#ifdef APPCFG_SRT1_EN
static void app_srt1_activate(struct vsfsm_evtq_t *q);
#endif
#ifdef APPCFG_SRT2_EN
static void app_srt2_activate(struct vsfsm_evtq_t *q);
#endif
#ifdef APPCFG_SRT3_EN
static void app_srt3_activate(struct vsfsm_evtq_t *q);
#endif

struct vsfapp_t
{
	struct usrapp_t *usrapp;

#ifdef APPCFG_VSFTIMER_EN
#	if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
	VSFPOOL_DEFINE(vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
#	elif defined(APPCFG_BUFMGR_SIZE)
	struct vsf_dynpool_t vsftimer_pool;
#	endif
#endif

#if VSFSM_CFG_PREMPT_EN
#	ifdef APPCFG_MAINQ_EN
	struct vsfsm_evtq_t mainq;
	struct vsfsm_evtq_element_t mainq_ele[APPCFG_NRT_QUEUE_LEN];
#	endif
#	ifdef APPCFG_SRT0_EN
	struct vsfsm_evtq_t srt0q;
	struct vsfsm_evtq_element_t srt0q_ele[APPCFG_SRT0_QUEUE_LEN];
#	endif
#	ifdef APPCFG_SRT1_EN
	struct vsfsm_evtq_t srt1q;
	struct vsfsm_evtq_element_t srt1q_ele[APPCFG_SRT1_QUEUE_LEN];
#	endif
#	ifdef APPCFG_SRT2_EN
	struct vsfsm_evtq_t srt2q;
	struct vsfsm_evtq_element_t srt2q_ele[APPCFG_SRT2_QUEUE_LEN];
#	endif
#	ifdef APPCFG_SRT3_EN
	struct vsfsm_evtq_t srt3q;
	struct vsfsm_evtq_element_t srt3q_ele[APPCFG_SRT3_QUEUE_LEN];
#	endif
#endif
} static app =
{
	.usrapp = (struct usrapp_t *)&usrapp,
	
#if VSFSM_CFG_PREMPT_EN
#	ifdef APPCFG_MAINQ_EN
	.mainq.size = dimof(app.mainq_ele),
	.mainq.queue = app.mainq_ele,
	.mainq.activate = NULL,
#	endif
#	ifdef APPCFG_SRT0_EN
	.srt0q.size = dimof(app.srt0q_ele),
	.srt0q.queue = app.srt0q_ele,
	.srt0q.activate = app_srt0_activate,
#	endif
#	ifdef APPCFG_SRT1_EN
	.srt1q.size = dimof(app.srt1q_ele),
	.srt1q.queue = app.srt1q_ele,
	.srt1q.activate = app_srt1_activate,
#	endif
#	ifdef APPCFG_SRT2_EN
	.srt2q.size = dimof(app.srt2q_ele),
	.srt2q.queue = app.srt2q_ele,
	.srt2q.activate = app_srt2_activate,
#	endif
#	ifdef APPCFG_SRT3_EN
	.srt3q.size = dimof(app.srt3q_ele),
	.srt3q.queue = app.srt3q_ele,
	.srt3q.activate = app_srt3_activate,
#	endif
#endif

#if defined(APPCFG_VSFTIMER_EN) && \
		!(defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)) && \
		defined(APPCFG_BUFMGR_SIZE)
	.vsftimer_pool.item_size = sizeof(struct vsftimer_t),
	.vsftimer_pool.pool_size = APPCFG_VSFTIMER_NUM,
#endif
};

#ifdef APPCFG_BUFMGR_SIZE
static uint8_t bufmgr_buff[APPCFG_BUFMGR_SIZE];
#endif

#ifdef APPCFG_VSFTIMER_EN
#	if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
static struct vsftimer_t* vsftimer_memop_alloc(void)
{
	return VSFPOOL_ALLOC(&app.vsftimer_pool, struct vsftimer_t);
}

static void vsftimer_memop_free(struct vsftimer_t *timer)
{
	VSFPOOL_FREE(&app.vsftimer_pool, timer);
}
#else
static struct vsftimer_t* vsftimer_memop_alloc(void)
{
	return vsf_dynpool_alloc(&app.vsftimer_pool);
}

static void vsftimer_memop_free(struct vsftimer_t *timer)
{
	vsf_dynpool_free(&app.vsftimer_pool, timer);
}
#endif

const struct vsftimer_mem_op_t vsftimer_memop =
{
	.alloc			= vsftimer_memop_alloc,
	.free			= vsftimer_memop_free,
};

// tickclk interrupt, simply call vsftimer_callback_int
static void app_tickclk_callback_int(void *param)
{
	vsftimer_callback_int();
}
#endif

static void vsfapp_init(struct vsfapp_t *app) @ "VSF_INIT_CODE"
{
	vsfhal_core_init(NULL);

#if VSFSM_CFG_PREMPT_EN
#	if defined(APPCFG_SRT3_EN)
	vsfsm_evtq_set(&app->srt3q);
#	elif defined(APPCFG_SRT2_EN)
	vsfsm_evtq_set(&app->srt2q);
#	elif defined(APPCFG_SRT1_EN)
	vsfsm_evtq_set(&app->srt1q);
#	elif defined(APPCFG_SRT0_EN)
	vsfsm_evtq_set(&app->srt0q);
#	elif defined(APPCFG_MAINQ_EN)
	vsfsm_evtq_set(&app->mainq);
#	else
	vsfsm_evtq_set(NULL);
#	endif
#endif

	vsfhal_tickclk_init(VSFHAL_TICKCLK_PRIORITY);
	vsfhal_tickclk_start();

#ifdef APPCFG_VSFTIMER_EN
#	if defined(APPCFG_VSFTIMER_NUM) && (APPCFG_VSFTIMER_NUM > 0)
	VSFPOOL_INIT(&app->vsftimer_pool, struct vsftimer_t, APPCFG_VSFTIMER_NUM);
#	elif defined(APPCFG_BUFMGR_SIZE)
	vsf_dynpool_init(&app->vsftimer_pool);
#	endif
	vsftimer_init((struct vsftimer_mem_op_t *)&vsftimer_memop);
	vsfhal_tickclk_config_cb(app_tickclk_callback_int, NULL);
#endif

#ifdef APPCFG_BUFMGR_SIZE
	vsf_bufmgr_init(bufmgr_buff, APPCFG_BUFMGR_SIZE);
#endif

#if VSFSM_CFG_PREMPT_EN
#	if defined(APPCFG_SRT3_EN)
	usrapp_srt3_init(app->usrapp);
#	endif
#	if defined(APPCFG_SRT2_EN)
	vsfsm_evtq_set(&app->srt2q);
	usrapp_srt2_init(app->usrapp);
#	endif
#	if defined(APPCFG_SRT1_EN)
	vsfsm_evtq_set(&app->srt1q);
	usrapp_srt1_init(app->usrapp);
#	endif
#	if defined(APPCFG_SRT0_EN)
	vsfsm_evtq_set(&app->srt0q);
	usrapp_srt0_init(app->usrapp);
#	endif
#	if defined(APPCFG_MAINQ_EN)
	vsfsm_evtq_set(&app->mainq);
	usrapp_nrt_init(app->usrapp);
#	endif
#else
	vsfsm_evtq_set(NULL);
#endif
}

#if defined(APPCFG_SRT_NUM) && (APPCFG_SRT_NUM > 0)
static void app_on_srt(void *param)
{
	struct vsfsm_evtq_t *evtq_cur = param, *evtq_old = vsfsm_evtq_set(evtq_cur);

	while (vsfsm_get_event_pending())
		vsfsm_poll();
	vsfsm_evtq_set(evtq_old);
}
#endif
#ifdef APPCFG_SRT0_EN
static void app_srt0_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_swi_trigger(0);
}
#endif
#ifdef APPCFG_SRT1_EN
static void app_srt1_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_swi_trigger(1);
}
#endif
#ifdef APPCFG_SRT2_EN
static void app_srt2_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_swi_trigger(2);
}
#endif
#ifdef APPCFG_SRT3_EN
static void app_srt3_activate(struct vsfsm_evtq_t *q)
{
	vsfhal_swi_trigger(3);
}
#endif

int vsfmain(void)
{
	DISABLE_GLOBAL_INTERRUPT();

	usrapp_initial_init(app.usrapp);

#ifdef APPCFG_MAINQ_EN
	vsfsm_evtq_init(&app.mainq);
#endif
#ifdef APPCFG_SRT0_EN
	vsfsm_evtq_init(&app.srt0q);
	vsfhal_swi_init(0, VSF_TASK_PRIORITY_L1, app_on_srt, &app.srt0q);
#endif
#ifdef APPCFG_SRT1_EN
	vsfsm_evtq_init(&app.srt1q);
	vsfhal_swi_init(1, VSF_TASK_PRIORITY_L2, app_on_srt, &app.srt1q);
#endif
#ifdef APPCFG_SRT2_EN
	vsfsm_evtq_init(&app.srt2q);
	vsfhal_swi_init(2, VSF_TASK_PRIORITY_L3, app_on_srt, &app.srt2q);
#endif
#ifdef APPCFG_SRT3_EN
	vsfsm_evtq_init(&app.srt3q);
	vsfhal_swi_init(3, VSF_TASK_PRIORITY_L4, app_on_srt, &app.srt3q);
#endif

	vsfapp_init(&app);
	ENABLE_GLOBAL_INTERRUPT();

	while (1)
	{
#if defined(APPCFG_MAINQ_EN)
		vsfsm_poll();
		DISABLE_GLOBAL_INTERRUPT();
		if (vsfsm_get_event_pending())
			ENABLE_GLOBAL_INTERRUPT();
		else
			vsfhal_core_sleep(VSFHAL_SLEEP_WFI);	// will enable interrupt
#else
		vsfhal_core_sleep(VSFHAL_SLEEP_WFI);
#endif
	}
}
