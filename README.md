# E2M 模板式开发工具测试上位机工程

> 本工程为 SOEM + imgui 搭建、用于测试 E2M 工具生成代码的可行性

开发板端代码仓库：

[https://github.com/POMIN-163/sdk-bsp-rzn2l-etherkit-e2m-test](https://github.com/POMIN-163/sdk-bsp-rzn2l-etherkit-e2m-test)

## 测试截图

![-](image/test.png)

## 开发板端（仅在EtherKit测试通过）生成代码例子

```c
// ===== EtherCAT → Modbus RTU 映射表（自动生成）=====
// 生成时间: 2025-11-29 00:12:38
// 共 10 个对象

e2m_mdata_t mdata_list[] = {
    { 0x3000, 0x01, 0x0000, "状态字" },  // 设备1运行状态
    { 0x3001, 0x01, 0x0001, "转速设定" },  // 目标转速
    { 0x3002, 0x01, 0x0002, "实际转速" },  // 当前转速
    { 0x3003, 0x01, 0x0003, "运行电流" },  // 电流反馈
    { 0x3004, 0x02, 0x0010, "阀门开度" },  // 调节阀开度
    { 0x3005, 0x02, 0x0001, "管道压力" },  // 压力值
    { 0x3006, 0x03, 0x0002, "环境温度" },  // 温度传感器
    { 0x3007, 0x01, 0x0005, "故障代码" },  // 报警信息
    { 0x3008, 0x01, 0x0004, "运行时间高" },  // 累计运行时间高16位
    { 0x3009, 0x01, 0x0009, "运行时间低" }  // 累计运行时间低16位
};

UINT16 mdata_count = 10;

// ==============================================
```

## 主站端生成代码例子

```c
// ===== 主站 SDO 对象配置（自动生成）=====
// 生成时间: 2025-11-29 00:12:38
// 共 10 个对象

static SDO_Object sdo_objects[] = {
    { 0x3000, u8"状态字", 0, "", true }, // 设备1运行状态  设备 1 寄存器 0
    { 0x3001, u8"转速设定", 0, "", true }, // 目标转速  设备 1 寄存器 1
    { 0x3002, u8"实际转速", 0, "", true }, // 当前转速  设备 1 寄存器 2
    { 0x3003, u8"运行电流", 0, "", true }, // 电流反馈  设备 1 寄存器 3
    { 0x3004, u8"阀门开度", 0, "", true }, // 调节阀开度  设备 2 寄存器 16
    { 0x3005, u8"管道压力", 0, "", true }, // 压力值  设备 2 寄存器 1
    { 0x3006, u8"环境温度", 0, "", true }, // 温度传感器  设备 3 寄存器 2
    { 0x3007, u8"故障代码", 0, "", true }, // 报警信息  设备 1 寄存器 5
    { 0x3008, u8"运行时间高", 0, "", true }, // 累计运行时间高16位  设备 1 寄存器 4
    { 0x3009, u8"运行时间低", 0, "", true } // 累计运行时间低16位  设备 1 寄存器 9
};

static const int sdo_count = 10;

// ==============================================
```

## ESI 生成例子（OD节点）

```xml
			<!-- ==== CUSTOM MODBUS MAPPING OBJECTS START ==== -->
    <Object>
        <Index>#x3000</Index>
        <Name>状态字</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>设备1运行状态</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3001</Index>
        <Name>转速设定</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>目标转速</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3002</Index>
        <Name>实际转速</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>当前转速</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3003</Index>
        <Name>运行电流</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>电流反馈</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3004</Index>
        <Name>阀门开度</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>调节阀开度</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3005</Index>
        <Name>管道压力</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>压力值</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3006</Index>
        <Name>环境温度</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>温度传感器</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3007</Index>
        <Name>故障代码</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>报警信息</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3008</Index>
        <Name>运行时间高</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>累计运行时间高16位</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <Object>
        <Index>#x3009</Index>
        <Name>运行时间低</Name>
        <Type>UINT</Type>
        <BitSize>16</BitSize>
        <Info>
            <Description>累计运行时间低16位</Description>
        </Info>
        <Flags>
            <Access>rw</Access>
        </Flags>
    </Object>
    <!-- ==== CUSTOM MODBUS MAPPING OBJECTS END ==== -->
```

## 使用方式

仅需使用 pip 安装 Jinja 工具后，**进入 e2m 目录中**运行 python3 e2m_bridge_gen.py，即可得到 output 目录下的生成代码，复制到开发板端工程、TwinCAT ESI 路径即可使用

