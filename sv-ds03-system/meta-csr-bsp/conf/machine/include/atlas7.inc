BOARD = "atlas7cb"

#$(BR2_SDRAM_TYPE)_${BR2_SDRAM_SIZE}MB${BR2_SDRAM_CLOCK}_${BR2_BOARD}.h
# DDR2_1CS_128MB is not set
# DDR2_1CS_256MB is not set

# DDR2_1CS_1GB is not set
# DDR2_2CS_512MB is not set

# DDR3_1CS_256MB is not set
DDR3_1CS_512MB="y"
# DDR3_2CS_512MB is not set
# DDR3_1CS_1GB is not set
# LPDDR2_2CS_512MB is not set
SDRAM_SIZE="1024"
SDRAM_TYPE="DDR3_1CS"
SDRAM_CLOCK="401"

# ENABLE_BPP32 is not set

# GPSRF_TRIG is not set
GPSRF_TRIGLITE="y"
# GPSRF_3IPLUS_16M is not set
# GPSRF_3IPLUS_26M is not set
# GPSRF_CD4150 is not set

# GPSVER_B116 is not set
GPSVER_S117="y"
# GPSVER_S118 is not set

GPSRF="triglite"
GPSVER="s117"

