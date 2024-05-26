# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\jmadridh\ECE423\workspace\lab_prefab_wrapper\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\jmadridh\ECE423\workspace\lab_prefab_wrapper\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {lab_prefab_wrapper}\
-hw {C:\Users\jmadridh\ECE423\ece423_prefab\lab_prefab\lab_prefab_wrapper.xsa}\
-out {C:/Users/jmadridh/ECE423/workspace}

platform write
domain create -name {standalone_ps7_cortexa9_0} -display-name {standalone_ps7_cortexa9_0} -os {standalone} -proc {ps7_cortexa9_0} -runtime {cpp} -arch {32-bit} -support-app {hello_world}
platform generate -domains 
platform active {lab_prefab_wrapper}
domain active {zynq_fsbl}
domain active {standalone_ps7_cortexa9_0}
platform generate -quick
platform generate
platform active {lab_prefab_wrapper}
bsp reload
bsp setlib -name xilffs -ver 4.7
bsp write
bsp reload
catch {bsp regenerate}
bsp reload
bsp write
bsp config enable_multi_partition "false"
bsp config enable_multi_partition "true"
bsp config num_logical_vol "10"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_ps7_cortexa9_0 
platform active {lab_prefab_wrapper}
bsp reload
bsp reload
platform generate -domains 
platform generate
platform generate
platform generate
platform generate
platform generate
platform generate
platform generate
platform generate
platform generate
platform generate
platform generate
