# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct C:\Users\srutete\workspace\new_platform\platform.tcl
# 
# OR launch xsct and run below command.
# source C:\Users\srutete\workspace\new_platform\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {new_platform}\
-hw {C:\Users\srutete\ece423_prefab\lab_prefab\lab_prefab_wrapper.xsa}\
-proc {ps7_cortexa9_0} -os {standalone} -out {C:/Users/srutete/workspace}

platform write
platform generate -domains 
platform active {new_platform}
bsp reload
bsp reload
bsp setlib -name xilffs -ver 4.7
bsp config num_logical_vol "10"
bsp config enable_multi_partition "true"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
platform generate
platform generate
domain create -name {standalone_ps7_cortexa9_1} -display-name {standalone_ps7_cortexa9_1} -os {standalone} -proc {ps7_cortexa9_1} -runtime {cpp} -arch {32-bit} -support-app {hello_world}
platform generate -domains 
platform active {new_platform}
domain active {zynq_fsbl}
domain active {standalone_domain}
domain active {standalone_ps7_cortexa9_1}
platform generate -quick
domain active {zynq_fsbl}
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE DDR"
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE DDR"
bsp write
bsp reload
catch {bsp regenerate}
domain active {standalone_domain}
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE DDR"
bsp write
bsp reload
catch {bsp regenerate}
domain active {standalone_ps7_cortexa9_1}
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE DDR -DUSE AMP=1"
bsp write
bsp reload
catch {bsp regenerate}
domain active {zynq_fsbl}
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE DDR"
bsp reload
domain active {standalone_domain}
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE DDR"
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR"
bsp write
bsp reload
catch {bsp regenerate}
domain active {zynq_fsbl}
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR"
bsp write
bsp reload
catch {bsp regenerate}
domain active {standalone_ps7_cortexa9_1}
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR -DUSE_AMP=1"
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR -DUSE_AMP=1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate
domain active {zynq_fsbl}
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR"
bsp write
domain active {standalone_domain}
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR"
bsp write
domain active {standalone_ps7_cortexa9_1}
bsp reload
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR -DUSE_AMP=1"
bsp config dependency_flags "-MMD -MP"
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR -DUSE_AMP=1"
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR -DUSE_AMP=1"
bsp write
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR-DUSE_AMP=1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_ps7_cortexa9_1 
bsp config extra_compiler_flags "-mcpu=cortex-a9 -mfpu=vfpv3 -mfloat-abi=hard -nostartfiles -g -Wall -Wextra -fno-tree-loop-distribute-patterns -DSHAREABLE_DDR -DUSE_AMP=1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_ps7_cortexa9_1 
platform generate
platform generate
