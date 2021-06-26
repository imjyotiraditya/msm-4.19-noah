ccflags-y += $(DEFS_PLATFORM)
ifeq ($(CONFIG_OPPO_FINGERPRINT_QCOM),y)
ccflags-y += -DQCOM_PLATFORM
else ifeq ($(CONFIG_OPPO_FINGERPRINT_MTK),y)
ccflags-y += -DMTK_PLATFORM
endif

obj-y     +=  oppo_fp_common.o

