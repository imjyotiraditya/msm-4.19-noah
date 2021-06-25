/***************************************************
 * File:touch.c
 * VENDOR_EDIT
 * Copyright (c)  2008- 2030  Oppo Mobile communication Corp.ltd.
 * Description:
 *             tp dev
 * Version:1.0:
 * Date created:2016/09/02
 * Author: hao.wang@Bsp.Driver
 * TAG: BSP.TP.Init
*/


#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/serio.h>
#include "oppo_touchscreen/tp_devices.h"
#include "oppo_touchscreen/touchpanel_common.h"

#include <soc/oppo/oppo_project.h>
#include "touch.h"

#define MAX_LIMIT_DATA_LENGTH         100

struct tp_dev_name tp_dev_names[] = {
    {TP_BOE,"BOE"},
    {TP_INNOLUX, "INNOLUX"},
    {TP_BOE_90HZ, "90HZ_BOE"},
    {TP_HLT_90HZ, "90HZ_HLT"},
    {TP_TIANMA, "TIANMA"},
    {TP_SAMSUNG, "SAMSUNG"},
    {TP_UNKNOWN, "UNKNOWN"},
};

#define GET_TP_DEV_NAME(tp_type) ((tp_dev_names[tp_type].type == (tp_type))?tp_dev_names[tp_type].name:"UNMATCH")

int g_tp_dev_vendor = TP_UNKNOWN;
int g_chip_name = UNKNOWN_IC;   //add by wanglongfei  distinguish IC
typedef enum {
    TP_INDEX_NULL,
    ili9881_boe,
    nt36525b_inx,
    ili9881_90hz_boe,
    ili9881_90hz_hlt,
    hx83112a_tianma,
    gt9886_samsung,
} TP_USED_INDEX;
TP_USED_INDEX tp_used_index  = TP_INDEX_NULL;
extern char* saved_command_line;

/*
* this function is used to judge whether the ic driver should be loaded
* For incell module, tp is defined by lcd module, so if we judge the tp ic
* by the boot command line of containing lcd string, we can also get tp type.
*/

TP_USED_IC __init tp_judge_ic_match(void)
{
    pr_err("[TP] saved_command_line = %s \n", saved_command_line);

    if (strstr(saved_command_line, "mdss_dsi_ili9881h_boe_video")) {
        g_tp_dev_vendor = TP_BOE;
        tp_used_index = ili9881_boe;
        g_chip_name = ILI9881H;
        printk("g_tp_dev_vendor: %d, tp_used_index: %d\n", g_tp_dev_vendor, tp_used_index);
        return g_chip_name;
    }
//#ifdef ODM_HQ_EDIT
/* dongfeiju, 2020/05/15, Add for NOVATEK bringup */
    if (strstr(saved_command_line, "mdss_dsi_nt36525b_inx_video")) {
        g_tp_dev_vendor = TP_INNOLUX;
        tp_used_index = nt36525b_inx;
        g_chip_name =  NT36525B;
        printk("[TP]touchpanel: g_tp_dev_vendor: %d, tp_used_index: %d\n", g_tp_dev_vendor, tp_used_index);
        return g_chip_name;
    }
    if (strstr(saved_command_line, "mdss_dsi_ili9881h_boe_90hz_video")) {
        g_tp_dev_vendor = TP_BOE_90HZ;
        tp_used_index = ili9881_90hz_boe;
        g_chip_name = ILI9881H;
        printk("g_tp_dev_vendor: %d, tp_used_index: %d\n", g_tp_dev_vendor, tp_used_index);
        return g_chip_name;
    }

#ifdef VENDOR_EDIT
/*Jianghao, 2020/06/11, Add for coco TP notifier register*/
    if (strstr(saved_command_line, "mdss_dsi_ili9881h_hlt_90hz_video")) {
        g_tp_dev_vendor = TP_HLT_90HZ;
        tp_used_index = ili9881_90hz_hlt;
        g_chip_name = ILI9881H;
        printk("g_tp_dev_vendor: %d, tp_used_index: %d\n", g_tp_dev_vendor, tp_used_index);
        return g_chip_name;
    }

    if (strstr(saved_command_line, "mdss_dsi_hx83112a_tm_90hz_video")) {
        g_tp_dev_vendor = TP_TIANMA;
        tp_used_index = hx83112a_tianma;
        g_chip_name = HIMAX83112A;
        printk("g_tp_dev_vendor: %d, tp_used_index: %d\n", g_tp_dev_vendor, tp_used_index);
        return g_chip_name;
    }
#endif

    if (strstr(saved_command_line, "mdss_s6e8fc1_samsung_video")) {
        g_tp_dev_vendor = TP_SAMSUNG;
        tp_used_index = gt9886_samsung;
        g_chip_name = GT9886;
        printk("g_tp_dev_vendor: %d, tp_used_index: %d\n", g_tp_dev_vendor, tp_used_index);
        return g_chip_name;
    }

    pr_err("Lcd module not found\n");
    return UNKNOWN_IC;
}

int tp_util_get_vendor(struct hw_resource *hw_res, struct panel_info *panel_data)
{

    char *vendor = NULL;
    panel_data->test_limit_name = kzalloc(MAX_LIMIT_DATA_LENGTH, GFP_KERNEL);
    if (panel_data->test_limit_name == NULL) {
        pr_err("[TP]panel_data.test_limit_name kzalloc error\n");
    }
    panel_data->tp_type = g_tp_dev_vendor;
    vendor = GET_TP_DEV_NAME(panel_data->tp_type);
    strncpy(panel_data->manufacture_info.manufacture, vendor, MAX_DEVICE_MANU_LENGTH);

    if (panel_data->tp_type == TP_UNKNOWN) {
        pr_err("[TP]%s type is unknown\n", __func__);
        return 0;
    }

    panel_data->vid_len = 8;
//#ifdef ODM_HQ_EDIT
/*Hao.Jiang@BSP.TP.Function, 2020/05/14, add for Rum TP FW*/
    switch(get_project()) {
    case 20021:
    case 20022:
    case 20229:
    case 20228:
    case 20227:
    case 20226:
    case 20225:
    case 20224:
    case 20223:
    case 20222:
    case 20221:
        if (panel_data->fw_name) {
            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                     "tp/20221/FW_%s_%s.bin",
                     panel_data->chip_name, vendor);
        }

        if (panel_data->test_limit_name) {
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "/tp/20221/LIMIT_%s_%s.img",
                     panel_data->chip_name, vendor);
            printk("panel_data->test_limit_name = %s\n", panel_data->test_limit_name);
        }
        break;
    case 20215:
    case 20214:
    case 20213:
    case 20212:
    case 20211:
    case 20673:
    case 20674:
    case 20675:
    case 20677:
    case 0x2067A:
    case 0x2067D:
    case 0x2067E:
		//printk("[jianghao]:coco");
        if (panel_data->fw_name) {
            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                     "tp/20673/FW_%s_%s.bin",
                     panel_data->chip_name, vendor);
        }

        if (panel_data->test_limit_name) {
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "/tp/20673/LIMIT_%s_%s.img",
                     panel_data->chip_name, vendor);
            printk("panel_data->test_limit_name = %s\n", panel_data->test_limit_name);
        }
        break;
    case 20670:
    case 20671:
    case 20672:
    case 20676:
    case 20679:
    case 0x2067C:
		//printk("[jianghao]:coco-b");
        if (panel_data->fw_name) {
            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                     "tp/20673/FW_B_%s_%s.bin",
                     panel_data->chip_name, vendor);
        }

        if (panel_data->test_limit_name) {
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "/tp/20673/LIMIT_B_%s_%s.img",
                     panel_data->chip_name, vendor);
            printk("panel_data->test_limit_name = %s\n", panel_data->test_limit_name);
        }
        break;
    default:
        if (panel_data->fw_name) {
            snprintf(panel_data->fw_name, MAX_FW_NAME_LENGTH,
                     "tp/%d/FW_%s_%s.bin",
                     panel_data->chip_name, vendor);
        }

        if (panel_data->test_limit_name) {
            snprintf(panel_data->test_limit_name, MAX_LIMIT_DATA_LENGTH,
                     "/tp/%d/LIMIT_%s_%s.img",
                     panel_data->chip_name, vendor);
            printk("panel_data->test_limit_name = %s\n",get_project(), panel_data->test_limit_name);
        }
        break;
    }
//#endif

   if (tp_used_index == ili9881_boe) {
        memcpy(panel_data->manufacture_info.version, "RA378BI_", 8);
        panel_data->firmware_headfile.firmware_data = FW_20673_ILI9881H_BOE;
        panel_data->firmware_headfile.firmware_size = sizeof(FW_20673_ILI9881H_BOE);
    } else if (tp_used_index == nt36525b_inx) {
        memcpy(panel_data->manufacture_info.version, "RA373IN_", 8);
        panel_data->firmware_headfile.firmware_data = FW_20221_NT36525B_INNOLUX;
        panel_data->firmware_headfile.firmware_size = sizeof(FW_20221_NT36525B_INNOLUX);
//#ifdef ODM_HQ_EDIT
/*Hao.Jiang@BSP.TP.Function, 2020/05/14, add for Rum TP FW*/
    } else if (tp_used_index == gt9886_samsung) {
        memcpy(panel_data->manufacture_info.version, "RA373ON_", 8);
//#endif
    } else if (tp_used_index == ili9881_90hz_boe) {
        if((get_project() == 20673)||(get_project() == 20674)||(get_project() == 20675)||(get_project() == 20677)||(get_project() == 0x2067A)||(get_project() == 0x2067D)||(get_project() == 0x2067E)){
			//printk("[jianghao]:coco111");
			panel_data->vid_len = 14;
			memcpy(panel_data->manufacture_info.version, "RA378_BOE_ILI_", 14);
			panel_data->firmware_headfile.firmware_data = FW_20673_ILI9881H_90HZ_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20673_ILI9881H_90HZ_BOE);
			}
        if((get_project() == 20670)||(get_project() == 20671)||(get_project() == 20672)||(get_project() == 20676)||(get_project() == 20679)||(get_project() == 0x2067C)){
			//printk("[jianghao]:coco-b111");
			panel_data->vid_len = 16;
			memcpy(panel_data->manufacture_info.version, "RA378_B_BOE_ILI_", 16);
			panel_data->firmware_headfile.firmware_data = FW_B_20671_ILI9881H_90HZ_BOE;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_B_20671_ILI9881H_90HZ_BOE);
			}
#ifdef VENDOR_EDIT
/*Jianghao, 2020/06/11, Add for coco TP notifier register*/
	} else if (tp_used_index == ili9881_90hz_hlt) {
	    if((get_project() == 20673)||(get_project() == 20674)||(get_project() == 20675)||(get_project() == 20677)||(get_project() == 0x2067A)||(get_project() == 0x2067D)||(get_project() == 0x2067E)){
			//printk("[jianghao]:coco222");
			panel_data->vid_len = 14;
			memcpy(panel_data->manufacture_info.version, "RA378_HLT_ILI_", 14);
			panel_data->firmware_headfile.firmware_data = FW_20673_ILI9881H_90HZ_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_20673_ILI9881H_90HZ_HLT);
			}
        if((get_project() == 20670)||(get_project() == 20671)||(get_project() == 20672)||(get_project() == 20676)||(get_project() == 20679)||(get_project() == 0x2067C)){
			//printk("[jianghao]:coco-b222");
			panel_data->vid_len = 16;
			memcpy(panel_data->manufacture_info.version, "RA378_B_HLT_ILI_", 16);
			panel_data->firmware_headfile.firmware_data = FW_B_20671_ILI9881H_90HZ_HLT;
			panel_data->firmware_headfile.firmware_size = sizeof(FW_B_20671_ILI9881H_90HZ_HLT);

			}

	} else if (tp_used_index == hx83112a_tianma) {
		panel_data->vid_len = 15;
		memcpy(panel_data->manufacture_info.version, "RA378_TM_Himax_", 15);
		panel_data->firmware_headfile.firmware_data = FW_20671_Hx83221A_TIANMA;
		panel_data->firmware_headfile.firmware_size = sizeof(FW_20671_Hx83221A_TIANMA);
#endif
	} else {
        panel_data->firmware_headfile.firmware_data = NULL;
        panel_data->firmware_headfile.firmware_size = 0;
    }


    panel_data->manufacture_info.fw_path = panel_data->fw_name;

    pr_info("fw_path: %s\n", panel_data->manufacture_info.fw_path);
    pr_info("[TP]vendor:%s fw:%s limit:%s\n",
            vendor, panel_data->fw_name,
            panel_data->test_limit_name == NULL ? "NO Limit" : panel_data->test_limit_name);

    return 0;

}
