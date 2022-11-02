#
# Copyright (C) 2021 Teltonika
#

export TLT_PLATFORM_TRB2:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_trb2xx),y,)
export TLT_PLATFORM_RUTX:=$(if $(CONFIG_TARGET_ipq40xx_generic_DEVICE_teltonika_rutx),y,)
export TLT_PLATFORM_RUT2:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_rut2xx),y,)
export TLT_PLATFORM_RUT2M:=$(if $(CONFIG_TARGET_ramips_mt76x8_DEVICE_teltonika_rut2m),y,)
export TLT_PLATFORM_TCR1:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_tcr1xx),y,)
export TLT_PLATFORM_RUT9:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_rut9xx),y,)
export TLT_PLATFORM_RUT9M:=$(if $(CONFIG_TARGET_ramips_mt76x8_DEVICE_teltonika_rut9m),y,)
export TLT_PLATFORM_RUT952:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_rut952),y,)
export TLT_PLATFORM_RUT30X:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_rut30x),y,)
export TLT_PLATFORM_RUT36X:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_rut36x),y,)
export TLT_PLATFORM_OTD1:=$(if $(CONFIG_TARGET_ath79_generic_DEVICE_teltonika_otd1xx),y,)
export TLT_PLATFORM_TRB5:=$(if $(CONFIG_TARGET_sdxprairie_DEVICE_teltonika_trb5xx),y,)

# TRB1 family
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1400),y,)
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1401),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1410),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1411),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1412),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1420),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1421),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1422),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1430),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1450),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_mdm9x07_DEVICE_trb1451),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1400),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1401),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1410),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1411),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1412),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1420),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1421),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1422),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1423),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1430),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1450),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1451),y,$(TLT_PLATFORM_TRB1))
export TLT_PLATFORM_TRB1:=$(if $(CONFIG_TARGET_DEVICE_mdm9x07_DEVICE_trb1452),y,$(TLT_PLATFORM_TRB1))

export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_TRB2),TRB2,unknown)
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUT2),RUT2,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUT2M),RUT2M,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUT9),RUT9,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUT9M),RUT9M,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUT952),RUT952,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUT30X),RUT300,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUT36X),RUT360,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_RUTX),RUTX,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_TRB1),TRB1,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_TCR1),TCR1,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_OTD1),OTD1,$(TLT_PLATFORM_NAME))
export TLT_PLATFORM_NAME:=$(if $(TLT_PLATFORM_TRB5),TRB5,$(TLT_PLATFORM_NAME))

TARGET_CPPFLAGS += -D$(TLT_PLATFORM_NAME)_PLATFORM

TARGET_CPPFLAGS += $(if $(CONFIG_DEV_SINGLE_ETH_PORT),-DSINGLE_ETH_PORT)
TARGET_CPPFLAGS += $(if $(CONFIG_MOBILE_SUPPORT),-DMOBILE_SUPPORT)
TARGET_CPPFLAGS += $(if $(CONFIG_DUAL_SIM_SUPPORT),-DDUAL_SIM_SUPPORT)
TARGET_CPPFLAGS += $(if $(CONFIG_GPS_SUPPORT),-DGPS_SUPPORT)
TARGET_CPPFLAGS += $(if $(CONFIG_BLUETOOTH_SUPPORT),-DBLUETOOTH_SUPPORT)
TARGET_CPPFLAGS += $(if $(CONFIG_WIFI_SUPPORT),-DWIFI_SUPPORT)
TARGET_CPPFLAGS += $(if $(CONFIG_IO_SUPPORT),-DIO_SUPPORT)