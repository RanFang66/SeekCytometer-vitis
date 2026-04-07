# 
# Usage: To re-create this platform project launch xsct with below options.
# xsct E:\seek_cytometer\software\cytometer_platform\platform.tcl
# 
# OR launch xsct and run below command.
# source E:\seek_cytometer\software\cytometer_platform\platform.tcl
# 
# To create the platform in a different location, modify the -out option of "platform create" command.
# -out option specifies the output directory of the platform project.

platform create -name {cytometer_platform}\
-hw {E:\seek_cytometer\hardware\seek_cytometer\cytometer_platform_wrapper.xsa}\
-proc {psu_cortexa53_0} -os {standalone} -arch {64-bit} -fsbl-target {psu_cortexa53_0} -out {E:/seek_cytometer/software}

platform write
platform generate -domains 
platform active {cytometer_platform}
bsp reload
platform generate
bsp reload
bsp setlib -name xiltimer -ver 1.2
bsp setlib -name lwip213 -ver 1.0
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
domain active {zynqmp_fsbl}
bsp reload
bsp reload
bsp config en_interval_timer "true"
bsp config interval_timer "psu_ttc_0"
bsp config sleep_timer "psu_ttc_1"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform clean
platform generate
bsp config sleep_timer "psu_ttc_1"
bsp write
catch {bsp regenerate}
bsp config sleep_timer "psu_ttc_1"
bsp write
platform generate -domains standalone_domain 
bsp config sleep_timer "psu_ttc_1"
bsp write
platform generate -domains 
bsp config sleep_timer "psu_ttc_0"
bsp config interval_timer "psu_ttc_0"
bsp config sleep_timer "psu_ttc_0"
bsp config interval_timer "psu_ttc_1"
bsp config sleep_timer "none"
bsp write
bsp reload
catch {bsp regenerate}
platform generate -domains standalone_domain 
platform active {cytometer_platform}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
domain active {zynqmp_fsbl}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
domain active {zynqmp_fsbl}
domain active {standalone_domain}
bsp reload
bsp reload
bsp config xil_interrupt "true"
bsp write
bsp reload
catch {bsp regenerate}
platform clean
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform clean
platform generate
bsp reload
bsp reload
platform generate -domains 
platform active {cytometer_platform}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
domain active {zynqmp_fsbl}
bsp reload
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform clean
platform active {cytometer_platform}
platform generate
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate
platform clean
platform generate
platform generate
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform generate -domains standalone_domain 
platform generate -domains standalone_domain 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform generate -domains standalone_domain 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform generate -domains standalone_domain 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform generate
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform generate -domains standalone_domain 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains standalone_domain 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform active {cytometer_platform}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
platform config -updatehw {E:/seek_cytometer/hardware/scripts/seek_cytometer/cytometer_platform_wrapper.xsa}
platform generate -domains 
