# <center>NOMAD的超级电容模组开源

项目负责人：
<img src="\Picture\Wechat_Head.jpg" alt="Wechat_Head" width="100" height="100">
<img src="\Picture\QQ_Head.jpg" alt="QQ_Head" width="100" height="100">
<img src="\Picture\Bilbili_Head.jpg" alt="Bilbili_Head" width="100" height="100">
(没错都是我)

## <center>前言
本开源文档于2024.5.5开始撰写，提笔于此，感慨万千。笔者于大一上学期入队，自大一暑假接手超电研发任务，至落笔此开源文档已有三个年头。这四年见证了队伍从联盟赛队伍一跃成为超级对抗赛的常驻队伍，笔者的技术、心态也逐渐成熟。依旧记得22赛季的赛场上，训练赛与小组赛受到单方面的薄纱而导致0胜绩大败而归，彼时的沮丧与不甘；依旧记得23赛季联盟赛，小组赛与某一线强队打平，万年弱队的滤镜被打破，彼时的兴奋与激动；依旧记得23赛季对抗赛，小组赛遇上两支国赛4强，对其实力与技术水平的由衷佩服。

笔者自接手超电研发任务以来，研发工作横跨22、23、24三个赛季。22赛季笔者技术水平不足，对队伍无实际贡献，因此22赛季的超电系购买淘宝某某创新的超级电容控制器与超级电容组使用；23赛季笔者借鉴中南大学FYT战队于CSDN的开源，研制一套可切换功率路径的串联降压方案，将其应用于赛场，并进行并联式双向四开关方案的预研；24赛季笔者对于并联式双向四开关的研制工作已经结束，并计划将其投入赛场。

本开源内容即笔者研制的并联式双向四开关方案的具体实现。纯属处于笔者的喜好，喜欢对自己的每个有代表意义的作品命名，该项目的超级电容控制器被命名为“Adernal”。**本开源项目不属于战队开源，属于个人开源**。本开源作为笔者的首次开源，将受到持续不定时的维护。由于笔者技术水平与精力有限，文档内的表述难免出现不恰当或者错误的表述(赛博口糊与赛博嘴瓢)，望各位谅解并欢迎指出。

## <center>控制器硬件设计
**该项目的物料选型请以BOM表为准，原理图中的物料仅供参考（仅仅封装与BOM表内物料是一样的）。**  
完整的硬件设计请参考PCB项目工程文件，本文档仅对项目中的关键硬件设计进行解释。
### 方案选择
笔者的研发工作进行过程中，RoboMaster高校系列赛的赛场上的超级电容控制器方案已近成熟。高校战队自行研制的超级电容控制器的硬件方案大致可划分为以下三种：

- 串联式降压/升压方案

<img src="\Picture\TandemElevatorLoweringProgram.png" alt="TandemElevatorLoweringProgram">

该方案被认为是出现在RoboMaster高校系列赛赛场上最古早的硬件方案。来源于裁判系统电源管理模块Chassis电源接口的电流经过恒流降压部分为超级电容组恒功率（等于裁判系统功率限制）充电。电容组（可能有恒流降压输出）的能量经过一升压部分传递给底盘动力系统。此外，存在一电子开关将裁判系统Chassis电源与底盘动力系统直接连接，当电容组电压足够低导致升压部分无法正常工作时将升压部分失能同时导通电子开关。

笔者认为该方案的优点为搭建难度低，可通过选购适合的淘宝成品模块即可搭建一个可实现基本功能的硬件平台；控制难度低，控制器仅需控制恒流降压部分的使能与充电电流、升压部分的使能、电子开关的使能即可实现基本功能。该方案的缺点为硬件平台的体积过大，即便将所有硬件功能部分集成于单板、叠板，其体积对于结构组而言也是难以接受的；此外，在电容组电压较低的工况下，由于恒流降压部分与升压部分其输入、输出两端的压差较大，因此导致两者效率降低，从而导致该方案整体的能量转化效率较低，该方案在早期的RoboMaster高校系列赛受到普遍使用的原因为当时电容组的最高能量不受限制。

- 并联式四开关方案

<img src="\Picture\ParallelFourSwitchProgram.png" alt="ParallelFourSwitchProgram">

该方案被普遍认为是当前RoboMaster高校系列赛赛场上的“版本答案”。将裁判系统Chassis电源串联一个ORing（这是可以省去的）后与底盘动力系统连接，将FSBB（Four-Switch-BuckBoost）与底盘动力系统所处网络并联，并将FSBB另一端与超级电容组连接。当底盘动力系统消耗功率低于裁判系统限制，FSBB将为超级电容组恒功率充电以保证裁判系统Chassis电源的输出功率与裁判系统功率限制保持一致；当底盘动力系统消耗功率高于裁判系统限制，FSBB将通过超级电容组的恒功率放电以保证裁判系统Chassis电源的输出功率与裁判系统功率限制保持一致。

笔者认为该方案的优点为：从功率、效率方面看，该方案性能足够高，是所有主流方案中最靠近规则限制下超级电容控制器理论最优性能的方案。该方案的缺点为：研发难度较高，存在微控制器编程与电子电路设计两大难点，并且在实际开发中，这两个难点具有较高的耦合，这要求研发者对电子电路设计与微控制器编程都具有一定的造诣（从另一个角度看可以算是优点，对这个方案的超电进行独立研发确实很锻炼人）。笔者当初想挑战自我，故将超级电容控制器的硬件方案定为该方案。

- 并联式两开关方案

<img src="\Picture\ParallelTwoSwitchprogram.png" alt="ParallelTwoSwitchprogram">

该方案被认为是并联四四开关方案的简化版本。将裁判系统Chassis电源串联一个ORing（这是可以省去的）后与底盘动力系统连接，将单半桥变换器与底盘动力系统所处网络并联，并将单半桥变换器另一端与超级电容组连接。当底盘动力系统消耗功率低于裁判系统限制，单半桥变换器将为超级电容组恒功率充电以保证裁判系统Chassis电源的输出功率与裁判系统功率限制保持一致；当底盘动力系统消耗功率高于裁判系统限制，单半桥变换器将通过超级电容组的恒功率放电以保证裁判系统Chassis电源的输出功率与裁判系统功率限制保持一致。

笔者认为该方案的优点为搭建复杂度低于并联式四开关方案，相对于并联式四开关，从硬件角度看，该方案仅需要对一组半桥进行控制，从软件角度看，该方案无需考虑电容组电压处于电池电压附近时的控制问题；此外，该方案从理论上具有高于四开关的能量效率。该方案的缺点为：性能上限没有并联式四开关方案高；此外额外需要解决机器人死亡后底盘断电，电容组电压从半桥上管体二极管窜入母线导致“僵尸车”的问题。

### 关键物料选型
对于FSBB上的无源器件，笔者选型如下：

|物料类型|型号|
|:-----:|:--:|
|  C  |ZE11000UF35V119EC0014|
|   L   |BCIHP2213-150M|

其中，ZE11000UF35V119EC0014为容值为1000uF，20℃、100kHz下的ESR不高于8mR的高性能固态电容器；BCIHP2213-150M为感值为15uH，DCR典型值为5.62mR，额定电流为23.5A的一体成型功率电感。根据上述选型可以了解到电感纹波电流最高约为2.5A，输入/输出纹波电压最高不超过105.625mV。

对于半桥及驱动，笔者选型如下：
|物料类型|物料|
|:-----:|:--:|
|  MOS  |STL130N6F7|
|Driver |2EDF7275KXUMA1|

其中，STL130N6F7为Qg为42nC，Rds(ON)典型值为3mR的高性能N型开关管，具有不错的直流导通性能与开关性能；2EDF7275KXUMA1为灌电流8A，拉电流4A的隔离栅极驱动器，可以提供稳定的ns级的上升/下降时间。

>就目前行情来看，笔者更加推荐使用CSD18540作为开关管使用，该开关管虽然从正规渠道购买难度较大且价格高昂，但是淘宝部分商家售卖的这个管子报价为0.3RMB一颗，笔者听说该渠道的这个管子良品率高达70%(截止于2024上半年的消息)，且CSD18540的直流性能与开关性能都强于STL130N6F7。

对于模拟信号检测，笔者选型如下：

|物料类型|物料|
|:-----:|:--:|
|Sensor1|INA197AIDBVT|
|Sensor2|INA240A1DR|
|Opamp|COS722SR|
|Reference|CJ431|

其中，INA197AIDBVT为共模电压高达80V，放大倍数为50，带宽为300kHz的单向检测电流感应放大器，被放置于高侧检测母线电流；INA240A1DR为共模电压高达80V，放大倍数为20，带宽为400kHz，内置PWM抑制的双向检测电流感应放大器，被放置于高侧检测DCDC两端电流；COS722SR为供电电压在2.1V-5.5V，压摆率为7.2V/us的轨到轨运算放大器，被用来检查母线与电容组的电压；CJ431为供电电压高达36V，低至200mR的动态输出阻抗的电压基准芯片。

>当前，INA197的价格可能是绝大部分队伍难以接受的，笔者也是趁着该芯片价格低廉时购买。笔者在该项目中将该芯片作为母线功率环路反馈的提供者。考虑到裁判系统仅以10Hz反馈Chassis功率，因此母线电流的电流感应放大器可以认为仅需保证具有足够高的共模电压以满足在RM赛事工况下不会对芯片造成损坏，从这个角度来看，国内外芯片厂具有更多性价比之选。笔者不建议通过低侧采样的方式检测母线电流，考虑到RM赛事中超级电容控制器与超级电容组之间存在裁判系统超级电容管理模块，该模块与裁判系统电源管理模块之间存在着4Pin通讯线的直接连接（包含GND），不排除电流从4PIN线直接回流至裁判系统电源管理模块的可能性，导致低侧采样出现失准。

>针对于FSBB两端的电流采样，笔者认识到PWM抑制功能的用处是有限的。在INA240的数据手册-典型应用中，INA240被放置在半桥的SW网络检测无刷电机三相线的电流或螺线管的电流，但是INA240不适合放置在超级电容控制器的SW网络。下图为INA240数据手册中展示PWM抑制能力的曲线图，考察可知发生共模电压阶跃后输出反馈的失准时间约为1us，以该项目超级电容控制器为例，开关频率为160kHz，上升沿失准与下降沿失准占据整个开关周期约32%，半桥占空比较大或较小可能导致INA240无法反馈准确的电流值。(还在用INA240的原因是懒得改了)笔者在此更推荐采用低侧电流检测方案并直接使用运放搭建采样电路，可以更好的降低成本并锻炼开发人员的模拟电路搭建调试能力。
<img src="\Picture\INA240_PWMreject.PNG" alt="INA240_PWMreject">


在该项目中，以下杂散物料的选型也值得一谈：
|物料类型|物料|
|:-----:|:--:|
|   C   |CL21B473KBCNNNC|
|MicroControllerUnit|STM32G474CET6|

其中，CL21B473KBCNNNC为三星的容值47nF的MLCC，笔者在迭代过程中发现半桥SW噪声约为40-50MHz，该噪声耦合到整板的电压轨与信号网络导致超级电容控制器工作出现异常。笔者了解到该MLCC在30-70Mhz下具有30~50mR的阻抗（见下图）,在关键位置的电源轨铺设该MLCC，提高了整机工作的稳定性；STM32G474系意法半导体推出的主流微控制器，D-Power产品组合的一员，专门针对数字电源相关应用，例如D-SMPS、照明、焊接、太阳能系统逆变器及无线充电器。内置高分辨率定时器可提供高达184ps的分辨率，为电源提供精确电流、功率控制。

<img src="\Picture\CL21B473KBCNNNC-f-Z.PNG" alt="CL21B473KBCNNNC-f-Z">


### 关键电路设计
原理图中物料型号仅作为参考。

- 半桥驱动电路

<img src="\Picture\Driver_Sch.PNG" alt="Driver_Sch">

对于半桥驱动器的控制部分，内置LDO未启用，电源轨直接使用数字信号的3V3电源；对于半桥驱动器的驱动部分，高侧N型开关管直接使用自举电路驱动，栅极驱动输出与开关管通过R1,R2电阻连接，用处是吸收Vgs振荡与减缓开关管的开启/关断时间以抑制电磁问题。R5、C13为RC组合，用于吸收半桥开关时SW网络的振荡能量，R、C值的敲定遵从[Analog Design Journal-Slyt740-Reducing noise on the output of a switching regulator](https://www.ti.com/lit/an/slyt740/slyt740.pdf?ts=1702068348410&ref_url=https%253A%252F%252Fwww.bing.com%252F)。
<br>
- 缓启动/防反接电路

<img src="\Picture\PwrSafe_Sch.PNG" alt="PwrSafe_Sch">

电源硬件缓启动依托P型开关管的弥勒效应实现。当VCC_bat电压上升，VCC_bat通过R7、R10给C16充电，延长Q6(NPN-bjt)开启时间，Q6开启过程中，R6两端压差逐渐增大，Q5的Rds快速减小，当VCC24_H网络有0电压迅速上升至VCC_bat，经过C17与D6为P型开关管的栅极补充电荷，延长米勒平台。在Q5完全开启后，母线上任何电压尖峰的能量都由D4(TVS)消耗。  
防反接通过通过低侧N型开关管实现。当VCC24_H上升并保持稳定，经过R13、R14分压，Q7正常导通。当发生反接，GND_bat网络为电池电压，Vgs为0，Q7阻隔反向高压，实现防反接功能。考虑到未反接时，VCC24_H电压下降导致Q7挂断，此时GND为浮地，因此在GND与GND_bat间连接RC将GND网络下拉至GND_bat（别骂了笔者现在也觉得没啥用），反接时，电流从GND_bat经过该电阻与D7返回VCC_bat网络，板载芯片承受的负压仅为D7的正向偏置电压。
该电路提供Multisim仿真文件。
<br>
- 信号调理

<img src="\Picture\INA240_Sch.PNG" alt="INA240_Sch">

以INA240的电流检测电路为例，检流电阻两端经过R58、R59、C48、C49、C50到达INA240的差分采样引脚。R58、C48与R59、C50这两个RC组合用于抑制半桥SW噪声耦合到电源轨上导致的共模瞬变，C48、C50需要连接至模拟信号参考层，C49用于差模滤波与抑制共模滤波器一致性偏差导致的差模误差。  
3V3电压基准经过L8（磁珠）与C27，经过INA240内部电阻分压为双向电流检测提供电压偏置。
>之所以为带有PWM抑制功能的INA240添加共模RC滤波器的原因在"关键物料选型"小节有所提及，下图截取与INA240的用户手册，对于5R的外部电阻，增益误差仅为0.17%，因此依旧能维持足够高水平的一致性。
><img src="\Picture\INA240_OutsideCommonModeFilter.PNG" alt="INA240_OutsideCommonModeFilter">

### Layout

<img src="\Picture\Layer1.PNG" alt="Layer1">
<img src="\Picture\Layer2.PNG" alt="Layer2">
<img src="\Picture\Layer3.PNG" alt="Layer3">
<img src="\Picture\Layer4.PNG" alt="Layer4">

在PCB板上，功率部分与控制部分被严格分隔，功率线通过区域填充增加宽度保证了足够的承载电流能力。功率区域中，1层铺地用于提高散热能力，2层铺地尽可能完整用以抑制杂散噪声并屏蔽其对下层信号的影响，三层铺地用于辅助散热。4层地为功率地，布置为“工字形”以保证FSBB工作过程中尽可能小的回路面积，以减小对外的电磁干扰。  
在控制区域，大部分信号器件贴装于1层，1层铺铜为数字信号参考层；2层为数字信号电源层，3层为模拟信号电源层，4层为模拟信号参考层。功率与信号的参考层通过与降压路径相同路径的走线进行单点连接，数字、模拟信号电源轨通过放置在微控制器模拟电源引脚附近的集中放置的过孔进行单点连接。

### 整体效果
<img src="\Picture\HardwareOverview.jpg" alt="HavdwareOverview">

## <center>控制器软件设计
由于该项目中硬件与软件的耦合度较高，笔者对软件的开发伴随着硬件的研发工作持续了约1年的时间，不同部分的代码风格存在一定的变化。  
值得注意的是，由于控制方案的原因，本开源项目中提供的控制器软件设计并不能很好的发挥出控制器硬件的全部性能，详细的优化建议已在下文给出，根据优化建议对控制方案进行改进后，理论上将能发挥出控制器硬件的全部性能。  
STM32工程中，需要单独将LowOptZone.c与Safety.c文件的优化等级设置为-O0，以避免控制执行出现异常的现象。  
完整的硬件设计请参考STM32项目工程文件，本文档仅对项目中的关键软件设计进行解释。
### 环路控制
<img src="\Picture\LoopCtrl_Simulink.PNG" alt="LoopCtrl_Simulink">
环路控制方案的框图如上图所示。  
主环路为PID-FF-PID环路。最外层环路为母线（裁判系统Chassis电源输出）的功率PID环路，理想状态下，其期望恒定为裁判系统功率限制，反馈量为当前母线的实时功率，控制量为FSBB连接母线一端的期望功率（正负代表充电与放电）。

中间的FF环节将FSBB母线端期望功率( $P_s$ )转为电容组一端的期望电流( $I_c$ )。假定FSBB效率恒为100%（即 $P_s = P_c$ ， $P_c$ 为电容组当前充放电功率）， $I_c$ 与 $P_s$ 的关系可以通过下式直接表示：
$$I_c = \frac{P_c}{U_c} = \frac{P_s}{U_c}$$
该式被认为是简单模型，将上式引入控制环路后发现， $I_c$ 稳定后的收敛值准确，但是 $P_s$ 变化幅度较大时， $I_c$ 值出现抖动收敛的现象，其曲线类似于带阻尼的谐波曲线。  
推测上述现象是电容组的ESR导致， $P_s$ 变化后， $I_c$ 依上式发生变化，由于电容组存在ESR， $I_c$ 变化往往会导致 $U_c$ 短时间发生较大变化，因而下一次环路控制根据变化后的 $U_c$ 对 $I_c$ 进行调整。上述过程反复进行，其结果就是 $I_c$ 抖动并逐渐收敛至恒定值。解决抖动问题的方法就是将在控制模型中考虑电容组的ESR。修改简单模型为：
$$I_c = \frac{P_s}{U_c - I_{Clast}·ESR + I_c·ESR}$$
其中， $I_{Clast}$ 为上一次环路控制中FF输出的电容组电流期望值，整理得：
$$I_c = \frac{P_s}{U_c + ESR·(I_c - I_{Clast})}$$
上式被称之为ESR模型。欲求出 $I_c$ 的解析解，需要通过求根公式对二元一次方程进行求解，求出的解析解形式复杂，对微控制器运算压力较大，因此需要对上式进行简化以描述 $P_s$ 转成 $I_c$ 的过程。  
将简单模型 $I_c = \frac{P_s}{U_c}$ 引入上式中等号右边分母的 $I_c$ ，将上式简化为：
$$I_c = \frac{P_s}{U_c + ESR·(\frac{P_s}{U_c} - I_{Clast})}$$
上式被称之为简化后的ESR模型。将上式引入控制环路， $I_c$ 抖动问题得到了很好的解决，实际的 $P_s$ 可以在足够短的时间内达到预设的期望值，拥有远优于PID控制器的响应速度。但由于简化后的ESR模型不能很好的描述FSBB与电容组实际的表现，在电容组电压过低时， $P_s$ 的变化会为 $I_c$ 的曲线带来一定程度的“超调”（当然还是会收敛），在模型输入端添加数字滤波器可以很好的解决“超调”问题。理论上 $P_s$ 反馈与预设期望之间的误差与FSBB效率有很强的关联性，在本开源项目中提供的硬件平台下，若预设期望为205W，反馈值约为203W左右。

FF环节的输出作为最内环的期望值，最内环为电容组电流PID环路(正负代表电荷流动方向)，其反馈量为当前电容组的实时电流，控制量为广义占空比。

>简化后的ESR模型应用于环路控制过程中，需要对代入ESR模型的简单模型进行限幅，FF输出的电容组电流期望值也需要进行限幅，两者的限幅理应相同

>以下讨论未经实际测试（纯口嗨），请读者自行判断可行性。  
>ESR模型的表达式从形式上被认为适合使用数值迭代法得到更为精确的结果，其迭代方式类似于递归。根据ESR模型的表达式构造函数：
>$$f(x)=I_c = \frac{P_s}{U_c + ESR·(x - I_{Clast})}$$
>设定初始值 $x=\frac{P_s}{U_c}$ ，迭代一次的结果为：
>$$f(\frac{P_s}{U_c}) = \frac{P_s}{U_c + ESR·(\frac{P_s}{U_c} - I_{Clast})}$$
>上述结果与简化后的ESR模型具有相同的表达式。迭代两次的简要表达如下：
>$$f[f(\frac{P_s}{U_c})]$$
>以此类推，通过有限次的迭代，可使得计算结果与 $I_C$ 的理想期望值之间的误差小于任意的大于零的实数特定值，这种数值迭代的算法从理论上可以很好的解决电容组电压过低导致的输出失准，且不会带来部署滤波器导致时滞的问题。算法的运算时间与迭代次数近似呈正比，过多的迭代次数会对CPU带来一定程度的压力。

分析FSBB电路可以得到以下结论：
$$\frac{U_1}{U_2} = \frac{D_2}{D_1}$$
其中， $U_1$ 、 $U_2$ 分别为FSBB的1、2两端的电压值， $D_1$ 、 $D_2$ 分别为FSBB的1、2两边半桥上管PWM的duty。  
定义一 $\alpha$ 值，使其满足以下关系：
$$\alpha = \frac{U_1}{U_2} = \frac{D_2}{D_1}$$
定义FSBB的2端与母线连接（与底盘动力系统的电源轨并联），1端与电容组连接， $U_2$ 被认为是相对恒定的，因此可以得出在理想状态下 $U_1$ 与 $\alpha$ 呈正比的结论；与此同时， $\alpha$ 可以直接拆分成 $D_1$ 与 $D_2$ 。根据 $\alpha$ 的上述特性，笔者认为 $\alpha$ 很适合作为主环路的控制量， $\alpha$ 被定义为广义占空比。环路输出广义占空比后，被拆分成两边半桥的duty并更新半桥的占空比，这种控制方案被认为是解决电容组电压在电池电压附近导致存在振荡现象的有效手段。  
在实际进行的超电控制过程中，每次对FSBB提供PWM前都被建议为主环路的电容组电流环路预载积分值，预载积分值具体大小为： $\frac{U_c}{U_b}$ 。其中， $U_c$ 为当前电容组电压值， $U_b$ 为当前母线电压值，这种做法可以很好的解决FSBB于初始阶段存在短时大电流的问题。

主环路将与高电压环与低电压环竞争，高电压环与低电压环的反馈量都为电容组电压，控制量都为广义占空比。高电压环的期望为电容组的最高电压，低电压环期望为电容组的最低电压。在微控制器输出PWM前，两者都被建议预载积分值，高电压环的预载值为其积分限幅的上限，低电压环的预载值为其积分限幅的下限。

取主环路、高电压环、低电压环三者广义占空比输出的中位数，中位数对应的环路被认为竞争成功，拆分为实际占空比更新半桥duty。

>实际上，上述控制方案仍存在一定问题。若高电压环竞争成功而主环路竞争失败，主环路的所有PID环节的积分值将会持续增至积分限幅，若欲重新令主环路竞争成功，需要消耗一定的时间使积分值下降，直到主环路输出低于高电压环输出，低电压环与主环路的竞争同理，上述过程导致FSBB在特定状况下具有极慢的响应。笔者认为存在更优控制方案。需要注意的是以下对更优控制方案的讨论未经实际测试（纯口嗨），请读者自行判断可行性。
>考虑到RoboMaster高校系列赛中，裁判系统会以10Hz的频率通过串口向主控提供缓冲能量值，笔者认为可以将主环路的功率PID环替代为对缓冲能量的FF控制。定义30J为FF控制的期望值， $E_0$ 为微控制器获取到当前裁判系统反馈的缓冲能量， $E_1$ 为上一次裁判系统反馈的缓冲能量。欲保证下一次裁判系统反馈的缓冲能量为30J（维持在期望）,可以将FF控制的输出（FSBB连接母线一端的期望功率）更新为：
>$$P_s = \frac{2·E_0 - E_1 - 30J}{100ms} + P_{slast}$$
>其中， $P_{slast}$ 为上一次环路控制过程中该FF环节的输出。该式的推导不再赘述。
>笔者意识到功率环为主环路最外环的控制方案存在问题却依旧采用，原因是笔者原所处战队的历史遗留问题，战队原来很长一段时间根据缓冲能量剩余值为底盘动力系统的转矩电流乘以适当比例以实现功率控制，笔者对缓冲能量的控制在当时是不被接受的；其次，笔者对控制方案进行Simulink仿真过程中，仿真结果未出现该问题（不知道是什么原因）；最后一点是笔者没精力改了()

为了保证FSBB的效率尽可能高，对广义占空比拆分为半桥实际占空比的规则提出要求： $U_c$ 小于 $U_b$ 时，与电容组电源轨网络连接的半桥占空比为最高值； $U_c$ 大于 $U_b$ 时，与母线电源轨网络连接的半桥占空比为最高值；两边半桥上管同时开启的模态持续时间尽可能长。

环路控制方案提供Simulink仿真文件。考虑到不同版本Matlab对于打开Simulink仿真文件的一些妙妙特性，特此告知笔者通过R2022a版本的Matlab创建该".slx"文件，适配其余版本Matlab的".slx"文件恕笔者不再提供。

### 功能实现

- AlarmSystem

依托定时器上溢中断实现，同时提供多达32个非阻塞、非精确的以ms为单位的倒计时服务。该功能系项目初期实现，用于为超级电容控制器的工作任务提供时间片，对单次事件进行计时。
<br>
- Display  

更新板载OLED屏幕显示。反馈电容组电压、工作模式、安全信息等内容。
<br>
- LoopCtrl  

为FSBB提供10kHz的环路控制，FSBB的开关频率为160kHz。
<br>
- Mode  

设定三种控制FSBB的模式，分别为Silent、Work、Charge。各模式具体机制参阅《Adrenal用户手册》。FSBB的参数设定于此，最高充放电功率设定为200W，电容组一端最高充放电电流设定为13.5A。
<br>
- UsrMsg  

根据与机器人主控通过CAN帧通讯所遵守的协议，对接收到的CAN帧解包，对欲发送的信息进行封包。
<br>
- Safety  

超级电容控制器对自身的安全检查。检查项目包括8类，每类检测项根据安全事件的严重程度分为5个等级。安全管理的具体机制参阅《Adrenal用户手册》。

## <center>电容组设计

笔者认为，在RoboMaster高效系列赛的工况下，对于并联式四开关方案的控制器而言，超级电容组遵循以下以下设计规则将会得到最高的性能。

1. 超级电容组充满时，其两端电压值为30V。
2. 超级电容组充满时，其存储能量值为2kj。
3. 超级电容组的ESR需保证尽可能小。

根据以上设计规则，笔者将型号为WTR3V050F0Z-1840L的物料作为电容单体，将其按照11s1p的连接方式进行连接。  
WTR3V050F0Z-1840L为容值为50F，耐压为3V，ESR为22mR，额定电流为12A的超级电容单体。其组成的超级电容组容值为4.54F，耐压为33V，ESR为242mR。
当该超级电容组充满至30V时，其存储能量为：
$$E = \frac{1}{2}·C·U^2 = 2045.45J$$

电容组的均衡电路依托BW6103芯片实现。BW6103为是一款超级电容充电保护芯片，它内置高精度基准，确保输出精度达到±1%。内置的功率管使得过充保护后的泄流能力达到 0.2A@(VIN=2.95V)，很好地满足了超级电容级联使用时的充电特性。

考虑到200mA的泄放电流对于RoboMaster高校系列赛工况而言存在泄放能力不足的情况，考虑使用外置N型开关管AO3400A串联电阻作为主要泄放回路，BW6103此时仅用于泄放回路的触发。

提供超级电容组的PCB工程文件。

<img src="\Picture\Bank_Overview.jpg" alt="Bank_Overview">

## <center>实际表现

[麦轮原地陀螺](https://www.bilibili.com/video/BV1j6TweaEU2/?vd_source=5b2eb473ca9053cf0f2716bc338a787a)

## <center>后记
<!-- 感谢开源，感谢群友，感谢支持陪伴的人-->
落笔于此。笔者回想起大一入队，大二大三持续在队内干活的场景，那时候就想着，大学时光为这个比赛付出了这么多，总想留下一些什么东西不至于被忘记，故将这套超级电容控制器整理后开源。当各位看到这个文档的时候，笔者就摘下了RMer这个称号，笔者回顾自己的的RM个人生涯，倍感五味杂陈，内心再有万般不舍，也知道自己该走了，给更年轻的人腾位置，前方也有新的位置等着我。

感谢环节（不分先后）：  
感谢马斯克开源(不是  
感谢原大连理工大学凌bug战队的“光速翼”石澄今同学首次开源的《基于STM32G474的双向超级电容控制器》，笔者首次看其开源大受震撼，为笔者的开源项目的硬件方案奠定了基础。  
感谢原深圳大学RoboPilot战队的林子杰同学开源的《基于STM32F334的超级电容控制器》，其开源中的部分Layout思路解决了笔者画板时候的困惑，与其交流也收获颇丰。  
感谢西交利物浦大学GMaster战队的超级电容控制器开源，其开源中的数字滤波方案被笔者在本开源中很好的沿用。  
感谢原河北工业大学山海Mas战队的“Sirius-RX”同学，笔者时常与该同学交流超级电容控制器的研发心得，虽然其本人也独立开源研发成果且具有足够高的质量但是好像没发到RMbbs上面，因此似乎没什么人气，欢迎前去[围观](https://github.com/Sirius-RX/SP_Ultra_CAP/tree/main)。  
感谢香港科技大学ENTERPRIZE战队开源的《数控超级电容方案开源》，其开源中详细的安全处理部分很好的启发笔者。  
感谢广州城市理工学院野狼队的“空格”同学，请我吃了寿司和薯条，为我的研发工作提供能量（物理上的）。  
也感谢RM硬件交流群（826872833）的各位活跃群友，在笔者研发过程中，经常在群里问各种问题，吃百家饭。笔者有今天的技术水平与研发结果离不开各位群友的慷慨喂饭。  

## 补充

### 授权1

兹授权“深圳斯特埃科技有限责任公司”使用本开源项目用以商业目的。

授权期限：  2025  年 4 月 5 日至  2026  年 4 月 5 日。
