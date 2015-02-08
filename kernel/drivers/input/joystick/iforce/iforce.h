


#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/usb.h>
#include <linux/serio.h>
#include <linux/circ_buf.h>
#include <linux/mutex.h>

#include <linux/ioport.h>


#define IFORCE_MAX_LENGTH	16

/* iforce::bus */
#define IFORCE_232	1
#define IFORCE_USB	2

#define IFORCE_EFFECTS_MAX	32

#define FF_MOD1_IS_USED		0
#define FF_MOD2_IS_USED		1
#define FF_CORE_IS_USED		2
#define FF_CORE_IS_PLAYED	3	/* Effect is currently being played */
#define FF_CORE_SHOULD_PLAY	4	/* User wants the effect to be played */
#define FF_CORE_UPDATE		5	/* Effect is being updated */
#define FF_MODCORE_CNT		6

struct iforce_core_effect {
	/* Information about where modifiers are stored in the device's memory */
	struct resource mod1_chunk;
	struct resource mod2_chunk;
	unsigned long flags[BITS_TO_LONGS(FF_MODCORE_CNT)];
};

#define FF_CMD_EFFECT		0x010e
#define FF_CMD_ENVELOPE		0x0208
#define FF_CMD_MAGNITUDE	0x0303
#define FF_CMD_PERIOD		0x0407
#define FF_CMD_CONDITION	0x050a

#define FF_CMD_AUTOCENTER	0x4002
#define FF_CMD_PLAY		0x4103
#define FF_CMD_ENABLE		0x4201
#define FF_CMD_GAIN		0x4301

#define FF_CMD_QUERY		0xff01

/* Buffer for async write */
#define XMIT_SIZE		256
#define XMIT_INC(var, n)	(var)+=n; (var)&= XMIT_SIZE -1
/* iforce::xmit_flags */
#define IFORCE_XMIT_RUNNING	0
#define IFORCE_XMIT_AGAIN	1

struct iforce_device {
	u16 idvendor;
	u16 idproduct;
	char *name;
	signed short *btn;
	signed short *abs;
	signed short *ff;
};

struct iforce {
	struct input_dev *dev;		/* Input device interface */
	struct iforce_device *type;
	int bus;

	unsigned char data[IFORCE_MAX_LENGTH];
	unsigned char edata[IFORCE_MAX_LENGTH];
	u16 ecmd;
	u16 expect_packet;

#ifdef CONFIG_JOYSTICK_IFORCE_232
	struct serio *serio;		/* RS232 transfer */
	int idx, pkt, len, id;
	unsigned char csum;
#endif
#ifdef CONFIG_JOYSTICK_IFORCE_USB
	struct usb_device *usbdev;	/* USB transfer */
	struct urb *irq, *out, *ctrl;
	struct usb_ctrlrequest cr;
#endif
	spinlock_t xmit_lock;
	/* Buffer used for asynchronous sending of bytes to the device */
	struct circ_buf xmit;
	unsigned char xmit_data[XMIT_SIZE];
	unsigned long xmit_flags[1];

					/* Force Feedback */
	wait_queue_head_t wait;
	struct resource device_memory;
	struct iforce_core_effect core_effects[IFORCE_EFFECTS_MAX];
	struct mutex mem_mutex;
};

/* Get hi and low bytes of a 16-bits int */
#define HI(a)	((unsigned char)((a) >> 8))
#define LO(a)	((unsigned char)((a) & 0xff))

#define HIFIX80(a) ((unsigned char)(((a)<0? (a)+255 : (a))>>8))

/* Encode a time value */
#define TIME_SCALE(a)	(a)


/* Public functions */
/* iforce-serio.c */
void iforce_serial_xmit(struct iforce *iforce);

/* iforce-usb.c */
void iforce_usb_xmit(struct iforce *iforce);

/* iforce-main.c */
int iforce_init_device(struct iforce *iforce);

/* iforce-packets.c */
int iforce_control_playback(struct iforce*, u16 id, unsigned int);
void iforce_process_packet(struct iforce *iforce, u16 cmd, unsigned char *data);
int iforce_send_packet(struct iforce *iforce, u16 cmd, unsigned char* data);
void iforce_dump_packet(char *msg, u16 cmd, unsigned char *data) ;
int iforce_get_id_packet(struct iforce *iforce, char *packet);

/* iforce-ff.c */
int iforce_upload_periodic(struct iforce *, struct ff_effect *, struct ff_effect *);
int iforce_upload_constant(struct iforce *, struct ff_effect *, struct ff_effect *);
int iforce_upload_condition(struct iforce *, struct ff_effect *, struct ff_effect *);

/* Public variables */
extern struct serio_driver iforce_serio_drv;
extern struct usb_driver iforce_usb_driver;