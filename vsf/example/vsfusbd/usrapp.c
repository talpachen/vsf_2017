#include "vsf.h"
#include "usrapp.h"

#define APPCFG_USBD_VID							0x4321
#define APPCFG_INFO_PRODUCT						"Product    : Test"
#define APPCFG_USBD_PID							0x1234
#define APPCFG_USBD_VENDOR_DESC_LEN				10
#define APPCFG_USBD_VENDOR_DESC					'1', 0, '2', 0, '3', 0, '4', 0
#define APPCFG_USBD_PRODUCT_DESC_LEN			10
#define APPCFG_USBD_PRODUCT_DESC				'1', 0, '2', 0, '3', 0, '4', 0
#define APPCFG_SCSI_VOLID						"1234 1234 1"
#define APPCFG_SCSI_VENDOR						"1234    "
#define APPCFG_SCSI_PRODUCT						"1234 456        "

// USB descriptors
static const uint8_t USB_DeviceDescriptor[] =
{
	0x12,	// bLength = 18
	USB_DT_DEVICE,	// USB_DESC_TYPE_DEVICE
	0x00,
	0x02,	// bcdUSB
	0x00,	// device class:
	0x00,	// device sub class
	0x00,	// device protocol
	0x40,	// max packet size
	(APPCFG_USBD_VID >> 0) & 0xFF,
	(APPCFG_USBD_VID >> 8) & 0xFF,
			// vendor
	(APPCFG_USBD_PID >> 0) & 0xFF,
	(APPCFG_USBD_PID >> 8) & 0xFF,
			// product
	0x00,
	0x01,	// bcdDevice
	1,		// manu facturer
	2,		// product
	3,		// serial number
	0x01	// number of configuration
};

static const uint8_t USB_ConfigDescriptor[] =
{
	// Configuation Descriptor
	0x09,	// bLength: Configuation Descriptor size
	USB_DT_CONFIG,
			// bDescriptorType: Configuration
	32,		// wTotalLength:no of returned bytes*
	0x00,
	0x01,	// bNumInterfaces: 1 interface
	0x01,	// bConfigurationValue: Configuration value
	0x00,	// iConfiguration: Index of string descriptor describing the configuration
	0x80,	// bmAttributes: bus powered
	0x64,	// MaxPower 200 mA

	// Interface Descriptor for MSC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0,		// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x08,	// bInterfaceClass: MSC
	0x06,	// bInterfaceSubClass: SCSI
	0x50,	// bInterfaceProtocol:
	0x00,	// iInterface:

	// Endpoint 1 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x81,	// bEndpointAddress: (IN1)
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer

	// Endpoint 2 Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x02,	// bEndpointAddress: (OUT2)
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00	// bInterval
};

static const uint8_t USB_StringLangID[] =
{
	4,
	USB_DT_STRING,
	0x09,
	0x04
};

static const uint8_t USB_StringVendor[] =
{
	APPCFG_USBD_VENDOR_DESC_LEN,
	USB_DT_STRING,
	APPCFG_USBD_VENDOR_DESC,
};

static const uint8_t USB_StringProduct[] =
{
	APPCFG_USBD_PRODUCT_DESC_LEN,
	USB_DT_STRING,
	APPCFG_USBD_PRODUCT_DESC,
};

static uint8_t USB_StringSerial[68] =
{
	68,
	USB_DT_STRING,
	'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0,
	'8', 0, '9', 0, 'A', 0, 'B', 0, 'C', 0, 'D', 0, 'E', 0, 'F', 0,
	'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0,
	'8', 0, '9', 0, 'A', 0, 'B', 0, 'C', 0, 'D', 0, 'E', 0, 'F', 0,
};

static const struct vsfusbd_desc_filter_t USB_descriptors[] =
{
	VSFUSBD_DESC_DEVICE(0, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor)),
	VSFUSBD_DESC_CONFIG(0, 0, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor)),
	VSFUSBD_DESC_STRING(0, 0, USB_StringLangID, sizeof(USB_StringLangID)),
	VSFUSBD_DESC_STRING(0x0409, 1, USB_StringVendor, sizeof(USB_StringVendor)),
	VSFUSBD_DESC_STRING(0x0409, 2, USB_StringProduct, sizeof(USB_StringProduct)),
	VSFUSBD_DESC_STRING(0x0409, 3, USB_StringSerial, sizeof(USB_StringSerial)),
	VSFUSBD_DESC_NULL
};

static struct fakefat32_file_t fakefat32_root_dir[] =
{
	{
		.memfile.file.name = APPCFG_SCSI_VOLID,
		.memfile.file.attr = VSFILE_ATTR_VOLUMID,
	},
	{
		.memfile.file.name = NULL,
	},
};

struct usrapp_t usrapp =
{
	.mal.fakefat32.sector_size				= 512,
	.mal.fakefat32.sector_number			= 0x00010800,
	.mal.fakefat32.sectors_per_cluster		= 1,
	.mal.fakefat32.volume_id				= 0x0CA93E47,
	.mal.fakefat32.disk_id					= 0x12345678,
	.mal.fakefat32.root[0].memfile.file.name= "ROOT",
	.mal.fakefat32.root[0].memfile.d.child	= (struct vsfile_memfile_t *)fakefat32_root_dir,

	.mal.pbuffer[0]							= usrapp.mal.buffer[0],
	.mal.pbuffer[1]							= usrapp.mal.buffer[1],
	.mal.mal.drv							= &fakefat32_mal_drv,
	.mal.mal.param							= &usrapp.mal.fakefat32,
	.mal.scsistream.mbuf.count				= dimof(usrapp.mal.buffer),
	.mal.scsistream.mbuf.size				= sizeof(usrapp.mal.buffer[0]),
	.mal.scsistream.mbuf.buffer_list		= usrapp.mal.pbuffer,
	
	.mal.mal2scsi.malstream.mal				= &usrapp.mal.mal,
	.mal.mal2scsi.cparam.block_size			= 512,
	.mal.mal2scsi.cparam.removable			= true,
	.mal.mal2scsi.cparam.vendor				= APPCFG_SCSI_VENDOR,
	.mal.mal2scsi.cparam.product			= APPCFG_SCSI_PRODUCT,
	.mal.mal2scsi.cparam.revision			= "1.00",
	.mal.mal2scsi.cparam.type				= SCSI_PDT_DIRECT_ACCESS_BLOCK,
	
	.mal.lun[0].op							= (struct vsfscsi_lun_op_t *)&vsf_mal2scsi_op,
	// lun->stream MUST be scsistream for mal2scsi
	.mal.lun[0].stream						= (struct vsf_stream_t *)&usrapp.mal.scsistream,
	.mal.lun[0].param						= &usrapp.mal.mal2scsi,
	.mal.scsi_dev.max_lun					= 0,
	.mal.scsi_dev.lun						= usrapp.mal.lun,
	
	.usbd.msc.param.ep_in					= 1,
	.usbd.msc.param.ep_out					= 2,
	.usbd.msc.param.scsi_dev				= &usrapp.mal.scsi_dev,
	.usbd.ifaces[0].class_protocol			= (struct vsfusbd_class_protocol_t *)&vsfusbd_MSCBOT_class,
	.usbd.ifaces[0].protocol_param			= &usrapp.usbd.msc.param,
	.usbd.config[0].num_of_ifaces			= dimof(usrapp.usbd.ifaces),
	.usbd.config[0].iface					= usrapp.usbd.ifaces,
	.usbd.device.num_of_configuration		= dimof(usrapp.usbd.config),
	.usbd.device.config						= usrapp.usbd.config,
	.usbd.device.desc_filter				= (struct vsfusbd_desc_filter_t *)USB_descriptors,
	.usbd.device.device_class_iface			= 0,
	.usbd.device.drv						= (struct vsfhal_usbd_t *)&vsfhal_usbd,
	.usbd.device.int_priority				= VSFHAL_USB_HS_PRIORITY,
};

static void usrapp_usbd_conn(void *p)
{
	struct usrapp_t *app = p;
	app->usbd.device.drv->connect();
}

static void usrapp_pendsv_do(void *p)
{
	struct usrapp_t *app = p;

	// mal init
	vsfscsi_init(&app->mal.scsi_dev);
	vsfusbd_device_init(&app->usbd.device);
	app->usbd.device.drv->disconnect();
	
	vsftimer_create_cb(200, 1, usrapp_usbd_conn, app);
}

void usrapp_initial_init(struct usrapp_t *app)
{
	// null
}

void usrapp_srt_init(struct usrapp_t *app)
{
	// Increase the difficulty of disassembly
	vsftimer_create_cb(10, 1, usrapp_pendsv_do, app);
}

void usrapp_nrt_init(struct usrapp_t *app)
{

}

