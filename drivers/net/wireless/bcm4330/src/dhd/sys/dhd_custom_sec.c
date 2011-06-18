#include <typedefs.h>
#include <linuxver.h>
#include <osl.h>

#include <proto/ethernet.h>
#include <dngl_stats.h>
#include <dhd.h>
#include <dhd_dbg.h>

#include <linux/fcntl.h>
#include <linux/fs.h>

struct dhd_info;
extern int _dhd_set_mac_address(struct dhd_info *dhd, int ifidx, struct ether_addr *addr);


#ifdef READ_MACADDR

int
dhd_read_macaddr(struct dhd_info *dhd, struct ether_addr *mac)
{
    struct file *fp      = NULL;
    struct file *fpnv      = NULL;
    char macbuffer[18]   = {0};
    mm_segment_t oldfs   = {0};
	char randommac[3]    = {0};
	char buf[18]         = {0};
	char* filepath       = "/data/.mac.info";
#ifdef CONFIG_TARGET_LOCALE_VZW
    char* nvfilepath       = "/data/misc/wifi/.nvmac.info";
#else
    char* nvfilepath       = "/data/.nvmac.info";
#endif
	int ret = 0;

	//MAC address copied from nv
	fpnv = filp_open(nvfilepath, O_RDONLY, 0);
	if (IS_ERR(fpnv)) {
start_readmac:
		fpnv = NULL;
		fp = filp_open(filepath, O_RDONLY, 0);
		if (IS_ERR(fp)) {
			/* File Doesn't Exist. Create and write mac addr.*/
			fp = filp_open(filepath, O_RDWR | O_CREAT, 0666);
			if(IS_ERR(fp)) {
				DHD_ERROR(("[WIFI] %s: File open error\n", filepath));
				return -1;
			}

			oldfs = get_fs();
			set_fs(get_ds());

			/* Generating the Random Bytes for 3 last octects of the MAC address */
			get_random_bytes(randommac, 3);
			
			sprintf(macbuffer,"%02X:%02X:%02X:%02X:%02X:%02X\n",
				0x60,0xd0,0xa9,randommac[0],randommac[1],randommac[2]);
			DHD_INFO(("[WIFI] The Random Generated MAC ID : %s\n", macbuffer));
			printk("[WIFI] The Random Generated MAC ID : %s\n", macbuffer);

			if(fp->f_mode & FMODE_WRITE) {			
				ret = fp->f_op->write(fp, (const char *)macbuffer, sizeof(macbuffer), &fp->f_pos);
				if(ret < 0)
					DHD_ERROR(("[WIFI] Mac address [%s] Failed to write into File: %s\n", macbuffer, filepath));
				else
					DHD_INFO(("[WIFI] Mac address [%s] written into File: %s\n", macbuffer, filepath));
			}
			set_fs(oldfs);
		}
		/* Reading the MAC Address from .mac.info file( the existed file or just created file)*/
		//rtn_value=kernel_read(fp, fp->f_pos, buf, 18);	
		ret = kernel_read(fp, 0, buf, 18);	
	}
	else {
		/* Reading the MAC Address from .nvmac.info file( the existed file or just created file)*/
		ret = kernel_read(fpnv, 0, buf, 18);
		buf[17] ='\0';   // to prevent abnormal string display when mac address is displayed on the screen. 
		printk("Read MAC : [%s] [%d] \r\n" , buf, strncmp(buf , "00:00:00:00:00:00" , 17));
		if(strncmp(buf , "00:00:00:00:00:00" , 17) == 0) {
			filp_close(fpnv, NULL);
			goto start_readmac;
		}

		fp = filp_open(filepath, O_RDWR | O_CREAT, 0666); // File is always created.
		if(IS_ERR(fp)) {
			DHD_ERROR(("[WIFI] %s: File open error\n", filepath));
			if (fpnv)
				filp_close(fpnv, NULL);
			return -1;
		}
		else {
			oldfs = get_fs();
			set_fs(get_ds());
			
			if(fp->f_mode & FMODE_WRITE) {
				ret = fp->f_op->write(fp, (const char *)buf, sizeof(buf), &fp->f_pos);
				if(ret < 0)
					DHD_ERROR(("[WIFI] Mac address [%s] Failed to write into File: %s\n", buf, filepath));
				else
					DHD_INFO(("[WIFI] Mac address [%s] written into File: %s\n", buf, filepath));
			}       
			set_fs(oldfs);

			ret = kernel_read(fp, 0, buf, 18);	
		}

	}

	if(ret)
		sscanf(buf,"%02X:%02X:%02X:%02X:%02X:%02X",
			   mac->octet[0], mac->octet[1], mac->octet[2], 
			   mac->octet[3], mac->octet[4], mac->octet[5]);
	else
		DHD_ERROR(("dhd_bus_start: Reading from the '%s' returns 0 bytes\n", filepath));

	if (fp)
		filp_close(fp, NULL);
	if (fpnv)
		filp_close(fpnv, NULL);    	

	/* Writing Newly generated MAC ID to the Dongle */
	if (0 == _dhd_set_mac_address(dhd, 0, mac))
		DHD_INFO(("dhd_bus_start: MACID is overwritten\n"));
	else
		DHD_ERROR(("dhd_bus_start: _dhd_set_mac_address() failed\n"));

    return 0;
}

#endif /* READ_MACADDR */
#ifdef RDWR_MACADDR
static int g_iMacFlag;

enum {
	MACADDR_NONE =0 ,
	MACADDR_MOD,
	MACADDR_MOD_RANDOM,
	MACADDR_MOD_NONE,
	MACADDR_COB,
	MACADDR_COB_RANDOM
};

int WriteRDWR_Macaddr(struct ether_addr *mac)
{
	char* filepath			= "/data/.mac.info";
	struct file *fp_mac	= NULL;
	char buf[18]			= {0};
	mm_segment_t oldfs		= {0};
	int ret = -1;

	if ((g_iMacFlag != MACADDR_COB) && (g_iMacFlag != MACADDR_MOD))
		return 0;
	
	sprintf(buf,"%02X:%02X:%02X:%02X:%02X:%02X\n",
			mac->octet[0],mac->octet[1],mac->octet[2],
			mac->octet[3],mac->octet[4],mac->octet[5]);

	fp_mac = filp_open(filepath, O_RDWR | O_CREAT, 0666); // File is always created.
	if(IS_ERR(fp_mac)) {
		DHD_ERROR(("[WIFI] %s: File open error\n", filepath));
		return -1;
	}
	else {
		oldfs = get_fs();
		set_fs(get_ds());
		
		if(fp_mac->f_mode & FMODE_WRITE) {
			ret = fp_mac->f_op->write(fp_mac, (const char *)buf, sizeof(buf), &fp_mac->f_pos);
			if(ret < 0)
				DHD_ERROR(("[WIFI] Mac address [%s] Failed to write into File: %s\n", buf, filepath));
			else
				DHD_INFO(("[WIFI] Mac address [%s] written into File: %s\n", buf, filepath));
		}       
		set_fs(oldfs);
		filp_close(fp_mac, NULL);
	}

	return 0;
	
}

#if 0 /* disable because it's not used yet */
int ReadMacAddress_OTP(dhd_pub_t *dhdp, char cur_mac[])
{
	int ret = -1;

	dhd_os_proto_block(dhdp);
	strcpy(cur_mac, "cur_etheraddr");
	ret = dhdcdc_query_ioctl(dhdp, 0, WLC_GET_VAR, cur_mac, sizeof(cur_mac));
	if (ret < 0) {
		printk("Current READ MAC error \r\n");
		memset(cur_mac , 0 , ETHER_ADDR_LEN);
		return -1;
	}
	else {
		printk("READ MAC (OTP) : [%02X][%02X][%02X][%02X][%02X][%02X] \r\n" , 
		cur_mac[0], cur_mac[1], cur_mac[2], cur_mac[3], cur_mac[4], cur_mac[5]);
	}
	dhd_os_proto_unblock(dhdp);

	return 0;
}
#endif

int CheckRDWR_Macaddr(	struct dhd_info *dhd, dhd_pub_t *dhdp, struct ether_addr *mac)
{
	struct file *fp_mac	= NULL;
	struct file *fp_nvm	= NULL;
	char macbuffer[18]		= {0};
	mm_segment_t oldfs		= {0};
	char randommac[3]		= {0};
	char buf[18]			= {0};
	char* filepath			= "/data/.mac.info";
	char* nvfilepath		= "/data/.nvmac.info";
	char cur_mac[128]		= {0};
	char dummy_mac[ETHER_ADDR_LEN]		= { 0x00, 0x90, 0x4C, 0xC5, 0x12, 0x38 };
	char zero_mac[ETHER_ADDR_LEN]		= { 0, 0, 0, 0, 0, 0 };
	char cur_macbuffer[18]	= {0};
	int ret = -1;

	g_iMacFlag = MACADDR_NONE;	

	fp_nvm = filp_open(nvfilepath, O_RDONLY, 0);
	if(IS_ERR(fp_nvm)) { // file is not exist
		//read MAC Address;
		//ReadMacAddress_OTP(dhd,cur_mac);
		strcpy(cur_mac, "cur_etheraddr");
		ret = dhd_wl_ioctl_cmd(dhdp, WLC_GET_VAR, cur_mac, sizeof(cur_mac), 0, 0);
		if (ret < 0) {
			printk("Current READ MAC error \r\n");
			memset(cur_mac , 0 , ETHER_ADDR_LEN);
			return -1;
		}
		else {
			printk("READ MAC (OTP) : %02X:%02X:%02X:%02X:%02X:%02X\n" , 
			cur_mac[0], cur_mac[1], cur_mac[2], cur_mac[3], cur_mac[4], cur_mac[5]);
		}

		fp_mac = filp_open(filepath, O_RDONLY, 0);
		if(IS_ERR(fp_mac)) { // file is not exist
		
			if(memcmp(cur_mac,dummy_mac,ETHER_ADDR_LEN) == 0) { // read mac is 00:90:4C:C5:12:38
				/* Try reading out from CIS */
				cis_rw_t *cish = (cis_rw_t *)&cur_mac[8];

				cish->source = 0;
				cish->byteoff = 0;
				cish->nbytes = sizeof(cur_mac);

				strcpy(cur_mac, "cisdump");
				ret = dhd_wl_ioctl_cmd(dhdp, WLC_GET_VAR, cur_mac, sizeof(cur_mac), 0, 0);
				if (ret < 0) {
					printk("%s: CIS reading failed, err=%d\n", __FUNCTION__, ret);
				} else {
					/* get MAC addr from CIS */
					memcpy(mac, &cur_mac[0x19], ETHER_ADDR_LEN);
					
					if (memcmp(mac, zero_mac, ETHER_ADDR_LEN) == 0) {
						// Invalid CIS MAC
				g_iMacFlag = MACADDR_MOD_RANDOM;
					} else {
						/* Valid MAC Addr */
						printk("READ MAC (CIS) : %02X:%02X:%02X:%02X:%02X:%02X\n" , 
							mac->octet[0], mac->octet[1], mac->octet[2], 
							mac->octet[3], mac->octet[4], mac->octet[5]); 
						if (0 == _dhd_set_mac_address(dhd, 0, mac)) {
							DHD_INFO(("%d: MACID is overwritten\n", __FUNCTION__));
							g_iMacFlag = MACADDR_MOD;
						}
						else {
							DHD_ERROR(("%s: _dhd_set_mac_address() failed\n", __FUNCTION__));
							g_iMacFlag = MACADDR_NONE;
						}
					}
				}
			}
			else if(strncmp(buf , "00:00:00:00:00:00" , 17) == 0) {
				g_iMacFlag = MACADDR_MOD_RANDOM;
			}
			else {
				g_iMacFlag = MACADDR_MOD;
			}
		}
		else {
			ret = kernel_read(fp_mac, 0, buf, 18);
			buf[17] ='\0';
			printk("Read MAC (FIL) : %s\n" , buf);
			sscanf(buf,"%02X:%02X:%02X:%02X:%02X:%02X",
			   &(mac->octet[0]), &(mac->octet[1]), &(mac->octet[2]), 
			   &(mac->octet[3]), &(mac->octet[4]), &(mac->octet[5]));
			if(memcmp(cur_mac,mac->octet,ETHER_ADDR_LEN) == 0) { // read mac is same
				g_iMacFlag = MACADDR_NONE;
			}
			else { // change mac..
				if (0 == _dhd_set_mac_address(dhd, 0, mac)) {
					DHD_INFO(("dhd_bus_start: MACID is overwritten\n"));
					g_iMacFlag = MACADDR_MOD;
				}
				else {
					DHD_ERROR(("dhd_bus_start: _dhd_set_mac_address() failed\n"));
					g_iMacFlag = MACADDR_NONE;
				}
			}
			filp_close(fp_mac, NULL);
		}
	}
	else {
		// COB type. only COB.
		/* Reading the MAC Address from .nvmac.info file( the existed file or just created file)*/
		ret = kernel_read(fp_nvm, 0, buf, 18);
		buf[17] ='\0';   // to prevent abnormal string display when mac address is displayed on the screen. 
		printk("Read MAC : [%s] [%d] \r\n" , buf, strncmp(buf , "00:00:00:00:00:00" , 17));
		if(strncmp(buf , "00:00:00:00:00:00" , 17) == 0) {
			filp_close(fp_nvm, NULL);
			g_iMacFlag = MACADDR_COB_RANDOM;
		}
		else {
			sscanf(buf,"%02X:%02X:%02X:%02X:%02X:%02X",
			   mac->octet[0], mac->octet[1], mac->octet[2], 
			   mac->octet[3], mac->octet[4], mac->octet[5]);
			/* Writing Newly generated MAC ID to the Dongle */
			if (0 == _dhd_set_mac_address(dhd, 0, mac)) {
				DHD_INFO(("dhd_bus_start: MACID is overwritten\n"));
				g_iMacFlag = MACADDR_COB;
			}
			else {
				DHD_ERROR(("dhd_bus_start: _dhd_set_mac_address() failed\n"));
			}
		}
		filp_close(fp_nvm, NULL);
	}
	
	if((g_iMacFlag == MACADDR_COB_RANDOM) || (g_iMacFlag == MACADDR_MOD_RANDOM)) {
		get_random_bytes(randommac, 3);
		sprintf(macbuffer,"%02X:%02X:%02X:%02X:%02X:%02X\n",
				0x60,0xd0,0xa9,randommac[0],randommac[1],randommac[2]);		
		printk("[WIFI] The Random Generated MAC ID : %s\n", macbuffer);
		sscanf(macbuffer,"%02X:%02X:%02X:%02X:%02X:%02X",
			   &(mac->octet[0]), &(mac->octet[1]), &(mac->octet[2]), 
			   &(mac->octet[3]), &(mac->octet[4]), &(mac->octet[5]));
		if (0 == _dhd_set_mac_address(dhd, 0, mac)) {
			DHD_INFO(("dhd_bus_start: MACID is overwritten\n"));
			g_iMacFlag = MACADDR_COB;
		}
		else {
			DHD_ERROR(("dhd_bus_start: _dhd_set_mac_address() failed\n"));
		}
	}

	
	return 0;
}


#endif

