.section .time_critical.firmware
.global firmware
.type firmware, %object
.balign 4
firmware:
.incbin "../342-0270-C.bin"
