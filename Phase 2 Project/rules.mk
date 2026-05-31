# SPDX-License-Identifier: Apache-2.0

###################################################################################################
#
#   FileName : ruls.mk
#
#   Copyright (c) Telechips Inc.
#
#   Description :
#
#
###################################################################################################

MCU_BSP_APP_SAMPLE_BASE_PATH := $(MCU_BSP_BUILD_CURDIR)

# Flags
COMMON_FLAGS += -DMCU_BSP_SUPPORT_APP_BASE=1

# Paths
VPATH += $(MCU_BSP_APP_SAMPLE_BASE_PATH)
VPATH += $(MCU_BSP_APP_SAMPLE_BASE_PATH)/ioLibrary_Driver/Ethernet
VPATH += $(MCU_BSP_APP_SAMPLE_BASE_PATH)/ioLibrary_Driver/Ethernet/W5500

# Includes
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_BASE_PATH)
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_BASE_PATH)/$(MCU_BSP_CHIPSET_FAMILY_NAME)
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_BASE_PATH)/ioLibrary_Driver
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_BASE_PATH)/ioLibrary_Driver/Ethernet
INCLUDES += -I$(MCU_BSP_APP_SAMPLE_BASE_PATH)/ioLibrary_Driver/Ethernet/W5500


# Sources
SRCS += main.c
SRCS += uss.c
SRCS += scc.c
SRCS += uart5.c
SRCS += lfa.c
SRCS += mcb.c
SRCS += pedal.c
SRCS += speed.c
SRCS += cant.c
SRCS += w5500_port.c
SRCS += w5500.c
SRCS += socket.c
SRCS += wizchip_conf.c
