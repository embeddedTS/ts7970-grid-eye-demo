################################################################################
#
# TS7970_GRID_EYE_DEMO
#
################################################################################

TS7970_GRID_EYE_DEMO_VERSION = v2.0.0
TS7970_GRID_EYE_DEMO_SITE = $(call github,embeddedTS,ts7970-grid-eye-demo,$(TS7970_GRID_EYE_DEMO_VERSION))
TS7970_GRID_EYE_DEMO_LICENSE = BSD-2-Clause
TS7970_GRID_EYE_DEMO_LICENSE_FILES = LICENSE
TS7970_GRID_EYE_DEMO_INSTALL_STAGING = YES

$(eval $(qmake-package))
