#define SR130PC10_COMPLETE
//#undef SR130PC10_COMPLETE
/*
 * Driver for sr130pc10 (VGA camera) from Samsung Electronics
 * 
 * 1/6" 1.3Mp CMOS Image Sensor SoC with an Embedded Image Processor
 *
 * Copyright (C) 2010, Sungkoo Lee <skoo0.lee@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
#ifndef __SR130PC10_H__
#define __SR130PC10_H__

struct sr130pc10_reg {
	unsigned int addr;
	unsigned int val;
};

struct sr130pc10_regset_type {
	unsigned int *regset;
	int len;
};

/*
 * Macro
 */
#define REGSET_LENGTH(x)	(sizeof(x)/sizeof(sr130pc10_reg))

/*
 * User defined commands
 */
/* S/W defined features for tune */
#define REG_DELAY	0xFF	/* in ms */
#define REG_CMD		0xFFFF	/* Followed by command */

/* Following order should not be changed */
enum image_size_sr130pc10 {
	/* This SoC supports upto SXGA (1280*1024) */
#if 0
	QQVGA,	/* 160*120*/
	QCIF,	/* 176*144 */
	QVGA,	/* 320*240 */
	CIF,	/* 352*288 */
	VGA,	/* 640*480 */
#endif
	SVGA,	/* 800*600 */
#if 0
	HD720P,	/* 1280*720 */
	SXGA,	/* 1280*1024 */
	UXGA,	/* 1600*1200 */
#endif
};

/*
 * Following values describe controls of camera
 * in user aspect and must be match with index of sr130pc10_regset[]
 * These values indicates each controls and should be used
 * to control each control
 */
enum sr130pc10_control {
	sr130pc10_INIT,
	sr130pc10_EV,
	sr130pc10_AWB,
	sr130pc10_MWB,
	sr130pc10_EFFECT,
	sr130pc10_CONTRAST,
	sr130pc10_SATURATION,
	sr130pc10_SHARPNESS,
};

#define SR130PC10_REGSET(x)	{	\
	.regset = x,			\
	.len = sizeof(x)/sizeof(sr130pc10_reg),}

/*
 * SXGA Self shot init setting
 */
 
static unsigned short sr130pc10_init_reg[] = {
0x01f1,  //sleep
0x01f3,  //s/w reset  
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

//PAGE0	///////////////////////////////////////////
0x0300,
0x0e03,  //PLL X 2.0
0x0e73,  //PLL X 2.0 enable
0x100c,  //Vsync2 | Normal full size
0x1191,	//y Flip
0x1200,  //Sync type default:0x00

//windowing 1280*960
0x2000,  //row start H
0x212A,  //row start L 0x2a(42)
0x2200,  //col start H
0x230c,  //col start L 0x0c(12)
0x2403,  //win height H
0x25c0,  //win height L 0x3c0(960)
0x2605,  //win width H
0x2700,  //win width L  0x500(1280)

0x3802,  //pre3 row start
0x390A,  //pre3 col start
0x3A01,  //pre3 height_h
0x3B55,  //pre3 height_l
0x3C05,  //pre3 width_h
0x3D00,  //pre3 width_l

0x4001, //Hblank 280
0x4118, 
0x4200, //Vblank 20
0x4314, 

//BLC
0x8008,  //100408 bit[6] adeptive blc   bit[3] BLC on/off
0x8197,  //100408 
0x8290,  //100408  
0x8330,  
0x84cc,  //100408   
0x8500,  
0x86d4,  //100408

0x900f, //BLC_TIME_TH_ON
0x910f, //BLC_TIME_TH_OFF 
0x9297, //BLC_AG_TH_ON
0x938f, //BLC_AG_TH_OFF
0x9495,  //091202
0x9590,  //091202 
0x9838,  //20

//Dark BLC
0xa001,  //20100309  Blue sign[6] offset[5:0]
0xa201,  //20100309  Gb
0xa401,  //20100309  Red
0xa601,  //20100309  Gr

//Normal BLC
0xa841, //Blue
0xaa41, //Gb
0xac41, //Red
0xae41, //Gr

//Out  BLC
0x9943, //Red
0x9a43, //Gr
0x9b43, //Gb
0x9c43, //Blue

// PAGE2  ///////////////////////////////////////////////
0x0302,
0x184c,  //0x2c->0x4c 2010.8.25 don't touch update
0x1900,  //c0,  //pclk delay 00->0xc0
0x1a39,  //0x9 ->0x39 2010.8.25  
0x1c3c,
0x1d01,
0x1e30,
0x1f10,

0x2077,
0x21ed,  //2010.8.25
0x22a7,  
0x2332,  //0x32->0x30 2010.8.25
0x2447,  //


0x2734,
0x2B80,
0x2E11,
0x2FA1,

0x3000,
0x3199,
0x3200,
0x3300,
0x3422,

0x3601,  //2010.8.25 preview2 mode support
0x3701,

0x3D03,
0x3e0d,  //0xb->0xd 2010.8.25
0x49d1,

0x5028,
0x5201,	//2010.8.25
0x5381,  //2010.8.25 preview2 mode suppot
0x543C,  
0x551c,
0x5611,
0x5da2,
0x5E5a,

0x606c,
0x6177,
0x626c,
0x6375,
0x646c,
0x6575,
0x670c,
0x680c,
0x690c,
0x726d,
0x7374,
0x746d,
0x7574,
0x766d,
0x7774,
0x7C6b,
0x7D84,
0x8001,
0x8164,
0x821e,
0x832c,
0x8463,
0x8565,
0x8663,
0x8765,
0x9241,
0x934e,
0x9463,
0x9565,
0x9663,
0x9765,
0xA002,
0xA161,
0xA202,
0xA361,
0xA461,
0xA502,
0xA661,
0xA702,
0xA86c,
0xA970,
0xAA6c,
0xAB70,
0xAC1c,
0xAD22,
0xAE1c,
0xAF22,

0xB077,
0xB180,
0xB277,
0xB380,
0xB478,
0xB57f,
0xB678,
0xB77f,
0xB878,
0xB97e,
0xBA78,
0xBB7e,
0xBC79,
0xBD7d,
0xBE79,
0xBF7d,

0xc42e,
0xc53f,
0xc650,
0xc760,
0xc82f,
0xc93e,
0xca2f,
0xcb3e,
0xcc51,
0xcd5f,
0xce51,
0xcf5f,

0xd00a,
0xd109,
0xd220,
0xd300,
0xd40f,  //DCDC_TIME_TH_ON
0xd50f,  //DCDC_TIME_TH_OFF
0xd697,  //DCDC_AG_TH_ON
0xd78f,  //DCDC_AG_TH_OFF

0xe0e1,
0xe1e1,
0xe2e1,
0xe3e1,
0xe4e1,
0xe501,

0xe900,  // preview2 mode support 2010.8.25 
0xea64,

// PAGE 10  /////////////////////////////////////
0x0310,
0x1003,  //Ycbcr422_bit Order: YUYV
0x1210,  //y offset[4], dif_offset[5]
0x1302,  //contrast effet enable : 0x02
0x3400,  //hidden 10->00 100209
0x3f04,
0x4080,  //Y offset
0x4886,  //Contrast (Y = constrast * (Y - 128) + 128)	
0x5300,  //dif_offset option
0x5530,  //dif_offset option  diff_offset max

0x6083,  //out color sat en[7] | auto color decrement en[1] | manual color sat en[0]
0x6100,  //sat ctrl2

0x62c0, //blue saturation
0x63b0, //red saturation

0x64ff, //auto decresment on AG th
0x65ff, //auto decresment on DG th
0x66e4, //Outdoor saturation step 137fps apply out th
0x6744, //Outdoor saturation B/R 
0x8000,
0x8100,
0x8200,
0x8300,
0x8400,
0x8500,
0x8600,
0x8700,
0x8800,
0x8900,
0x8a00,
0x8b00,
0xb090, //read only outdoor sat red ?
0xb197, //read only outdoor sat blue?

// PAGE 11  D_LPF, Etc.. ////////////////////////////////////////
0x0311,
0x103f, 	//B[6]:Blue En  Dlpf on[4:0] Sky over off : 0x7f->3f 
0x1120, 	// Uniform Full GbGr/OV-Nr

0x1280, 	//Blue MaxOpt  blue sky max filter optoin rate : 0 0xc0->80
0x13b8,  //dark2[7] | dark2 maxfilter ratio[6:4] | dark3[3] | dark3 maxfilter ratio[2:0] 

0x30ba,  //Outdoor2 H th
0x3110,  //Outdoor2 L th
0x3240,  //Outdoor2 gain ratio
0x331a,  //Outdoor2 H lum
0x3415,  //Outdoor2 M lum
0x351d,  //Outdoor2 L lum
  
0x36b0,  //Outdoor1 H th
0x3718,  //Outdoor1 L th
0x3840,  //Outdoor1 gain ratio  0x80->40 
0x391a,  //Outdoor1 H lum       0x28->1e  
0x3a15,  //Outdoor1 M lum       0x10->15
0x3b1d,  //Outdoor1 L lum       0x08->20 

0x3C4A,    //indoor H th
0x3D16,	//indoor L th
0x3e44,	//indoor gain ratio    0x20->0x44
0x3F12,	//indoor H lum         0x28->0x18
0x4018,	//indoor M lum
0x4118,	//indoor L lum

0x4298,	//dark1 H th
0x4328,	//dark1 L th
0x4450,	//dark1 gain ratio
0x4516,	//dark1 H lum         0x38->0x28  
0x4630,	//dark1 M lum         0x27->0x17
0x472e,	//dark1 L lum         0x20->0x1a 

0x4890,	//dark2 H th
0x492a,	//dark2 L th
0x4a50,	//dark2 gain ratio
0x4b20,	//dark2 H lum 
0x4c32,	//dark2 M lum 
0x4d2f,	//dark2 L lum  

0x4e80,	//dark3 H th
0x4f30,	//dark3 L th
0x5050,	//dark3 gain ratio
0x5120,	//dark3 H lum 
0x5231,	//dark3 M lum 
0x532e,	//dark3 L lum 

0x5a3f,  //blue sky mode out1/2 enable  0x27->3f 
0x5b3f,  //Impulse pixel enable dark123,in,out123
0x5C9f,  //Indoor maxfilter rate[7:5] | Uncertain onoff[4:0] 0x1f ->0x9f

0x603f,	//GbGr all enable 
0x620f,	//GbGr offset
0x6325,	//GbGr max
0x6410,	//GbGr min

0x650c, 	//Outdoor GbGr rate H 100% M 25% L 100%
0x660c,	//Indoor GbGr  rate H 100% M 25% L 100%
0x6700,	//dark GbGr    rate H/M/L  100%

0x700c,  // Abberation On/Off B[1]: Outdoor B[0]: Indoor 07>>c
0x75A0,  // Outdoor2 Abberation Luminance lvl 
0x7DB4,  // Indoor Abberation Luminance lvl

0x9608,  //indoor/Dark1 edgeoffset1
0x9714,  //indoor/Dark1 center G value
0x98F5,  //slope indoor :: left/right graph polarity, slope
0x992a,  //indoor uncertain ratio control
0x9a20,  //Edgeoffset_dark

// PAGE 12  YC_LPF ////////////////////////////////////////////////
0x0312,
0x2017,  //Yc2d ctrl1 Lpf status apply  0x13->0x17 
0x210f,  //Yc2d ctrl2
0x2206,  //Yc2d ctrl3 C filter mask outdoor1 | Indoor
0x2300,  
0x2459,  
0x2500,  //2010.8.25 bit[5] off -> sharpness pre block off
0x2a01,
0x2e00,  //2010.8.25 0x00

//region th
0x3035,  //Texture region(most detail)
0x31a0,  //STD uniform1 most blur region
0x32b0,  //STD uniform2      2nd blur
0x33c0,  //STD uniform3      3rd blur
0x34d0,  //STD normal noise1 4th blur
0x35e0,  //STD normal noise2 5th blur
0x36ff,  //STD normal noise3 6th blur

//add for reducing the color bluring
0x3b06,
0x3c06,

0x40a0,  //Outdoor2 H th
0x4110,  //Outdoor2 L th	
0x4210,  //Outdoor2 H luminance 
0x4310,  //Outdoor2 M luminance 
0x4411,  //Outdoor2 l luminance 
0x4540,  //Outdoor2 ratio

0x46a0,  //Outdoor1 H th
0x4720,  //Outdoor1 L th	
0x4810,  //Outdoor1 H luminance 
0x4910,  //Outdoor1 M luminance
0x4a11,  //Outdoor1 L luminance
0x4b40,  //Outdoor1 ratio

0x4c80,  //Indoor H th	
0x4d48,  //Indoor L th
0x4e11,  //indoor H lum	
0x4f12,  //indoor M lum
0x5012,  //indoor L lum 
0x5145,  //indoor ratio  0x10 -> 0x45

0x52a8,  //dark1 H th	
0x5330,  //dark1 L th
0x5428,  //dark1 H lum 
0x5530,  //dark1 M lum
0x5630,  //dark1 L lum
0x5750,  //dark1 ratio

0x58a0,  //dark2 H th
0x59a0,  //dark2 L th
0x5a28,  //dark2 H lum
0x5b30,  //dark2 M lum
0x5c30,  //dark2 L lum
0x5d50,  //dark2 ratio

0x5ea0,  //dark3 H th
0x5f40,  //dark3 L th
0x6029,  //dark3 H lum
0x6130,  //dark3 M lum
0x6230,  //dark3 L lum
0x6350,  //dark3 ratio

//C-filter(Out2&Out1)
0x7010,
0x710a,

//C-filter(Indoor&Dark3)
0x7210,
0x730a,

//C-filter(Dark2&Dark1)
0x7418,
0x7512,

//DPC-Dark1,2,3
0xad07,
0xae07,
0xaf07,

//Blue Det..
0xC523, //BlueRange   2010.8.25    0x40->23 
0xC620, //GreenRange  2010.8.25    0x3b->20 

0xd008, //2010.8.25 

//interpolated with average
0xdb18,  // 0x00->0x18
0xd904,  //strong_edge detect ratio

// PAGE13  Sharpness 1D/2D  ////////////////////////////////////
0x0313,
0x10ab,  //Edge enb
0x117b,  //slop limit en | lclp lmt en | hclp1 en | hclp2 en
0x120e,  //sharp coef sel auto | sharp slope auto en2 | harp lclp auto en
0x1412,  //scale AG, time sharp
0x1511,  //added option

0x200a,  //global negative gain
0x2105,  //global postive gain
0x2233,  //slope limit
0x2308,  //hclip th1
0x241a,  //hclip th2
0x2506,  //indoor lclip th
0x2620,  //lclip limit
0x2710,  //lclip  fit th
0x2910,  //sharp exp limit time th
0x2a30,  //sharp pga th

//1d low clip th
0x2b06,  //out2
0x2c07,  //out1
0x2d0a,  //dark1
0x2e0c,  //dark2
0x2f10,  //dark3
  
//1D Edge
0x5004,  //out2  hi nega	
0x5303,  //      hi pos	
0x5104,  //      mi nega
0x5403,  //      mi pos
0x5204,  //      lo nega
0x5503,  //      lo pos

0x5604,  //out1   hi nega 
0x5903,  //       hi pos 
0x5704,  //       mi nega
0x5a03,  //       mi pos 
0x5804,  //       lo nega
0x5b03,  //       lo pos

0x5c03,  //indoor hi  nega
0x5f02,  //       hi  pos
0x5d02,  //       mid nega 
0x6001,  //       mid pos 
0x5e02,  //       low nega  
0x6101,  //       low pos

0x6202,  //dark1  hi nega
0x6501,  //       hi  pos   
0x6301,  //       mid nega  
0x6601,  //       mid pos   
0x6401,  //       low nega  
0x6701,  //       low pos   

0x6801,  //dark2  hi  nega
0x6b01,  //       hi  pos       
0x6901,  //       mid nega      
0x6c01,  //       mid pos       
0x6a00,  //       low nega      
0x6d00,  //       low pos       

0x6e00,  //dark3  hi  nega
0x7100,  //       hi  pos       
0x6f00,  //       mid nega      
0x7200,  //       mid pos       
0x7000,  //       low nega      
0x7300,  //       low pos       

//2DY
0x80c1,  //sharp2 slope auto en | sharp2d statu auto en | sharp2d en
0x811f,  
0x8211,  //sharp2d coef sel auto | sharp2d coef sel : nomal gaussian
0x8311,  //pos lclip_div[7:4] fit div[3:0]
0x8486,  //Lpf gaussian coef out2[7:6]out1[5:4]in[3:2]dark[1:0]
0x901a,  //global Negative gain
0x9114,  //global postive gain
0x9233,  //neg limit[7:4] pos limit[3:0]
0x9330,  //low lclip limit
0x9410,  //hclip th1
0x9520,  //hclip th2
0x9720,  //sharp2d fit th
0x9920,  //postive lclip lvl

0xa004,  //out2   lclp nega
0xa104,  //out2   lclp pos
0xa204,  //out1   lclp nega
0xa304,  //out1   lclp pos 
0xa405,  //indoor lclp nega
0xa505,  //indoor lclp pos 
0xa60a,  //dark1  lclp nega
0xa70a,  //dark1  lclp pos 
0xa80d,  //dark2  lclp nega     
0xa90d,  //dark2  lclp pos     
0xaa0f,  //dark3  lclp nega  
0xab0f,  //dark3  lclp pos  


//2d edge
0xb00a,  //out2   H Ne
0xb306,  //       H Po
0xb10a,  //       M Ne
0xb406,  //       M Po
0xb20a,  //       L Ne
0xb506,  //       L Po

0xb60d,  //out1   H Ne   0xd->18
0xb908,  //       H Po   0x8->16
0xb70d,  //       M Ne   0xd->15
0xba08,  //       M Po   0x8->17
0xb80d,  //       L Ne   0xd->13
0xbb08,  //       L Po   0x8->11

0xbc45,  //indoor H Ne
0xbf3f,  //       H Po
0xbd4b,  //       M Ne
0xc045,  //       M Po
0xbe49,  //       L Ne 
0xc13e,  //       L Po

0xc211,  //dark1  H Ne
0xc50e,  //       H Po   
0xc313,  //       M Ne   
0xc611,  //       M Po   
0xc408,  //       L Ne   
0xc706,  //       L Po   

0xc810, //dark2   H Ne
0xcb0e,  //       H Po   
0xc912,  //       M Ne   
0xcc11,  //       M Po   
0xca08,  //       L Ne   
0xcd07,  //       L Po   

0xce10, //dark3   H Ne
0xd10e,  //       H Po   
0xcf11,  //       M Ne   
0xd211,  //       M Po   
0xd008,  //       L Ne   
0xd307,  //       L Po   

//Page 0x14
0x0314,
0x1011,
0x2080, // x center gg
0x2180, // y center gg
0x2280, // x center rr
0x2380, // y center rr
0x2480, // x center bb
0x2580, // y center bb
0x2860,
0x2960,
0x4061, //83 rr
0x4143, //59 gr
0x423d, //51 bb
0x4343, //59 gb


//PAGE15
//CMC Fuction Start
0x0315,
0x1021,
0x1440,	//CMCOFSGH 
0x1534,	//CMCOFSGM
0x1628,	//CMCOFSGL
0x172f,	//CMC SIGN

//CMC
0x30a5,
0x312b,
0x3206,
0x3317,
0x3498,
0x3501,
0x3600,
0x3739,
0x38b9,
//CMC OFS
0x408b,
0x4109,
0x4201,
0x4381,
0x440e,
0x458d,
0x468b,
0x478f,
0x481b,
//CMC POFS
0x5018,
0x5192,
0x5285,
0x5382,
0x5436,
0x55b1,
0x5600,
0x5785,
0x5805,

// Digital gain
0x8000,
0x8580,


//PAGE16

0x0316,
0x1031,  //GMA_CTL
0x187e,  //AG_ON
0x197d,  //AG_OFF
0x1A0e,  //TIME_ON
0x1B01,  //TIME_OFF
0x1Cdc,  //OUT_ON
0x1Dfe,  //OUT_OFF

//GMA //Indoor
0x3000,
0x310a,
0x3222,
0x333e,
0x346e,
0x358f,
0x36a6,
0x37b6,
0x38c4,
0x39cd,
0x3ad7,
0x3bdf,
0x3ce6,
0x3dec,
0x3ef2,
0x3ff5,
0x40f9,
0x41fd,
0x42ff,

//RGMA //Outdoor
0x5000,
0x5103,
0x5219,
0x5334,
0x5458,
0x5575,
0x568d,
0x57a1,
0x58b2,
0x59be,
0x5ac9,
0x5bd2,
0x5cdb,
0x5de3,
0x5eeb,
0x5ff0,
0x60f5,
0x61f7,
0x62f8,

//BGMA //Dark
0x7007,
0x7118,
0x7221,
0x7338,
0x7452,
0x756c,
0x7685,
0x779a,
0x78ad,
0x79bd,
0x7acb,
0x7bd6,
0x7ce0,
0x7de8,
0x7eef,
0x7ff4,
0x80f9,
0x81fc,
0x82ff,


//PAGE17
0x0317,  
0x1005,	//en_cnt_check[4] | en_histogra,[0]
0x110a,  
0x1260,	//0x40
0xc44b,	//FLK200
0xc53e,	//FLK240

////////////////////////////// 18 Page
0x0318,
0x1000,  //scaler off
0x81e4,
0x82e4,
0x834e,
0x844e,

//PAGE20
0x0320,
0x100c,
0x111c,  
0x1a04,  //fuzzy power control 20100205
0x2001,  //AE weight en | ae lowtemp disable  0x05->0x01 
0x2118,  
0x2200,	//backlight option
0x272a,  
0x28bf,  

// AntiBand setting
0x2af0,               
0x2b34, 
0x3078, 

0x3922,
0x3ade,
0x3b22,
0x3cde,

0x2f00,
0x43c0,
0x5628,
0x5778,
0x5820,
0x5960,

0x5e9f,  // hsync inc size 1.3mega 0x9f
0x5f77,  // vsync inc size 1.3mega 960 0x77

//AE weight start
0x60aa,
0x61aa,
0x62aa,
0x63aa,
0x64aa,
0x65aa,
0x66ab,
0x67ea,
0x68ab,
0x69ea,
0x6aaa,
0x6baa,
0x6caa,
0x6daa,
0x6eaa,
0x6faa,

0x7030, //1280*960

0x7184,  //00_2*4=+8
0x7622,  //unlock bound1 middle
0x7781,  //unlock bound2 middle
0x7822,  //Y-TH1  <lockboundary,unlock1>
0x7926,
0x7a24,
0x7B22,
0x7d22,
0x7e22,

//Pll 2.0x
0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 

0x8601, //EXPMin 7500.00 fps
0x8790, 

0x8805, //EXP Max 8.00 fps 
0x89b8, 
0x8ad8, 

0x8B75, //EXP100 
0x8C30, 

0x8D61, //EXP120 
0x8Ea8, 

0x9105,
0x92e9,
0x93ac,

0x9402,
0x957a,
0x96c4,

0x989e,  //outdoor th1
0x9946,  //outdoor th2

0x9b9a,  //outdoor th1
0x9a28,  //outdoor th2

0x9c07, //EXP Limit 1500.00 fps 
0x9dd0, 

0x9e01, //EXP Unit 
0x9f90, 

0xa000,
0xa1c3,
0xa250,

//Exp12000
0xa300,  //Antibandtime/256=120*256=30720fps
0xa430,

//Capture Exptime
0xA501,  //Capture ExpmaxH
0xA66E,  //Capture ExpmaxM
0xA736,  //Capture ExpmaxL
0xA81D,  //Capture Exp100H
0xA94C,  //Capture Exp100L
0xAA18,  //Capture Exp120H //PLL 2x ³·Ãè´Ù°í °¡Á¤.
0xAB6A,  //Capture Exp120L
0xAC00,  //Capture Exp12000H
0xAD18,  //Capture Exp12000L
0xAE23,  //Capture Outtime th2

//AG set ag min 15
0xb010,  //AG
0xb110,  //AGMIN
0xb29f,  //AGMAX
0xb316,  //AGLVL //1a
0xb416,  //AGTH1 //1a
0xb53c,  //AGTH2  0x44
0xb629,  //AGBTH1 0x2f
0xb723,  //AGBTH2 0x28
0xb820,  //AGBTH3 0x25
0xb91e,  //AGBTH4 0x22
0xba1c,  //AGBTH5 0x21
0xbb1b,  //AGBTH6 0x20
0xbc1b,  //AGBTH7 0x1f
0xbd1a,  //AGBTH8 0x1f

0xbe36,  //AGBTH9 //Adaptive_AG Min
0xbf36,  //AGBTH10
0xc035,  //AGBTH11
0xc135,  //AGBTH12
  
0xc230,  //Adaptive AGMIN
0xc330,  //Adaptive AGLVL
0xc430,  //20091210 2a->25
0xc50a,  //20091210 0z->1f 10/120sec
  
0xc648,  //Middle AG
0xcc48,  //Middle AG min
0xcae0,  //Middle DG
0xc718,  //Sky_gain
0xc8a0,  //Middle indoor max c8->d6
0xc980,  //Middle inddor min


//PAGE22
0x0322,
0x10ef,  //ea
0x112e,

//uniform
0x1800,  //uniform_on  0x01 
0x1a54,  //r_max
0x1b2e,  //r_min
0x1c44,  //b_max
0x1d2e,  //b_min
0x1e04,  //time_th
0x1f60,  //time_th



0x1941,  //Low temp off 0x40 -> Low temp on 0x41
0x2038,  //AWB unstable speed
0x2100,  //skin color on
0x2200,  
0x7ad6,
0x2340,
0x2407,
0x253C,
0x2600,
0x270B,  //6b
0x2866,
0x2900,  // method 1 //10
0x2A0a,
0x2B00,
0x2C00,
0x2D7A,
0x2Eff,  //68
0x2F00,  //lowtemp crcb method 0x00 old

0x3080,
0x3180,

//Pixel slope
0x3418,
0x350B,
0x362A,
0x373A,

0x3812,
0x3966,

0x40f6,

0x4602,
0x4DD4,  //d4
0x4e0a,

//stable band
0x4233,//csum2
0x5033,//csum4
0x4144,//cdiff
0x40f2,//yth
  
//unstable band
0x4544,  //csum2
0x5134,  //csum4
0x4444,  //cdiff
0x43f2,  //yth  

//dark band
0x4934,  //csum2
0x5223,  //csum4
0x4853,  //cdiff
0x47f3,  //yth  

//Out door band
0x4c34,  //csum2
0x5323,  //csum4
0x4b53,  //cdiff
0x4a84,  //yth  

//Csum band
0x5400,
0x55A5,  //sign
0x5611,  //ofs
0x5755,  //ofs

0x5AE0,
0x5BF0,
0x5C60,
0x5D14,
0x5E40,
0x5F00,

0x60F8,
0x6164,
0x6210,
0x63F8,
0x6420,
0x6522,  //D1/D3 valid color th
0x6644,  //D2/D4 valid color th
0x6714,
0x6814,
0x6914,
0x6A14,

0x6b00,
0x6c00,
0x6d40,
0x6e50,
0x6D40,
0x6E50,
0x6F30,

0x7040,
0x7150,
0x7230,
0x7388,
0x7440,
0x7550,
0x7630,
0x7788,
0xE122,

//Gain setti2ng
0x803a,
0x8120,
0x8235,

0x8354,	//Normal R max
0x8426,	//Normal R min  29->28   20->2a
0x8554,	//Normal B max  50->52
0x861f,	//Normal B mim

0x8752,	//middle R max
0x8830,	//middle R min
0x8946,	//middle B max  43->36 
0x8a1f,	//middle B min

0x8b50,	//bottom R max
0x8c3d,	//bottom R min
0x8d30,	//bottom B max
0x8e1f,	//bottom B min

// awb slop, band
0x8f55,	//p1
0x9053,	//p2
0x9150,	//p3
0x924c,	//p4	
0x9349,	//p5
0x943c,	//p6
0x9536,	//p7
0x9631,	//p8
0x9728,	//p9
0x9824,	//p10
0x991f,	//p11
0x9a1e,	//p12

0x9b77, //56 band12
0x9c77, //76 band34

0x9d4f,	//R_bandth 1
0x9e30,	//R_bandth 2
0x9f2c,	//R_bandth 3

//Outdoor limit r/b
0xa023,	//R delta 1
0xa132,	//B delta 1
0xa213,	//R delta 2
0xa321,	//B delta 2

0xa410,	//R_TH1	1920fps  0xa->0x10
0xa53a,	//R_TH2
0xa670,	//R_TH3

0xa700,	//low temp R min
0xa818,	//low temp R max
0xad38,
0xae30,
0xaf32,
0xb02c,
0xb100,  //lowtemp y control 0x28->0x00
0xb248,
0xb340,

//Low temp Tgt
0xB57f,
0xB600,
0xB701,  //low Cb,Cr target offset 0x09->0
0xB82c,  //Low temp off
0xB929,  //Low temp On
0xBC00,  //Low gain cb/cb retarget point
0xBD36,  //Low gain 80/80 point


0x0310,
0x3701,


//Resol detect control ///
0x0324,
0x604f,  //edge even frame | 16bit resol | white edge cnt | scene resol enable
0x6104,  //even frame update
0x6435,  //edge th1 H
0x6500,  //edge th1 L
0x6630,  //edge th2 H
0x6700,  //edge th2 L

0x0313,
0x7402,  //det slope en | gausian filter
0x7509,  //1D negative gain
0x7609,  //1D postive  gain
0x770c,  //1D hclp2

0xf80d,  //sharp2d resl coef_L => coef_h_en | slope en
0xf930,  //2D negative gain
0xfa30,  //2D postive  gain
0xfb1e,  //2D hclp2
0xfc37,  //2D Hi  frequence Gaussian coef
0xfd20,  //2D Hi  frequence th 
0xfe08,  //2D Low frequence th 

0x0312,  //false color reduction
0x7b10,  //Detect resolution only	
0x7c04,  //false blur offset
0x7d04,  //false blur offset limit

0x0320,
0x108c,//AE ON
0x0322,
0x10eb,//AWB ON
0x01f0,

0x0300,  //dummy1    delay_
0x0300,  //dummy2        
0x0300,  //dummy3

0xff14, //200ms delay

};

//==========================================================
//	CAMERA INITIAL for VT Preview 7 Fixed Frame (VGA SETTING)
//==========================================================

static unsigned short sr130pc10_init_vt_reg[] = {
0x01f1,  //sleep
0x01f3,  //s/w reset  
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

//PAGE0	///////////////////////////////////////////
0x0300,
0x0e03,  //PLL X 1
0x0e73,  //PLL X 2
0x100c,  //Vsync2 | Normal full size
0x1191,	//y Flip
0x1200,  //Sync type default:0x00

//windowing 1280*960
0x2000,  //row start H
0x212A,  //row start L 0x2a(42)
0x2200,  //col start H
0x230c,  //col start L 0x0c(12)
0x2403,  //win height H
0x25c0,  //win height L 0x3c0(960)
0x2605,  //win width H
0x2700,  //win width L  0x500(1280)

0x3802,  //pre3 row start
0x390A,  //pre3 col start
0x3A01,  //pre3 height_h
0x3B55,  //pre3 height_l
0x3C05,  //pre3 width_h
0x3D00,  //pre3 width_l

0x4002, //Hblank 680
0x41a8, 
0x4200, //Vblank 20
0x4314, 

//BLC
0x8008,  //100408 bit[6] adeptive blc   bit[3] BLC on/off
0x8197,  //100408 
0x8290,  //100408  
0x8330,  
0x84cc,  //100408   
0x8500,  
0x86d4,  //100408

0x900f, //BLC_TIME_TH_ON
0x910f, //BLC_TIME_TH_OFF 
0x9297, //BLC_AG_TH_ON
0x938f, //BLC_AG_TH_OFF
0x9495,  //091202
0x9590,  //091202 
0x9838,  //20

//Dark BLC
0xa001,  //20100309  Blue sign[6] offset[5:0]
0xa201,  //20100309  Gb
0xa401,  //20100309  Red
0xa601,  //20100309  Gr

//Normal BLC
0xa841, //Blue
0xaa41, //Gb
0xac41, //Red
0xae41, //Gr

//Out  BLC
0x9943, //Red
0x9a43, //Gr
0x9b43, //Gb
0x9c43, //Blue

// PAGE2  ///////////////////////////////////////////////
0x0302,
0x184c,  //0x2c->0x4c 2010.8.25 don't touch update
0x1900,  //c0,  //pclk delay 00->0xc0
0x1a39,  //0x9 ->0x39 2010.8.25  
0x1c3c,
0x1d01,
0x1e30,
0x1f10,

0x2077,
0x21ed,  //2010.8.25
0x22a7,  
0x2332,  //0x32->0x30 2010.8.25
0x2447,  //


0x2734,
0x2B80,
0x2E11,
0x2FA1,

0x3000,
0x3199,
0x3200,
0x3300,
0x3422,

0x3601,  //2010.8.25 preview2 mode support
0x3701,

0x3D03,
0x3e0d,  //0xb->0xd 2010.8.25
0x49d1,

0x5028,
0x5201,	//2010.8.25
0x5381,  //2010.8.25 preview2 mode suppot
0x543C,  
0x551c,
0x5611,
0x5da2,
0x5E5a,

0x606c,
0x6177,
0x626c,
0x6375,
0x646c,
0x6575,
0x670c,
0x680c,
0x690c,
0x726d,
0x7374,
0x746d,
0x7574,
0x766d,
0x7774,
0x7C6b,
0x7D84,
0x8001,
0x8164,
0x821e,
0x832c,
0x8463,
0x8565,
0x8663,
0x8765,
0x9241,
0x934e,
0x9463,
0x9565,
0x9663,
0x9765,
0xA002,
0xA161,
0xA202,
0xA361,
0xA461,
0xA502,
0xA661,
0xA702,
0xA86c,
0xA970,
0xAA6c,
0xAB70,
0xAC1c,
0xAD22,
0xAE1c,
0xAF22,

0xB077,
0xB180,
0xB277,
0xB380,
0xB478,
0xB57f,
0xB678,
0xB77f,
0xB878,
0xB97e,
0xBA78,
0xBB7e,
0xBC79,
0xBD7d,
0xBE79,
0xBF7d,

0xc42e,
0xc53f,
0xc650,
0xc760,
0xc82f,
0xc93e,
0xca2f,
0xcb3e,
0xcc51,
0xcd5f,
0xce51,
0xcf5f,

0xd00a,
0xd109,
0xd220,
0xd300,
0xd40f,  //DCDC_TIME_TH_ON
0xd50f,  //DCDC_TIME_TH_OFF
0xd697,  //DCDC_AG_TH_ON
0xd78f,  //DCDC_AG_TH_OFF

0xe0e1,
0xe1e1,
0xe2e1,
0xe3e1,
0xe4e1,
0xe501,

0xe900,  // preview2 mode support 2010.8.25 
0xea64,

// PAGE 10  /////////////////////////////////////
0x0310,
0x1003,  //Ycbcr422_bit Order: YUYV
0x1230,  //y offset[4], dif_offset[5]
0x1302,  //contrast effet enable : 0x02
0x3400,  //hidden 10->00 100209
0x3f04,
0x4080,  //Y offset
0x4886,  //Contrast (Y = constrast * (Y - 128) + 128)	
0x5350,  //dif_offset option
0x552d,  //dif_offset option  diff_offset max

0x6083,  //out color sat en[7] | auto color decrement en[1] | manual color sat en[0]
0x6100,  //sat ctrl2

0x62c0, //blue saturation
0x63b0, //red saturation

0x64ff, //auto decresment on AG th
0x65ff, //auto decresment on DG th
0x66e4, //Outdoor saturation step 137fps apply out th
0x6744, //Outdoor saturation B/R 
0x8000,
0x8100,
0x8200,
0x8300,
0x8400,
0x8500,
0x8600,
0x8700,
0x8800,
0x8900,
0x8a00,
0x8b00,
0xb090, //read only outdoor sat red ?
0xb197, //read only outdoor sat blue?

// PAGE 11  D_LPF, Etc.. ////////////////////////////////////////
0x0311,
0x103f, 	//B[6]:Blue En  Dlpf on[4:0] Sky over off : 0x7f->3f 
0x1120, 	// Uniform Full GbGr/OV-Nr

0x1280, 	//Blue MaxOpt  blue sky max filter optoin rate : 0 0xc0->80
0x13b8,  //dark2[7] | dark2 maxfilter ratio[6:4] | dark3[3] | dark3 maxfilter ratio[2:0] 

0x30ba,  //Outdoor2 H th
0x3110,  //Outdoor2 L th
0x3240,  //Outdoor2 gain ratio
0x331a,  //Outdoor2 H lum
0x3415,  //Outdoor2 M lum
0x351d,  //Outdoor2 L lum
  
0x36b0,  //Outdoor1 H th
0x3718,  //Outdoor1 L th
0x3840,  //Outdoor1 gain ratio  0x80->40 
0x391a,  //Outdoor1 H lum       0x28->1e  
0x3a15,  //Outdoor1 M lum       0x10->15
0x3b1d,  //Outdoor1 L lum       0x08->20 

0x3C4A,    //indoor H th
0x3D16,	//indoor L th
0x3e44,	//indoor gain ratio    0x20->0x44
0x3F12,	//indoor H lum         0x28->0x18
0x4018,	//indoor M lum
0x4118,	//indoor L lum

0x4298,	//dark1 H th
0x4328,	//dark1 L th
0x4450,	//dark1 gain ratio
0x4516,	//dark1 H lum         0x38->0x28  
0x4630,	//dark1 M lum         0x27->0x17
0x472e,	//dark1 L lum         0x20->0x1a 

0x4890,	//dark2 H th
0x492a,	//dark2 L th
0x4a50,	//dark2 gain ratio
0x4b20,	//dark2 H lum 
0x4c32,	//dark2 M lum 
0x4d2f,	//dark2 L lum  

0x4e80,	//dark3 H th
0x4f30,	//dark3 L th
0x5050,	//dark3 gain ratio
0x5120,	//dark3 H lum 
0x5231,	//dark3 M lum 
0x532e,	//dark3 L lum 

0x5a3f,  //blue sky mode out1/2 enable  0x27->3f 
0x5b3f,  //Impulse pixel enable dark123,in,out123
0x5C9f,  //Indoor maxfilter rate[7:5] | Uncertain onoff[4:0] 0x1f ->0x9f

0x603f,	//GbGr all enable 
0x620f,	//GbGr offset
0x6325,	//GbGr max
0x6410,	//GbGr min

0x650c, 	//Outdoor GbGr rate H 100% M 25% L 100%
0x660c,	//Indoor GbGr  rate H 100% M 25% L 100%
0x6700,	//dark GbGr    rate H/M/L  100%

0x700c,  // Abberation On/Off B[1]: Outdoor B[0]: Indoor 07>>c
0x75A0,  // Outdoor2 Abberation Luminance lvl 
0x7DB4,  // Indoor Abberation Luminance lvl

0x9608,  //indoor/Dark1 edgeoffset1
0x9714,  //indoor/Dark1 center G value
0x98F5,  //slope indoor :: left/right graph polarity, slope
0x992a,  //indoor uncertain ratio control
0x9a20,  //Edgeoffset_dark

// PAGE 12  YC_LPF ////////////////////////////////////////////////
0x0312,
0x2017,  //Yc2d ctrl1 Lpf status apply  0x13->0x17 
0x210f,  //Yc2d ctrl2
0x2206,  //Yc2d ctrl3 C filter mask outdoor1 | Indoor
0x2300,  
0x2459,  
0x2500,  //2010.8.25 bit[5] off -> sharpness pre block off
0x2a01,
0x2e00,  //2010.8.25 0x00

//region th
0x3035,  //Texture region(most detail)
0x31a0,  //STD uniform1 most blur region
0x32b0,  //STD uniform2      2nd blur
0x33c0,  //STD uniform3      3rd blur
0x34d0,  //STD normal noise1 4th blur
0x35e0,  //STD normal noise2 5th blur
0x36ff,  //STD normal noise3 6th blur

//add for reducing the color bluring
0x3b06,
0x3c06,

0x40a0,  //Outdoor2 H th
0x4110,  //Outdoor2 L th	
0x4210,  //Outdoor2 H luminance 
0x4310,  //Outdoor2 M luminance 
0x4411,  //Outdoor2 l luminance 
0x4540,  //Outdoor2 ratio

0x46a0,  //Outdoor1 H th
0x4720,  //Outdoor1 L th	
0x4810,  //Outdoor1 H luminance 
0x4910,  //Outdoor1 M luminance
0x4a11,  //Outdoor1 L luminance
0x4b40,  //Outdoor1 ratio

0x4c80,  //Indoor H th	
0x4d48,  //Indoor L th
0x4e11,  //indoor H lum	
0x4f12,  //indoor M lum
0x5012,  //indoor L lum 
0x5145,  //indoor ratio  0x10 -> 0x45

0x52a8,  //dark1 H th	
0x5330,  //dark1 L th
0x5428,  //dark1 H lum 
0x5530,  //dark1 M lum
0x5630,  //dark1 L lum
0x5750,  //dark1 ratio

0x58a0,  //dark2 H th
0x59a0,  //dark2 L th
0x5a28,  //dark2 H lum
0x5b30,  //dark2 M lum
0x5c30,  //dark2 L lum
0x5d50,  //dark2 ratio

0x5ea0,  //dark3 H th
0x5f40,  //dark3 L th
0x6029,  //dark3 H lum
0x6130,  //dark3 M lum
0x6230,  //dark3 L lum
0x6350,  //dark3 ratio

//C-filter(Out2&Out1)
0x7010,
0x710a,

//C-filter(Indoor&Dark3)
0x7210,
0x730a,

//C-filter(Dark2&Dark1)
0x7418,
0x7512,

//DPC-Dark1,2,3
0xad07,
0xae07,
0xaf07,

//Blue Det..
0xC523, //BlueRange   2010.8.25    0x40->23 
0xC620, //GreenRange  2010.8.25    0x3b->20 

0xd008, //2010.8.25 

//interpolated with average
0xdb18,  // 0x00->0x18
0xd904,  //strong_edge detect ratio

// PAGE13  Sharpness 1D/2D  ////////////////////////////////////
0x0313,
0x10ab,  //Edge enb
0x117b,  //slop limit en | lclp lmt en | hclp1 en | hclp2 en
0x120e,  //sharp coef sel auto | sharp slope auto en2 | harp lclp auto en
0x1412,  //scale AG, time sharp
0x1511,  //added option

0x200a,  //global negative gain
0x2105,  //global postive gain
0x2233,  //slope limit
0x2308,  //hclip th1
0x241a,  //hclip th2
0x2506,  //indoor lclip th
0x2620,  //lclip limit
0x2710,  //lclip  fit th
0x2910,  //sharp exp limit time th
0x2a30,  //sharp pga th

//1d low clip th
0x2b06,  //out2
0x2c07,  //out1
0x2d0a,  //dark1
0x2e0c,  //dark2
0x2f10,  //dark3
  
//1D Edge
0x5004,  //out2  hi nega	
0x5303,  //      hi pos	
0x5104,  //      mi nega
0x5403,  //      mi pos
0x5204,  //      lo nega
0x5503,  //      lo pos

0x5604,  //out1   hi nega 
0x5903,  //       hi pos 
0x5704,  //       mi nega
0x5a03,  //       mi pos 
0x5804,  //       lo nega
0x5b03,  //       lo pos

0x5c03,  //indoor hi  nega
0x5f02,  //       hi  pos
0x5d02,  //       mid nega 
0x6001,  //       mid pos 
0x5e02,  //       low nega  
0x6101,  //       low pos

0x6202,  //dark1  hi nega
0x6501,  //       hi  pos   
0x6301,  //       mid nega  
0x6601,  //       mid pos   
0x6401,  //       low nega  
0x6701,  //       low pos   

0x6801,  //dark2  hi  nega
0x6b01,  //       hi  pos       
0x6901,  //       mid nega      
0x6c01,  //       mid pos       
0x6a00,  //       low nega      
0x6d00,  //       low pos       

0x6e00,  //dark3  hi  nega
0x7100,  //       hi  pos       
0x6f00,  //       mid nega      
0x7200,  //       mid pos       
0x7000,  //       low nega      
0x7300,  //       low pos       

//2DY
0x80c1,  //sharp2 slope auto en | sharp2d statu auto en | sharp2d en
0x811f,  
0x8211,  //sharp2d coef sel auto | sharp2d coef sel : nomal gaussian
0x8311,  //pos lclip_div[7:4] fit div[3:0]
0x8486,  //Lpf gaussian coef out2[7:6]out1[5:4]in[3:2]dark[1:0]
0x901a,  //global Negative gain
0x9114,  //global postive gain
0x9233,  //neg limit[7:4] pos limit[3:0]
0x9330,  //low lclip limit
0x9410,  //hclip th1
0x9520,  //hclip th2
0x9720,  //sharp2d fit th
0x9920,  //postive lclip lvl

0xa004,  //out2   lclp nega
0xa104,  //out2   lclp pos
0xa204,  //out1   lclp nega
0xa304,  //out1   lclp pos 
0xa405,  //indoor lclp nega
0xa505,  //indoor lclp pos 
0xa60a,  //dark1  lclp nega
0xa70a,  //dark1  lclp pos 
0xa80d,  //dark2  lclp nega     
0xa90d,  //dark2  lclp pos     
0xaa0f,  //dark3  lclp nega  
0xab0f,  //dark3  lclp pos  


//2d edge
0xb00a,  //out2   H Ne
0xb306,  //       H Po
0xb10a,  //       M Ne
0xb406,  //       M Po
0xb20a,  //       L Ne
0xb506,  //       L Po

0xb60d,  //out1   H Ne   0xd->18
0xb908,  //       H Po   0x8->16
0xb70d,  //       M Ne   0xd->15
0xba08,  //       M Po   0x8->17
0xb80d,  //       L Ne   0xd->13
0xbb08,  //       L Po   0x8->11

0xbc45,  //indoor H Ne
0xbf3f,  //       H Po
0xbd4b,  //       M Ne
0xc045,  //       M Po
0xbe49,  //       L Ne 
0xc13e,  //       L Po

0xc211,  //dark1  H Ne
0xc50e,  //       H Po   
0xc313,  //       M Ne   
0xc611,  //       M Po   
0xc408,  //       L Ne   
0xc706,  //       L Po   

0xc810, //dark2   H Ne
0xcb0e,  //       H Po   
0xc912,  //       M Ne   
0xcc11,  //       M Po   
0xca08,  //       L Ne   
0xcd07,  //       L Po   

0xce10, //dark3   H Ne
0xd10e,  //       H Po   
0xcf11,  //       M Ne   
0xd211,  //       M Po   
0xd008,  //       L Ne   
0xd307,  //       L Po   

//Page 0x14
0x0314,
0x1011,
0x2080, // x center gg
0x2180, // y center gg
0x2280, // x center rr
0x2380, // y center rr
0x2480, // x center bb
0x2580, // y center bb
0x2860,
0x2960,
0x4061, //83 rr
0x4143, //59 gr
0x423d, //51 bb
0x4343, //59 gb


//PAGE15
//CMC Fuction Start
0x0315,
0x1021,
0x1440,	//CMCOFSGH 
0x1534,	//CMCOFSGM
0x1628,	//CMCOFSGL
0x172f,	//CMC SIGN

//CMC
0x30a5,
0x312b,
0x3206,
0x3317,
0x3498,
0x3501,
0x3600,
0x3739,
0x38b9,
//CMC OFS
0x408b,
0x4109,
0x4201,
0x4381,
0x440e,
0x458d,
0x468b,
0x478f,
0x481b,
//CMC POFS
0x5018,
0x5192,
0x5285,
0x5382,
0x5436,
0x55b1,
0x5600,
0x5785,
0x5805,

// Digital gain
0x8000,
0x8580,


//PAGE16

0x0316,
0x1031,  //GMA_CTL
0x187e,  //AG_ON
0x197d,  //AG_OFF
0x1A0e,  //TIME_ON
0x1B01,  //TIME_OFF
0x1Cdc,  //OUT_ON
0x1Dfe,  //OUT_OFF

//GMA //Indoor
0x3000,
0x310a,
0x3222,
0x333e,
0x346e,
0x358f,
0x36a6,
0x37b6,
0x38c4,
0x39cd,
0x3ad7,
0x3bdf,
0x3ce6,
0x3dec,
0x3ef2,
0x3ff5,
0x40f9,
0x41fd,
0x42ff,

//RGMA //Outdoor
0x5000,
0x5103,
0x5219,
0x5334,
0x5458,
0x5575,
0x568d,
0x57a1,
0x58b2,
0x59be,
0x5ac9,
0x5bd2,
0x5cdb,
0x5de3,
0x5eeb,
0x5ff0,
0x60f5,
0x61f7,
0x62f8,

//BGMA //Dark
0x7007,
0x7118,
0x7221,
0x7338,
0x7452,
0x756c,
0x7685,
0x779a,
0x78ad,
0x79bd,
0x7acb,
0x7bd6,
0x7ce0,
0x7de8,
0x7eef,
0x7ff4,
0x80f9,
0x81fc,
0x82ff,


//PAGE17
0x0317,  
0x1005,	//en_cnt_check[4] | en_histogra,[0]
0x110a,  
0x1260,	//0x40
0xc44b,	//FLK200
0xc53e,	//FLK240


0x81e4,
0x82e4,
0x834e,
0x844e,

//PAGE20
0x0320,
0x100c,
0x111c,  
0x1a04,  //fuzzy power control 20100205
0x2001,  //AE weight en | ae lowtemp disable  0x05->0x01 
0x2118,  
0x2200,	//backlight option
0x272a,  
0x28bf,  

// AntiBand setting
0x2af0,               
0x2b34, //flicker unlock time
0x3078, 

0x3922,
0x3ade,
0x3b22,
0x3cde,

0x2f00,
0x43c0,
0x5628,
0x5778,
0x5820,
0x5960,

0x5e9f,  // hsync inc size 1.3mega 0x9f
0x5f77,  // vsync inc size 1.3mega 960 0x77

//AE weight start
0x60aa,
0x61aa,
0x62aa,
0x63aa,
0x64aa,
0x65aa,
0x66ab,
0x67ea,
0x68ab,
0x69ea,
0x6aaa,
0x6baa,
0x6caa,
0x6daa,
0x6eaa,
0x6faa,

0x7030, //1280*960

0x7184,  //00_2*4=+8
0x7622,  //unlock bound1 middle
0x7781,  //unlock bound2 middle
0x7822,  //Y-TH1  <lockboundary,unlock1>
0x7926,
0x7a24,
0x7B22,
0x7d22,
0x7e22,

//Pll 1x
0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 
0x8601, //EXPMin 6000.00 fps
0x87f4, 
0x8803, //EXP Max 12.00 fps 
0x89d0, 
0x8a90,  
0x8B75, //EXP100 
0x8C30, 
0x8D61, //EXP120 
0x8Ea8, 

0x9105, //EXP Fix 8.00 fps
0x92a5, 
0x9350,

0x9402,
0x957a,
0x96c4,

0x989e,  //outdoor th1
0x9946,  //outdoor th2

0x9b9a,  //outdoor th1
0x9a28,  //outdoor th2

0x9c0d, //EXP Limit 857.14 fps 
0x9dac, 
0x9e01,  //EXP Unit 
0x9ff4, 
0xa000,
0xa1c3,
0xa250,

//Exp12000
0xa300,  //Antibandtime/256=120*256=30720fps
0xa430,

//Capture Exptime
0xA501,  //Capture ExpmaxH
0xA66E,  //Capture ExpmaxM
0xA736,  //Capture ExpmaxL
0xA81D,  //Capture Exp100H
0xA94C,  //Capture Exp100L
0xAA18,  //Capture Exp120H //PLL 2x ³·Ãè´Ù°í °¡Á¤.
0xAB6A,  //Capture Exp120L
0xAC00,  //Capture Exp12000H
0xAD18,  //Capture Exp12000L
0xAE23,  //Capture Outtime th2

//AG set ag min 15
0xb010,  //AG
0xb110,  //AGMIN
0xb29f,  //AGMAX
0xb316,  //AGLVL //1a
0xb416,  //AGTH1 //1a
0xb53c,  //AGTH2  0x44
0xb629,  //AGBTH1 0x2f
0xb723,  //AGBTH2 0x28
0xb820,  //AGBTH3 0x25
0xb91e,  //AGBTH4 0x22
0xba1c,  //AGBTH5 0x21
0xbb1b,  //AGBTH6 0x20
0xbc1b,  //AGBTH7 0x1f
0xbd1a,  //AGBTH8 0x1f

0xbe36,  //AGBTH9 //Adaptive_AG Min
0xbf36,  //AGBTH10
0xc035,  //AGBTH11
0xc135,  //AGBTH12
  
0xc230,  //Adaptive AGMIN
0xc330,  //Adaptive AGLVL
0xc430,  //20091210 2a->25
0xc50a,  //20091210 0z->1f 10/120sec
  
0xc648,  //Middle AG
0xcc48,  //Middle AG min
0xcae0,  //Middle DG
0xc718,  //Sky_gain
0xc8f8,  //Middle indoor max c8->d6
0xc980,  //Middle inddor min


//PAGE22
0x0322,
0x10ef,  //ea
0x112e,

//uniform
0x1800,  //uniform_on  0x01 
0x1a54,  //r_max
0x1b2e,  //r_min
0x1c44,  //b_max
0x1d2e,  //b_min
0x1e04,  //time_th
0x1f60,  //time_th



0x1941,  //Low temp off 0x40 -> Low temp on 0x41
0x2038,  //AWB unstable speed
0x2100,  //skin color on
0x2200,  
0x7ad6,
0x2340,
0x2407,
0x253C,
0x2600,
0x270B,  //6b
0x2866,
0x2900,  // method 1 //10
0x2A0a,
0x2B00,
0x2C00,
0x2D7A,
0x2Eff,  //68
0x2F00,  //lowtemp crcb method 0x00 old

0x3080,
0x3180,

//Pixel slope
0x3418,
0x350B,
0x362A,
0x373A,

0x3812,
0x3966,

0x40f6,

0x4602,
0x4DD4,  //d4
0x4e0a,

//stable band
0x4233,//csum2
0x5033,//csum4
0x4144,//cdiff
0x40f2,//yth
  
//unstable band
0x4544,  //csum2
0x5134,  //csum4
0x4444,  //cdiff
0x43f2,  //yth  

//dark band
0x4934,  //csum2
0x5223,  //csum4
0x4853,  //cdiff
0x47f3,  //yth  

//Out door band
0x4c34,  //csum2
0x5323,  //csum4
0x4b53,  //cdiff
0x4a84,  //yth  

//Csum band
0x5400,
0x55A5,  //sign
0x5611,  //ofs
0x5755,  //ofs

0x5AE0,
0x5BF0,
0x5C60,
0x5D14,
0x5E40,
0x5F00,

0x60F8,
0x6164,
0x6210,
0x63F8,
0x6420,
0x6522,  //D1/D3 valid color th
0x6644,  //D2/D4 valid color th
0x6714,
0x6814,
0x6914,
0x6A14,

0x6b00,
0x6c00,
0x6d40,
0x6e50,
0x6D40,
0x6E50,
0x6F30,

0x7040,
0x7150,
0x7230,
0x7388,
0x7440,
0x7550,
0x7630,
0x7788,
0xE122,

//Gain setti2ng
0x803a,
0x8120,
0x8235,

0x8354,	//Normal R max
0x8422,	//Normal R min  29->28   20->2a
0x8555,	//Normal B max  50->52
0x861f,	//Normal B mim

0x8752,	//middle R max
0x8830,	//middle R min
0x8946,	//middle B max  43->36 
0x8a1f,	//middle B min

0x8b50,	//bottom R max
0x8c3d,	//bottom R min
0x8d30,	//bottom B max
0x8e1f,	//bottom B min

// awb slop, band
0x8f55,	//p1
0x9053,	//p2
0x9150,	//p3
0x924c,	//p4	
0x9349,	//p5
0x943c,	//p6
0x9536,	//p7
0x9631,	//p8
0x9728,	//p9
0x9824,	//p10
0x991f,	//p11
0x9a1e,	//p12

0x9b77, //56 band12
0x9c77, //76 band34

0x9d4f,	//R_bandth 1
0x9e30,	//R_bandth 2
0x9f2c,	//R_bandth 3

//Outdoor limit r/b
0xa023,	//R delta 1
0xa132,	//B delta 1
0xa213,	//R delta 2
0xa321,	//B delta 2

0xa410,	//R_TH1	1920fps  0xa->0x10
0xa53a,	//R_TH2
0xa670,	//R_TH3

0xa700,	//low temp R min
0xa818,	//low temp R max
0xad38,
0xae30,
0xaf32,
0xb02c,
0xb100,  //lowtemp y control 0x28->0x00
0xb248,
0xb340,

//Low temp Tgt
0xB57f,
0xB600,
0xB701,  //low Cb,Cr target offset 0x09->0
0xB82c,  //Low temp off
0xB929,  //Low temp On
0xBC00,  //Low gain cb/cb retarget point
0xBD36,  //Low gain 80/80 point


0x0310,
0x3701,


//Resol detect control ///
0x0324,
0x604f,  //edge even frame | 16bit resol | white edge cnt | scene resol enable
0x6104,  //even frame update
0x6435,  //edge th1 H
0x6500,  //edge th1 L
0x6630,  //edge th2 H
0x6700,  //edge th2 L

0x0313,
0x7402,  //det slope en | gausian filter
0x7509,  //1D negative gain
0x7609,  //1D postive  gain
0x770c,  //1D hclp2

0xf80d,  //sharp2d resl coef_L => coef_h_en | slope en
0xf930,  //2D negative gain
0xfa30,  //2D postive  gain
0xfb1e,  //2D hclp2
0xfc37,  //2D Hi  frequence Gaussian coef
0xfd20,  //2D Hi  frequence th 
0xfe08,  //2D Low frequence th 

0x0312,  //false color reduction
0x7b10,  //Detect resolution only	
0x7c04,  //false blur offset
0x7d04,  //false blur offset limit

0x0320,
0x108c,//AE ON
0x0322,
0x10eb,//AWB ON
0x01f0,

0xff14, //200ms delay

};

static unsigned short sr130pc10_preview_reg[] = {
0x0300,
0x01f1,  //sleep on   

0x0320,  //page 20
0x100c,  //AE off
0x0322,  //page 22
0x106b,  //AWB off

0x0300,
0x101d,  //Sub 1/2 | VSYNC type2 | Preview1 Mode
0x1191,  //windowing | bad_frame skip 1 | y Flip

0x2913,  //Preview1 mode row start

0x0312,   //YC_LPF
0x2016,  //Yc2d ctrl1 Preview Yc2d    off[0x16] on[0x17]
0x210e,  //Yc2d ctrl2 Preview Cfilter off[0x0e] on[0x0f]

//DPC_CTRL
0x905c,	//Preview DPC off[0x5c] on[0x5d]

// PAGE13  Sharpness 1D/2D  ////////////////////////////////////
0x0313,
0x80c0,  //sharp2 slope auto en | sharp2d statu auto en | sharp2d off[0xc0] on[0xc1] 

0x0318,  //page 18
0x1000,  //scale off

0x0320,
0x108c,  //AE ON
0x0322,
0x10eb,  //AWB ON

0x01f0,  //Sleep Off

0x0300,  //dummy1    delay_
0x0300,  //dummy2        
0x0300,  //dummy3

0xff14, //200ms delay

};

static unsigned short sr130pc10_capture_reg[] = {
0x0300,  //Page 0                                  
0x01f1,  //sleep On                                
	                                                    
0x0320,  //Page 20                                 
0x100c,  //AE OFF                                  
0x0322,  //Page 22                                 
0x106b,  //AWB OFF                                 
    
0x0300,  //page 0                                  
0x100c,  //Vsync type2 | win_cfg_mode | full scale 

0x0312,   //YC_LPF
0x2017,  //Yc2d ctrl1 Preview Yc2d    off[0x16] on[0x17]
0x210f,  //Yc2d ctrl2 Preview Cfilter off[0x0e] on[0x0f]

//DPC_CTRL
0x905d,	//Preview DPC off[0x5c] on[0x5d]

// PAGE13  Sharpness 1D/2D  ////////////////////////////////////
0x0313,
0x80c1,  //sharp2 slope auto en | sharp2d statu auto en | sharp2d off[0xc0] on[0xc1] 


0x0318, //18page
0x1000, //Scale off

0x0300,  //dummy1    delay_
0x0300,  //dummy2        
0x0300,  //dummy3
0x0300,  //dummy4    
0x0300,  //dummy5        
0x0300,  //dummy6   
0x0300,  //dummy7    
0x0300,  //dummy8        
0x0300,  //dummy9
0x0300,  //dummy10                 

0x0300,  //Page 0
0x01f0,  //Sensor sleep off, Output Drivability Max

0xff14, //200ms delay

};

/************** Exposure Value Setting ****************/
/*
 * EV bias
 */
static unsigned short sr130pc10_ev_m4[] = {	
0x0310,
0x40d0,
};

static unsigned short sr130pc10_ev_m3[] = {	
0x0310,
0x40b0,
};

static unsigned short sr130pc10_ev_m2[] = {	
0x0310,
0x40a0,
};

static unsigned short sr130pc10_ev_m1[] = {	
0x0310,
0x4090,
};

static unsigned short sr130pc10_ev_default[] = {	
0x0310,
0x4000,
};

static unsigned short sr130pc10_ev_p1[] = {	
0x0310,
0x4010,
};

static unsigned short sr130pc10_ev_p2[] = {	
0x0310,
0x4020,
};

static unsigned short sr130pc10_ev_p3[] = {	
0x0310,
0x4030,
};

static unsigned short sr130pc10_ev_p4[] = {	
0x0310,
0x4050,
};


static unsigned short sr130pc10_ev_vt_m4[] = {	
0x0310,
0x40d0,
};

static unsigned short sr130pc10_ev_vt_m3[] = {	
0x0310,
0x40b0,
};

static unsigned short sr130pc10_ev_vt_m2[] = {	
0x0310,
0x40a0,
};

static unsigned short sr130pc10_ev_vt_m1[] = {	
0x0310,
0x4090,
};

static unsigned short sr130pc10_ev_vt_default[] = {	
0x0310,
0x4000,
};

static unsigned short sr130pc10_ev_vt_p1[] = {	
0x0310,
0x4010,
};

static unsigned short sr130pc10_ev_vt_p2[] = {	
0x0310,
0x4020,
};

static unsigned short sr130pc10_ev_vt_p3[] = {	
0x0310,
0x4030,
};

static unsigned short sr130pc10_ev_vt_p4[] = {	
0x0310,
0x4050,
};


/************** White Balance Setting ******************/
static unsigned short sr130pc10_wb_auto[] = {	
0x0322, //page 22 AWB
0x106b, //awb off
0x112e, //awbctrl2  
0x803a, //r_gain
0x8120, //g_gain 
0x8235, //b_gain  
0x8354, //r gain max
0x841c, //r gain min
0x855e, //b gain max
0x861f, //b gain min
0x10eb, //awb on
};

static unsigned short sr130pc10_wb_tungsten[] = {	
0x0322, //page 22 AWB
0x106b, //awb off
0x112e, //awbctrl2  
0x803a, //r_gain
0x8120, //g_gain 
0x8235, //b_gain  
0x8354, //r gain max
0x841c, //r gain min
0x855e, //b gain max
0x861f, //b gain min
0x10eb, //awb on
};

static unsigned short sr130pc10_wb_fluorescent[] = {	
0x0322,
0x106b, //awb off
0x112e, //awbctrl2
0x8037, //r gain
0x8120, //g gain
0x8248, //b gain
0x833c, //r gain max
0x8435, //r gain min
0x854d, //b gain max
0x8643, //b gain min
0x10eb, //awb on
};

static unsigned short sr130pc10_wb_sunny[] = {	
0x0322,
0x106b, //awb off
0x112e, //awbctrl2
0x8049, //r gain
0x8120, //g gain
0x8228, //b gain
0x834e, //r gain max  
0x8444, //r gain min
0x852d, //b gain max
0x8623, //b gain min
0x10eb, //awb on
};

static unsigned short sr130pc10_wb_cloudy[] = {	
0x0322,
0x106b, //awb off
0x112e, //awbctrl2
0x8049, //r gain
0x8120, //g gain
0x8228, //b gain
0x834e, //r gain max
0x8444, //r gain min
0x852d, //b gain max 
0x8623, //r gain min
0x10eb, //awb on
};

/************** Effect Setting ********************/
static unsigned short sr130pc10_effect_none[] = {	
0x0310, //Page 10 <Effect>
0x1103, //Effect control  0x03 default
0x1210, //0x10 default
};

static unsigned short sr130pc10_effect_gray[] = {	
0x0310, //Page 10 <Effect>
0x1103, //Effect control  0x03 default
0x1210, //0x10 default
};

static unsigned short sr130pc10_effect_sepia[] = {	
0x0310, //Page 10 <Effect>
0x1103, //Effect control  0x03 default
0x1213, //0x10 default ->0x13 cr,cb constant enable
0x4470, //Cb constant
0x4598, //Cr constant
};

static unsigned short sr130pc10_effect_negative[] = {	
0x0310, //Page 10 <Effect>  
0x1103, //Effect control  0x03 default
0x1238, //0x10 default ->0x18 enable Negative
};

static unsigned short sr130pc10_effect_aqua[] = {	
0x0310, //Page 10 <Effect>
0x1103, //Effect control  0x03 default
0x1210, //0x10 default
};

static unsigned short sr130pc10_dataline[] = {
0x0300,
0x5005, //Test Pattern
};

static unsigned short sr130pc10_dataline_stop[] = {	
0x0300,
0x5000, //Test Pattern
};


/************** FPS********************/
static unsigned short sr130pc10_fps_7[] = {	
0x0300,  //Page0 Fixed Frame,BLC
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

0x0300,  //Page0 Fixed Frame,BLC

0x101d,  //Sub 1/2 | VSYNC type2 | Preview1 Mode

0x1195,  //windowing, 1 badFrame skip Fixed frame rate[2]

0x0300, //PAGE 0
0x9004,  //BLC_TIME_TH_ON
0x9104,  //BLC_TIME_TH_OFF 
0x9278, //BLC_AG_TH_ON
0x9370, //BLC_AG_TH_OFF

0x0302, //PAGE 2
0xd404,  //DCDC_TIME_TH_ON
0xd504,  //DCDC_TIME_TH_OFF
0xd678, //DCDC_AG_TH_ON
0xd770, //DCDC_AG_TH_OFF

0x0320, //Page 20 
0x2bF4, 
0x30F8, 

0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 
0x8601, //EXPMin 7500.00 fps
0x8790, 
0x8801, //EXP Max 30.00 fps 
0x8986, 
0x8aa0, 
0x8B75, //EXP100 
0x8C30, 
0x8D61, //EXP120 
0x8Ea8, 
0x9106, //EXP Fix 7.00 fps
0x9279, 
0x93d0, 
0x9c07, //EXP Limit 1500.00 fps 
0x9dd0, 
0x9e01, //EXP Unit 
0x9f90,
                   
0x0320,
0x108c,  //AE ON
0x0322,
0x10eb,  //AWB ON

0x0300,  //Page0 
0x01f0,  //sleep off

};

static unsigned short sr130pc10_fps_10[] = {	
0x0300,  //Page0 Fixed Frame,BLC
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

0x0300,  //Page0 Fixed Frame,BLC

0x101d,  //Sub 1/2 | VSYNC type2 | Preview1 Mode

0x1195,  //windowing, 1 badFrame skip Fixed frame rate[2]

0x0300, //PAGE 0
0x0300, //PAGE 0
0x9004,  //BLC_TIME_TH_ON
0x9104,  //BLC_TIME_TH_OFF 
0x9278, //BLC_AG_TH_ON
0x9370, //BLC_AG_TH_OFF

0x0302, //PAGE 2
0xd404,  //DCDC_TIME_TH_ON
0xd504,  //DCDC_TIME_TH_OFF
0xd678, //DCDC_AG_TH_ON
0xd770, //DCDC_AG_TH_OFF

0x0320, //Page 20 
0x2bF4, 
0x30F8, 

0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 
0x8601, //EXPMin 7500.00 fps
0x8790, 
0x8801, //EXP Max 30.00 fps 
0x8986, 
0x8aa0, 
0x8B75, //EXP100 
0x8C30, 
0x8D61, //EXP120 
0x8Ea8, 
0x9104, //EXP Fix 10.00 fps
0x9284, 
0x9340, 
0x9c07, //EXP Limit 1500.00 fps 
0x9dd0, 
0x9e01, //EXP Unit 
0x9f90, 
                   
0x0320,
0x108c,  //AE ON
0x0322,
0x10eb,  //AWB ON

0x0300,  //Page0 
0x01f0,  //sleep off

};

static unsigned short sr130pc10_fps_15[] = {	
0x0300,  //Page0 Fixed Frame,BLC
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

0x0300,  //Page0 Fixed Frame,BLC

0x101d,  //Sub 1/2 | VSYNC type2 | Preview1 Mode

0x1195,  //windowing, 1 badFrame skip Fixed frame rate[2]

0x0300, //PAGE 0
0x9004,  //BLC_TIME_TH_ON
0x9104,  //BLC_TIME_TH_OFF 
0x9278, //BLC_AG_TH_ON
0x9370, //BLC_AG_TH_OFF

0x0302, //PAGE 2
0xd404,  //DCDC_TIME_TH_ON
0xd504,  //DCDC_TIME_TH_OFF
0xd678, //DCDC_AG_TH_ON
0xd770, //DCDC_AG_TH_OFF

0x0320, //Page 20 
0x2bF4, 
0x30F8, 

0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 
0x8601, //EXPMin 7500.00 fps
0x8790, 
0x8801, //EXP Max 30.00 fps 
0x8986, 
0x8aa0, 
0x8B75, //EXP100 
0x8C30, 
0x8D61, //EXP120 
0x8Ea8, 
0x9102, //EXP Fix 15.02 fps
0x92fc, 
0x93d8, 
0x9c07, //EXP Limit 1500.00 fps 
0x9dd0, 
0x9e01, //EXP Unit 
0x9f90, 

                   
0x0320,
0x108c,  //AE ON
0x0322,
0x10eb,  //AWB ON

0x0300,  //Page0 
0x01f0,  //sleep off		 
 
};

static unsigned short sr130pc10_vt_fps_7[] = {	
0x0300,  //Page0 Fixed Frame,BLC
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

0x0300,  //Page0 Fixed Frame,BLC

0x101d,  //Sub 1/2 | VSYNC type2 | Preview1 Mode

0x1195,  //windowing, 1 badFrame skip Fixed frame rate[2]

0x0300, //PAGE 0
0x9004,  //BLC_TIME_TH_ON
0x9104,  //BLC_TIME_TH_OFF 
0x9278, //BLC_AG_TH_ON
0x9370, //BLC_AG_TH_OFF

0x0302, //PAGE 2
0xd404,  //DCDC_TIME_TH_ON
0xd504,  //DCDC_TIME_TH_OFF
0xd678, //DCDC_AG_TH_ON
0xd770, //DCDC_AG_TH_OFF

0x0320, //Page 20 
0x2bF4, 
0x30F8, 

0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 
0x8601, //EXPMin 6000.00 fps
0x87f4, 
0x8801, //EXP Max 24.00 fps 
0x89e8, 
0x8a48, 
0x8B75, //EXP100 
0x8C30, 
0x8D61, //EXP120 
0x8Ea8, 
0x9106, //EXP Fix 7.00 fps
0x9276, 
0x934c, 
0x9c09, //EXP Limit 1200.00 fps 
0x9dc4, 
0x9e01, //EXP Unit 
0x9ff4, 
                   
0x0320,
0x108c,  //AE ON
0x0322,
0x10eb,  //AWB ON

0x0300,  //Page0 
0x01f0,  //sleep off
};

static unsigned short sr130pc10_vt_fps_10[] = {	
0x0300,  //Page0 Fixed Frame,BLC
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

0x0300,  //Page0 Fixed Frame,BLC

0x101d,  //Sub 1/2 | VSYNC type2 | Preview1 Mode

0x1195,  //windowing, 1 badFrame skip Fixed frame rate[2]

0x0300, //PAGE 0
0x9004,  //BLC_TIME_TH_ON
0x9104,  //BLC_TIME_TH_OFF 
0x9278, //BLC_AG_TH_ON
0x9370, //BLC_AG_TH_OFF

0x0302, //PAGE 2
0xd404,  //DCDC_TIME_TH_ON
0xd504,  //DCDC_TIME_TH_OFF
0xd678, //DCDC_AG_TH_ON
0xd770, //DCDC_AG_TH_OFF

0x0320, //Page 20 
0x2bF4, 
0x30F8, 

0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 
0x8601, //EXPMin 6000.00 fps
0x87f4, 
0x8801, //EXP Max 24.00 fps 
0x89e8, 
0x8a48, 
0x8B75, //EXP100 
0x8C30, 
0x8D61, //EXP120 
0x8Ea8, 
0x9104, //EXP Fix 10.00 fps
0x9280, 
0x9358, 
0x9c09, //EXP Limit 1200.00 fps 
0x9dc4, 
0x9e01, //EXP Unit 
0x9ff4, 
                   
0x0320,
0x108c,  //AE ON
0x0322,
0x10eb,  //AWB ON

0x0300,  //Page0 
0x01f0,  //sleep off
};

static unsigned short sr130pc10_vt_fps_15[] = {	
0x0300,  //Page0 Fixed Frame,BLC
0x01f1,  //sleep

0x0320,  //page 20
0x100c,  //ae off
0x0322,  //page 22
0x106b,  //awb off 

0x0300,  //Page0 Fixed Frame,BLC

0x101d,  //Sub 1/2 | VSYNC type2 | Preview1 Mode

0x1195,  //windowing, 1 badFrame skip Fixed frame rate[2]

0x0300, //PAGE 0
0x9004,  //BLC_TIME_TH_ON
0x9104,  //BLC_TIME_TH_OFF 
0x9278, //BLC_AG_TH_ON
0x9370, //BLC_AG_TH_OFF

0x0302, //PAGE 2
0xd404,  //DCDC_TIME_TH_ON
0xd504,  //DCDC_TIME_TH_OFF
0xd678, //DCDC_AG_TH_ON
0xd770, //DCDC_AG_TH_OFF

0x0320, //Page 20 
0x2bF4, 
0x30F8, 

0x8301, //EXP Normal 30.00 fps 
0x8486, 
0x85a0, 
0x8601, //EXPMin 6000.00 fps
0x87f4, 
0x8801, //EXP Max 24.00 fps 
0x89e8, 
0x8a48, 
0x8B75, //EXP100 
0x8C30, 
0x8D61, //EXP120 
0x8Ea8, 
0x9102, //EXP Fix 15.00 fps
0x92f9, 
0x93b8, 
0x9c09, //EXP Limit 1200.00 fps 
0x9dc4, 
0x9e01, //EXP Unit 
0x9ff4, 
                   
0x0320,
0x108c,  //AE ON
0x0322,
0x10eb,  //AWB ON

0x0300,  //Page0 
0x01f0,  //sleep off
};

#endif
