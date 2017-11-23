static const uint8_t USB_DeviceDescriptor[] =
{
	USB_DT_DEVICE_SIZE,
	USB_DT_DEVICE,
	0x00, 0x02,	// bcdUSB
	0xEF,		// device class
	0x02,		// device sub class
	0x01,		// device protocol
	0x40,		// max packet size
	0x09, 0x12,	// vendor
	0x67, 0x66,	// product
	0x00, 0x01,	// bcdDevice
	1,			// manu facturer
	2,			// product
	3,			// serial number
	0x01		// number of configuration 
};

static const uint8_t USB_ConfigDescriptor[] =
{
	USB_DT_CONFIG_SIZE,
	USB_DT_CONFIG,
	230, 0,		// wTotalLength
	0x07,		// bNumInterfaces
	0x01,		// bConfigurationValue: Configuration value
	0x00,		// iConfiguration: Index of string descriptor describing the configuration
	0x80,		// bmAttributes: bus powered
	0x32,		// MaxPower

	// Versaloon interface
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType:
	0x00,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints
	0xff,	// bInterfaceClass
	0x01,	// bInterfaceSubClass
	0x00,	// nInterfaceProtocol
	0x04,	// iInterface:

	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x81,	// bEndpointAddress
	0x02,	// bmAttributes: Bulk
	0x40,	// wMaxPacketSize:
	0x00,
	0x00,	// bInterval
		
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x02,	// bEndpointAddress
	0x02,	// bmAttributes: Bulk
	0x40,	// wMaxPacketSize:
	0x00,
	0x00,	// bInterval

	// IAD CDC UART
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,	// bDescriptorType: IAD
	1,		// bFirstInterface
	2,		// bInterfaceCount
	0x02,	// bFunctionClass
	0x02,	// bFunctionSubClass
	0x01,	// bFunctionProtocol
	0x05,	// iFunction
	
	// Interface Descriptor for CDC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x01,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x01,	// bNumEndpoints: One endpoints used
	0x02,	// bInterfaceClass: Communication Interface Class
	0x02,	// bInterfaceSubClass: Abstract Control Model
	0x01,	// bInterfaceProtocol: Common AT commands
	0x00,	// iInterface:
	
	// Header Functional Descriptor
	0x05,	// bLength: Endpoint Descriptor size
	0x24,	// bDescriptorType: CS_INTERFACE
	0x00,	// bDescriptorSubtype: Header Func Desc
	0x10,	// bcdCDC: spec release number
	0x01,
	
	// Call Managment Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x01,	// bDescriptorSubtype: Call Management Func Desc
	0x00,	// bmCapabilities: D0+D1
	0x01,	// bDataInterface: 1
	
	// ACM Functional Descriptor
	0x04,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x02,	// bDescriptorSubtype: Abstract Control Management desc
	0x02,	// bmCapabilities
	
	// Union Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x06,	// bDescriptorSubtype: Union func desc
	0,		// bMasterInterface: Communication class interface
	1,		// bSlaveInterface0: Data Class Interface
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x83,	// bEndpointAddress:
	0x03,	// bmAttributes: Interrupt
	8,		// wMaxPacketSize:
	0x00,
	0xFF,	// bInterval:
	
	// Data class interface descriptor
	0x09,	// bLength: Endpoint Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x02,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x0A,	// bInterfaceClass: CDC
	0x00,	// bInterfaceSubClass:
	0x00,	// bInterfaceProtocol:
	0x00,	// iInterface:
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x84,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x05,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer

	// IAD CDC UART
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,	// bDescriptorType: IAD
	3,		// bFirstInterface
	2,		// bInterfaceCount
	0x02,	// bFunctionClass
	0x02,	// bFunctionSubClass
	0x01,	// bFunctionProtocol
	0x06,	// iFunction
	
	// Interface Descriptor for CDC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x03,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x01,	// bNumEndpoints: One endpoints used
	0x02,	// bInterfaceClass: Communication Interface Class
	0x02,	// bInterfaceSubClass: Abstract Control Model
	0x01,	// bInterfaceProtocol: Common AT commands
	0x00,	// iInterface:
	
	// Header Functional Descriptor
	0x05,	// bLength: Endpoint Descriptor size
	0x24,	// bDescriptorType: CS_INTERFACE
	0x00,	// bDescriptorSubtype: Header Func Desc
	0x10,	// bcdCDC: spec release number
	0x01,
	
	// Call Managment Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x01,	// bDescriptorSubtype: Call Management Func Desc
	0x00,	// bmCapabilities: D0+D1
	0x01,	// bDataInterface: 1
	
	// ACM Functional Descriptor
	0x04,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x02,	// bDescriptorSubtype: Abstract Control Management desc
	0x02,	// bmCapabilities
	
	// Union Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x06,	// bDescriptorSubtype: Union func desc
	2,		// bMasterInterface: Communication class interface
	3,		// bSlaveInterface0: Data Class Interface
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x86,	// bEndpointAddress:
	0x03,	// bmAttributes: Interrupt
	8,		// wMaxPacketSize:
	0x00,
	0xFF,	// bInterval:
	
	// Data class interface descriptor
	0x09,	// bLength: Endpoint Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x04,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x0A,	// bInterfaceClass: CDC
	0x00,	// bInterfaceSubClass:
	0x00,	// bInterfaceProtocol:
	0x00,	// iInterface:
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x87,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x08,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer
	
	// IAD CDC UART
	0x08,	// bLength: IAD Descriptor size
	USB_DT_INTERFACE_ASSOCIATION,	// bDescriptorType: IAD
	5,		// bFirstInterface
	2,		// bInterfaceCount
	0x02,	// bFunctionClass
	0x02,	// bFunctionSubClass
	0x01,	// bFunctionProtocol
	0x07,	// iFunction
	
	// Interface Descriptor for CDC
	0x09,	// bLength: Interface Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x05,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x01,	// bNumEndpoints: One endpoints used
	0x02,	// bInterfaceClass: Communication Interface Class
	0x02,	// bInterfaceSubClass: Abstract Control Model
	0x01,	// bInterfaceProtocol: Common AT commands
	0x00,	// iInterface:
	
	// Header Functional Descriptor
	0x05,	// bLength: Endpoint Descriptor size
	0x24,	// bDescriptorType: CS_INTERFACE
	0x00,	// bDescriptorSubtype: Header Func Desc
	0x10,	// bcdCDC: spec release number
	0x01,
	
	// Call Managment Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x01,	// bDescriptorSubtype: Call Management Func Desc
	0x00,	// bmCapabilities: D0+D1
	0x01,	// bDataInterface: 1
	
	// ACM Functional Descriptor
	0x04,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x02,	// bDescriptorSubtype: Abstract Control Management desc
	0x02,	// bmCapabilities
	
	// Union Functional Descriptor
	0x05,	// bFunctionLength
	0x24,	// bDescriptorType: CS_INTERFACE
	0x06,	// bDescriptorSubtype: Union func desc
	4,		// bMasterInterface: Communication class interface
	5,		// bSlaveInterface0: Data Class Interface
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x89,	// bEndpointAddress:
	0x03,	// bmAttributes: Interrupt
	8,		// wMaxPacketSize:
	0x00,
	0xFF,	// bInterval:
	
	// Data class interface descriptor
	0x09,	// bLength: Endpoint Descriptor size
	USB_DT_INTERFACE,
			// bDescriptorType: Interface
	0x06,	// bInterfaceNumber: Number of Interface
	0x00,	// bAlternateSetting: Alternate setting
	0x02,	// bNumEndpoints: Two endpoints used
	0x0A,	// bInterfaceClass: CDC
	0x00,	// bInterfaceSubClass:
	0x00,	// bInterfaceProtocol:
	0x00,	// iInterface:
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x8a,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval
	
	// Endpoint Descriptor
	0x07,	// bLength: Endpoint Descriptor size
	USB_DT_ENDPOINT,
			// bDescriptorType: Endpoint
	0x0b,	// bEndpointAddress:
	0x02,	// bmAttributes: Bulk
	64,		// wMaxPacketSize:
	0x00,
	0x00,	// bInterval: ignore for Bulk transfer
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
	8,
	USB_DT_STRING,
	'V', 0, 'S', 0, 'F', 0,
};

static const uint8_t USB_StringProduct[] =
{
	22,
	USB_DT_STRING,
	'U', 0, 'S', 0, 'B', 0, ' ', 0, 'S', 0, 'e', 0, 'r', 0, 'i', 0,
	'a', 0, 'l', 0,
};

static const uint8_t USB_StringSerial[50] =
{
	50,
	USB_DT_STRING,
	'0', 0, '1', 0, '2', 0, '3', 0, '4', 0, '5', 0, '6', 0, '7', 0, 
	'8', 0, '9', 0, 'A', 0, 'B', 0, 'C', 0, 'D', 0, 'E', 0, 'F', 0, 
	'0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, '0', 0, 
};

static const uint8_t VLLINK_StringFunc[] =
{
	38,
	USB_DT_STRING,
	'V', 0, 'S', 0, 'L', 0, 'L', 0, 'I', 0, 'N', 0, 'K', 0, '(', 0, 'V', 0,
	'e', 0, 'r', 0, 's', 0, 'a', 0, 'l', 0, 'o', 0, 'o', 0, 'n', 0, ')', 0,
};

static const uint8_t SHELL_CDC_StringFunc[] =
{
	20,
	USB_DT_STRING,
	'S', 0, 'H', 0, 'E', 0, 'L', 0, 'L', 0, '-', 0, 'C', 0, 'D', 0, 'C', 0,
};

static const uint8_t WIFI_CDC_StringFunc[] =
{
	18,
	USB_DT_STRING,
	'W', 0, 'I', 0, 'F', 0, 'I', 0, '-', 0, 'C', 0, 'D', 0, 'C', 0,
};

static const uint8_t EXT_CDC_StringFunc[] =
{
	16,
	USB_DT_STRING,
	'E', 0, 'X', 0, 'T', 0, '-', 0, 'C', 0, 'D', 0, 'C', 0,
};

static const struct vsfusbd_desc_filter_t USB_descriptors[] =
{
	VSFUSBD_DESC_DEVICE(0, USB_DeviceDescriptor, sizeof(USB_DeviceDescriptor)),
	VSFUSBD_DESC_CONFIG(0, 0, USB_ConfigDescriptor, sizeof(USB_ConfigDescriptor)),
	VSFUSBD_DESC_STRING(0, 0, USB_StringLangID, sizeof(USB_StringLangID)),
	VSFUSBD_DESC_STRING(0x0409, 1, USB_StringVendor, sizeof(USB_StringVendor)),
	VSFUSBD_DESC_STRING(0x0409, 2, USB_StringProduct, sizeof(USB_StringProduct)),
	VSFUSBD_DESC_STRING(0x0409, 3, USB_StringSerial, sizeof(USB_StringSerial)),
	VSFUSBD_DESC_STRING(0x0409, 4, VLLINK_StringFunc, sizeof(VLLINK_StringFunc)),
	VSFUSBD_DESC_STRING(0x0409, 5, WIFI_CDC_StringFunc, sizeof(WIFI_CDC_StringFunc)),
	VSFUSBD_DESC_STRING(0x0409, 6, EXT_CDC_StringFunc, sizeof(EXT_CDC_StringFunc)),
	VSFUSBD_DESC_NULL
};