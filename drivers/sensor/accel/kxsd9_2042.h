#ifndef __KXSD92042_H__
#define __KXSD92042_H__

#define ACC_DEV_NAME "accelerometer"
#define ACC_DEV_MAJOR 241

/* smb ioctl command label */
#define IOCTL_KXSD9_GET_ACC_VALUE		0
#define DCM_IOC_MAGIC			's'
#define IOC_SET_ACCELEROMETER	_IO (DCM_IOC_MAGIC, 0x64)
#define BMA150_CALIBRATION		_IOWR(DCM_IOC_MAGIC,48,short)

#define KXSD9_POWER_OFF               0
#define KXSD9_POWER_ON                1


/* Function prototypes */



#endif   // __KXSD92042_H__





