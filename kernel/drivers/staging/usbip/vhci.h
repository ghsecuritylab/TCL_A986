

#include <linux/platform_device.h>
#include <linux/usb/hcd.h>


struct vhci_device {
	struct usb_device *udev;

	/*
	 * devid specifies a remote usb device uniquely instead
	 * of combination of busnum and devnum.
	 */
	__u32 devid;

	/* speed of a remote device */
	enum usb_device_speed speed;

	/*  vhci root-hub port to which this device is attached  */
	__u32 rhport;

	struct usbip_device ud;


	/* lock for the below link lists */
	spinlock_t priv_lock;

	/* vhci_priv is linked to one of them. */
	struct list_head priv_tx;
	struct list_head priv_rx;

	/* vhci_unlink is linked to one of them */
	struct list_head unlink_tx;
	struct list_head unlink_rx;

	/* vhci_tx thread sleeps for this queue */
	wait_queue_head_t waitq_tx;
};


/* urb->hcpriv, use container_of() */
struct vhci_priv {
	unsigned long seqnum;
	struct list_head list;

	struct vhci_device *vdev;
	struct urb *urb;
};


struct vhci_unlink {
	/* seqnum of this request */
	unsigned long seqnum;

	struct list_head list;

	/* seqnum of the unlink target */
	unsigned long unlink_seqnum;
};

#define VHCI_NPORTS 8

/* for usb_bus.hcpriv */
struct vhci_hcd {
	spinlock_t	lock;

	u32	port_status[VHCI_NPORTS];

	unsigned	resuming:1;
	unsigned long	re_timeout;

	atomic_t seqnum;

	/*
	 * NOTE:
	 * wIndex shows the port number and begins from 1.
	 * But, the index of this array begins from 0.
	 */
	struct vhci_device vdev[VHCI_NPORTS];

	/* vhci_device which has not been assiged its address yet */
	int pending_port;
};


extern struct vhci_hcd *the_controller;
extern struct attribute_group dev_attr_group;


/*-------------------------------------------------------------------------*/
/* prototype declaration */

/* vhci_hcd.c */
void rh_port_connect(int rhport, enum usb_device_speed speed);
void rh_port_disconnect(int rhport);
void vhci_rx_loop(struct usbip_task *ut);
void vhci_tx_loop(struct usbip_task *ut);

#define hardware		(&the_controller->pdev.dev)

static inline struct vhci_device *port_to_vdev(__u32 port)
{
	return &the_controller->vdev[port];
}

static inline struct vhci_hcd *hcd_to_vhci(struct usb_hcd *hcd)
{
	return (struct vhci_hcd *) (hcd->hcd_priv);
}

static inline struct usb_hcd *vhci_to_hcd(struct vhci_hcd *vhci)
{
	return container_of((void *) vhci, struct usb_hcd, hcd_priv);
}

static inline struct device *vhci_dev(struct vhci_hcd *vhci)
{
	return vhci_to_hcd(vhci)->self.controller;
}
