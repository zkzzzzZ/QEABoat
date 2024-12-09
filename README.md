# QEABoat
重庆大学明月科创实验班《定量工程设计方法》大一上学期结课作业 遥控船模

## 使用的模块
|模块|型号|
|:-----------:|:-----------:|
|主控|STM32F103C8T6|
|无线模块(LoRa)|A39C-T400A22D1a|
|震动传感器|SW-18010P|
|角度传感器|ADXL335|
|电调|SURPASS HOBBY 60A WATERPROOF BRUSHLESS ESC|
|电机|SURPASS HOBBY 2845 3800KV WATERPROOF MOTOR|



## 接线表
|MCU引脚|模块引脚|
|:-----------:|:-----------:|
|==========|无线模块|
|PB14|MD0|
|PB15|MD1|
|PB13|AUX|
|PB11(RX3)|TXD|
|PB10(TX3)|RXD|
|==========|电调(PWM)|
|PA8(TIM1_CH1)|ESCLeft|
|PA9(TIM1_CH2)|ESCRight|
|PA10(TIM1_CH3)|备用|
|==========|传感器|
|PA4(ADC1_IN4)|X|
|PA5(ADC1_IN5)|Y|
|PA6(ADC1_IN6)|Z|
|PA7(ADC1_IN7)|震动|
|==========|串口调试|
|PA2(TX2)|RXD(PC)|
|PA3(RX2)|TXD(PC)|

### 另请参阅(如何进行环境配置？)
https://blog.csdn.net/m0_52482021/article/details/140087428<br>
配置完毕后Import CMake project即可。
