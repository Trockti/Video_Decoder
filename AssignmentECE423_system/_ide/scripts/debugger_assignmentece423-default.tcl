# Usage with Vitis IDE:
# In Vitis IDE create a Single Application Debug launch configuration,
# change the debug type to 'Attach to running target' and provide this 
# tcl script in 'Execute Script' option.
# Path of this script: C:\Users\jmadridh\workspace\AssignmentECE423_system\_ide\scripts\debugger_assignmentece423-default.tcl
# 
# 
# Usage with xsct:
# To debug using xsct, launch xsct and run below command
# source C:\Users\jmadridh\workspace\AssignmentECE423_system\_ide\scripts\debugger_assignmentece423-default.tcl
# 
connect -url tcp:127.0.0.1:3121
targets -set -nocase -filter {name =~"APU*"}
rst -system
after 3000
targets -set -filter {jtag_cable_name =~ "Xilinx PYNQ-Z1 003017A8B751A" && level==0 && jtag_device_ctx=="jsn-Xilinx PYNQ-Z1-003017A8B751A-23727093-0"}
fpga -file C:/Users/jmadridh/workspace/AssignmentECE423/_ide/bitstream/lab_prefab_wrapper.bit
targets -set -nocase -filter {name =~"APU*"}
loadhw -hw C:/Users/jmadridh/workspace/new_platform/export/new_platform/hw/lab_prefab_wrapper.xsa -mem-ranges [list {0x40000000 0xbfffffff}] -regs
configparams force-mem-access 1
targets -set -nocase -filter {name =~"APU*"}
source C:/Users/jmadridh/workspace/AssignmentECE423/_ide/psinit/ps7_init.tcl
ps7_init
ps7_post_config
targets -set -nocase -filter {name =~ "*A9*#0"}
dow C:/Users/jmadridh/workspace/AssignmentECE423/Release/AssignmentECE423.elf
targets -set -nocase -filter {name =~ "*A9*#1"}
dow C:/Users/jmadridh/workspace/cpu1/Release/cpu1.elf
configparams force-mem-access 0
targets -set -nocase -filter {name =~ "*A9*#0"}
con
targets -set -nocase -filter {name =~ "*A9*#1"}
con
