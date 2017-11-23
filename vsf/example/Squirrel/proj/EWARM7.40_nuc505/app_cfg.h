#define APPCFG_VSFTIMER_NUM				32
#define APPCFG_BUFMGR_EN				1

// The 3 MACROs below define the Hard/Soft/Non-RealTime event queue
// undefine to indicate that the corresponding event queue is not supported
//	note that AT LEASE one event queue should be defined
// define to 0 indicating that the corresponding events will not be queued
//	note that the events can be unqueued ONLY IF the corresponding tasks will
//		not receive events from tasks in higher priority
// define to n indicating the length of corresponding real time event queue
//#define APPCFG_HRT_QUEUE_LEN			0
#define APPCFG_SRT_QUEUE_LEN			32
#define APPCFG_NRT_QUEUE_LEN			32

#if (defined(APPCFG_HRT_QUEUE_LEN) && (APPCFG_HRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_SRT_QUEUE_LEN) && (APPCFG_SRT_QUEUE_LEN > 0)) ||\
	(defined(APPCFG_NRT_QUEUE_LEN) && (APPCFG_NRT_QUEUE_LEN > 0))
#define VSFSM_CFG_PREMPT_EN				1
#else
#define VSFSM_CFG_PREMPT_EN				0
#endif

// define APPCFG_USR_POLL for round robin scheduling
//#define APPCFG_USR_POLL

#ifdef APPCFG_USR_POLL
#define APPCFG_TICKCLK_PRIORITY			-1
#else
#define APPCFG_TICKCLK_PRIORITY			0xFF
#endif

/*******************************************************************************
	APP Paramter Config
*******************************************************************************/
#define APPCFG_USBD_PRIORITY			0xFF
