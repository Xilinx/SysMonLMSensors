/******************************************************************************
* Copyright (C) 2022 Xilinx, Inc. All rights reserved.
*******************************************************************************/
# xsysmoni2c
I2C LMSensors Driver for Versal Sysmon Devices

This driver is used for reading the maximum chip temperature on a Versal Sysmon
chip through the I2C interface.

Note: this is an intrim driver that will be replaced with the stock versal-sysmon
driver when the I2C interface is added.

Example usage include:

cat the device:
$ cat /sys/devices/platform/axi/80060000.i2c/i2c-3/3-0018/hwmon/hwmon4/temp1_input

or using sensors:
$ sensors

Device tree node example:
axi_iic_1: i2c@80060000 {
	#address-cells = <1>;
	#size-cells = <0>;
	clock-names = "s_axi_aclk";
	clocks = <&zynqmp_clk 71>;
	compatible = "xlnx,axi-iic-2.1", "xlnx,xps-iic-2.00.a";
	interrupt-names = "iic2intc_irpt";
	interrupt-parent = <&gic>;
	interrupts = <0 92 4>;
	reg = <0x0 0x80060000 0x0 0x10000>;
	i2c@18 {
		reg = <0x18>;
		compatible = "xlnx,xsysmoni2c";
	};
};

