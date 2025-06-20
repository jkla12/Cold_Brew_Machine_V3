#ifndef __UART_LCD_H
#define __UART_LCD_H

#include "gd32f30x.h"

#define FIRMWARE_VER 562   /*!< 保持此固件版本号与真实屏幕一致，确保新增功能可用*/
#define CRC16_ENABLE 0     /*!< 如果需要CRC16校验功能，修改此宏为1(此时需要在VisualTFT工程中配CRC校验)*/
#define CMD_MAX_SIZE 64    /*!<单条指令大小，根据需要调整，尽量设置大一些*/
#define QUEUE_MAX_SIZE 128 /*!< 指令接收缓冲区大小，根据需要调整，尽量设置大一些*/

#define SD_FILE_EN 0
#define uint16   unsigned short int
#define uint8    unsigned char
#define uint32   unsigned long
#define uchar    unsigned char
#define int16    short int
#define int32    long

// UART ID定义
#define UART1_ID 1  // 屏幕2
#define UART2_ID 2  // 屏幕1

/*!
*  \brief  检查数据是否符合CRC16校验
*  \param buffer 待校验的数据，末尾存储CRC16
*  \param n 数据长度，包含CRC16
*  \return 校验通过返回1，否则返回0
*/
uint16 CheckCRC16(uint8 *buffer, uint16 n);

/*!
*  \brief  延时
*  \param  n 延时时间(毫秒单位)
*/
void DelayMS(unsigned int n);

/*!
*  \brief  锁定设备配置，锁定之后需要解锁，才能修改波特率、触摸屏、蜂鸣器工作方式
*  \param  uart_id UART ID
*/
void LockDeviceConfig(uint8 uart_id);

/*!
*  \brief  解锁设备配置
*  \param  uart_id UART ID
*/
void UnlockDeviceConfig(uint8 uart_id);

/*!
*  \brief  修改串口屏的波特率
*  \details  波特率选项范围[0~14]，对应实际波特率
*  {1200,2400,4800,9600,19200,38400,57600,115200,1000000,2000000,218750,437500,875000,921800,2500000}
*  \param  option 波特率选项
*  \param  uart_id UART ID
*/
void SetCommBps(uint8 option, uint8 uart_id);

/*!
*  \brief  发送握手命令
*  \param  uart_id UART ID
*/
void SetHandShake(uint8 uart_id);

/*!
*  \brief  设置前景色
*  \param  color 前景色
*  \param  uart_id UART ID
*/
void SetFcolor(uint16 color, uint8 uart_id);

/*!
*  \brief  设置背景色
*  \param  color 背景色
*  \param  uart_id UART ID
*/
void SetBcolor(uint16 color, uint8 uart_id);

/*!
*  \brief  获取颜色
*  \param  mode 模式
*  \param  x X坐标
*  \param  y Y坐标
*  \param  uart_id UART ID
*/
void ColorPicker(uint8 mode, uint16 x, uint16 y, uint8 uart_id);

/*!
*  \brief  清除画面
*  \param  uart_id UART ID
*/
void GUI_CleanScreen(uint8 uart_id);

/*!
*  \brief  设置文字间隔
*  \param  x_w 横向间隔
*  \param  y_w 纵向间隔
*  \param  uart_id UART ID
*/
void SetTextSpace(uint8 x_w, uint8 y_w, uint8 uart_id);

/*!
*  \brief  设置文字显示限制
*  \param  enable 是否启用限制
*  \param  width 宽度
*  \param  height 高度
*  \param  uart_id UART ID
*/
void SetFont_Region(uint8 enable, uint16 width, uint16 height, uint8 uart_id);

/*!
*  \brief  设置过滤色
*  \param  fillcolor_dwon 颜色下界
*  \param  fillcolor_up 颜色上界
*  \param  uart_id UART ID
*/
void SetFilterColor(uint16 fillcolor_dwon, uint16 fillcolor_up, uint8 uart_id);

/*!
*  \brief  显示文本
*  \param  x 位置X坐标
*  \param  y 位置Y坐标
*  \param  back 背景色
*  \param  font 字体
*  \param  strings 字符串内容
*  \param  uart_id UART ID
*/
void DisText(uint16 x, uint16 y, uint8 back, uint8 font, uchar *strings, uint8 uart_id);

/*!
*  \brief  显示光标
*  \param  enable 是否显示
*  \param  x 位置X坐标
*  \param  y 位置Y坐标
*  \param  width 宽度
*  \param  height 高度
*  \param  uart_id UART ID
*/
void DisCursor(uint8 enable, uint16 x, uint16 y, uint8 width, uint8 height, uint8 uart_id);

/*!
*  \brief  显示全屏图片
*  \param  image_id 图片索引
*  \param  masken 是否启用透明掩码
*  \param  uart_id UART ID
*/
void DisFull_Image(uint16 image_id, uint8 masken, uint8 uart_id);

/*!
*  \brief  指定位置显示图片
*  \param  x 位置X坐标
*  \param  y 位置Y坐标
*  \param  image_id 图片索引
*  \param  masken 是否启用透明掩码
*  \param  uart_id UART ID
*/
void DisArea_Image(uint16 x, uint16 y, uint16 image_id, uint8 masken, uint8 uart_id);

/*!
*  \brief  显示裁剪图片
*  \param  x 位置X坐标
*  \param  y 位置Y坐标
*  \param  image_id 图片索引
*  \param  image_x 图片裁剪位置X坐标
*  \param  image_y 图片裁剪位置Y坐标
*  \param  image_l 图片裁剪长度
*  \param  image_w 图片裁剪高度
*  \param  masken 是否启用透明掩码
*  \param  uart_id UART ID
*/
void DisCut_Image(uint16 x, uint16 y, uint16 image_id, uint16 image_x, uint16 image_y,
                  uint16 image_l, uint16 image_w, uint8 masken, uint8 uart_id);

/*!
*  \brief  显示GIF动画
*  \param  x 位置X坐标
*  \param  y 位置Y坐标
*  \param  flashimage_id 图片索引
*  \param  enable 是否显示
*  \param  playnum 播放次数
*  \param  uart_id UART ID
*/
void DisFlashImage(uint16 x, uint16 y, uint16 flashimage_id, uint8 enable, uint8 playnum, uint8 uart_id);

/*!
*  \brief  画点
*  \param  x 位置X坐标
*  \param  y 位置Y坐标
*  \param  uart_id UART ID
*/
void GUI_Dot(uint16 x, uint16 y, uint8 uart_id);

/*!
*  \brief  画线
*  \param  x0 起始位置X坐标
*  \param  y0 起始位置Y坐标
*  \param  x1 结束位置X坐标
*  \param  y1 结束位置Y坐标
*  \param  uart_id UART ID
*/
void GUI_Line(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  画折线
*  \param  mode 模式
*  \param  dot 数据点
*  \param  dot_cnt 点数
*  \param  uart_id UART ID
*/
void GUI_ConDots(uint8 mode, uint16 *dot, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  x坐标等距使用前景色连线
*  \param  x 横坐标
*  \param  x_space 距离
*  \param  dot_y 一组纵轴坐标
*  \param  dot_cnt 纵坐标个数
*  \param  uart_id UART ID
*/
void GUI_ConSpaceDots(uint16 x, uint16 x_space, uint16 *dot_y, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  按照坐标偏移量用前景色连线
*  \param  x 横坐标
*  \param  y 纵距离
*  \param  dot_offset 偏移量
*  \param  dot_cnt 偏移量个数
*  \param  uart_id UART ID
*/
void GUI_FcolorConOffsetDots(uint16 x, uint16 y, uint16 *dot_offset, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  按照坐标偏移量用背景色连线
*  \param  x 横坐标
*  \param  y 纵距离
*  \param  dot_offset 偏移量
*  \param  dot_cnt 偏移量个数
*  \param  uart_id UART ID
*/
void GUI_BcolorConOffsetDots(uint16 x, uint16 y, uint8 *dot_offset, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  将指定的多个坐标点用前景色连接起来
*  \param  dot 坐标点
*  \param  dot_cnt 偏移量个数
*  \param  uart_id UART ID
*/
void GUI_FcolorConDots(uint16 *dot, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  将指定的多个坐标点用背景色连接起来
*  \param  dot 坐标点
*  \param  dot_cnt 偏移量个数
*  \param  uart_id UART ID
*/
void GUI_BcolorConDots(uint16 *dot, uint16 dot_cnt, uint8 uart_id);

/*!
*  \brief  画空心圆
*  \param  x 圆心位置X坐标
*  \param  y 圆心位置Y坐标
*  \param  r 半径
*  \param  uart_id UART ID
*/
void GUI_Circle(uint16 x, uint16 y, uint16 r, uint8 uart_id);

/*!
*  \brief  画实心圆
*  \param  x 圆心位置X坐标
*  \param  y 圆心位置Y坐标
*  \param  r 半径
*  \param  uart_id UART ID
*/
void GUI_CircleFill(uint16 x, uint16 y, uint16 r, uint8 uart_id);

/*!
*  \brief  画弧线
*  \param  x 圆心位置X坐标
*  \param  y 圆心位置Y坐标
*  \param  r 半径
*  \param  sa 起始角度
*  \param  ea 终止角度
*  \param  uart_id UART ID
*/
void GUI_Arc(uint16 x, uint16 y, uint16 r, uint16 sa, uint16 ea, uint8 uart_id);

/*!
*  \brief  画空心矩形
*  \param  x0 起始位置X坐标
*  \param  y0 起始位置Y坐标
*  \param  x1 结束位置X坐标
*  \param  y1 结束位置Y坐标
*  \param  uart_id UART ID
*/
void GUI_Rectangle(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  画实心矩形
*  \param  x0 起始位置X坐标
*  \param  y0 起始位置Y坐标
*  \param  x1 结束位置X坐标
*  \param  y1 结束位置Y坐标
*  \param  uart_id UART ID
*/
void GUI_RectangleFill(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  画空心椭圆
*  \param  x0 起始位置X坐标
*  \param  y0 起始位置Y坐标
*  \param  x1 结束位置X坐标
*  \param  y1 结束位置Y坐标
*  \param  uart_id UART ID
*/
void GUI_Ellipse(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  画实心椭圆
*  \param  x0 起始位置X坐标
*  \param  y0 起始位置Y坐标
*  \param  x1 结束位置X坐标
*  \param  y1 结束位置Y坐标
*  \param  uart_id UART ID
*/
void GUI_EllipseFill(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  设置背光亮度
*  \param  light_level 亮度级别
*  \param  uart_id UART ID
*/
void SetBackLight(uint8 light_level, uint8 uart_id);

/*!
*  \brief  蜂鸣器设置
*  \param  time 持续时间(毫秒单位)
*  \param  uart_id UART ID
*/
void SetBuzzer(uint8 time, uint8 uart_id);

/*!
*  \brief  区域反色
*  \param  x0 起始位置X坐标
*  \param  y0 起始位置Y坐标
*  \param  x1 结束位置X坐标
*  \param  y1 结束位置Y坐标
*  \param  uart_id UART ID
*/
void GUI_AreaInycolor(uint16 x0, uint16 y0, uint16 x1, uint16 y1, uint8 uart_id);

/*!
*  \brief  触摸屏设置
*  \param  enable 触摸使能
*  \param  beep_on 触摸蜂鸣器
*  \param  work_mode 触摸工作模式：0按下就上传，1松开才上传，2不断上传坐标值，3按下和松开均上传数据
*  \param  press_calibration 连续点击触摸屏20下校准触摸屏：0禁用，1启用
*  \param  uart_id UART ID
*/
void SetTouchPaneOption(uint8 enable, uint8 beep_on, uint8 work_mode, uint8 press_calibration, uint8 uart_id);

/*!
*  \brief  校准触摸屏
*  \param  uart_id UART ID
*/
void CalibrateTouchPane(uint8 uart_id);

/*!
*  \brief  触摸屏测试
*  \param  uart_id UART ID
*/
void TestTouchPane(uint8 uart_id);

/*!
*  \brief  自动调节背光亮度
*  \param  enable 使能
*  \param  bl_off_level 待机亮度
*  \param  bl_on_level 激活亮度
*  \param  bl_on_time 偏移量个数
*  \param  uart_id UART ID
*/
void SetPowerSaving(uint8 enable, uint8 bl_off_level, uint8 bl_on_level, uint8 bl_on_time, uint8 uart_id);

/*!
*  \brief  设置当前写入图层
*  \details  一般用于实现双缓存效果(绘图时避免闪烁)：
*  \details  uint8 layer = 0;
*  \details  WriteLayer(layer);   设置写入层
*  \details  ClearLayer(layer);   使图层变透明
*  \details  添加一系列绘图指令
*  \details  DisText(100,100,0,4,"hello hmi!!!");
*  \details  DisplyLayer(layer);  切换显示层
*  \details  layer = (layer+1)%2; 双缓存切换
*  \see DisplyLayer
*  \see ClearLayer
*  \param  layer 图层编号
*  \param  uart_id UART ID
*/
void WriteLayer(uint8 layer, uint8 uart_id);

/*!
*  \brief  设置当前显示图层
*  \param  layer 图层编号
*  \param  uart_id UART ID
*/
void DisplyLayer(uint8 layer, uint8 uart_id);

/*!
*  \brief  拷贝图层
*  \param  src_layer 原始图层
*  \param  dest_layer 目标图层
*  \param  uart_id UART ID
*/
void CopyLayer(uint8 src_layer, uint8 dest_layer, uint8 uart_id);

/*!
*  \brief  清除图层，使图层变成透明
*  \param  layer 图层编号
*  \param  uart_id UART ID
*/
void ClearLayer(uint8 layer, uint8 uart_id);

/*!
*  \brief  显示RTC时钟
*  \param  enable 是否启用
*  \param  mode 模式
*  \param  font 字体
*  \param  color 颜色
*  \param  x X坐标
*  \param  y Y坐标
*  \param  uart_id UART ID
*/
void GUI_DispRTC(uint8 enable, uint8 mode, uint8 font, uint16 color, uint16 x, uint16 y, uint8 uart_id);

/*!
*  \brief  写数据到串口屏用户存储区
*  \param  startAddress 起始地址
*  \param  length 字节数
*  \param  _data 待写入的数据
*  \param  uart_id UART ID
*/
void WriteUserFlash(uint32 startAddress, uint16 length, uint8 *_data, uint8 uart_id);

/*!
*  \brief  从串口屏用户存储区读取数据
*  \param  startAddress 起始地址
*  \param  length 字节数
*  \param  uart_id UART ID
*/
void ReadUserFlash(uint32 startAddress, uint16 length, uint8 uart_id);

/*!
*  \brief  获取当前画面
*  \param  uart_id UART ID
*/
void GetScreen(uint8 uart_id);

/*!
*  \brief  设置当前画面
*  \param  screen_id 画面ID
*  \param  uart_id UART ID
*/
void SetScreen(uint16 screen_id, uint8 uart_id);

/*!
*  \brief  禁用/启用画面更新
*  \details 禁用/启用一般成对使用，用于避免闪烁、提高刷新速度
*  \details 用法：
*  \details SetScreenUpdateEnable(0);//禁止更新
*  \details 一系列更新画面的指令
*  \details SetScreenUpdateEnable(1);//立即更新
*  \param  enable 0禁用，1启用
*  \param  uart_id UART ID
*/
void SetScreenUpdateEnable(uint8 enable, uint8 uart_id);

/*!
*  \brief  设置控件输入焦点
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  focus 是否具有输入焦点
*  \param  uart_id UART ID
*/
void SetControlFocus(uint16 screen_id, uint16 control_id, uint8 focus, uint8 uart_id);

/*!
*  \brief  显示/隐藏控件
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  visible 是否显示
*  \param  uart_id UART ID
*/
void SetControlVisiable(uint16 screen_id, uint16 control_id, uint8 visible, uint8 uart_id);

/*!
*  \brief  设置触摸控件使能
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  enable 控件是否使能
*  \param  uart_id UART ID
*/
void SetControlEnable(uint16 screen_id, uint16 control_id, uint8 enable, uint8 uart_id);

/*!
*  \brief  获取控件值
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void GetControlValue(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  设置按钮状态
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  state 按钮状态
*  \param  uart_id UART ID
*/
void SetButtonValue(uint16 screen_id, uint16 control_id, uchar state, uint8 uart_id);

/*!
*  \brief  设置文本值
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  str 文本值
*  \param  uart_id UART ID
*/
void SetTextValue(uint16 screen_id, uint16 control_id, uchar *str, uint8 uart_id);

#if FIRMWARE_VER>=908
/*!
*  \brief  设置文本为整数值，要求FIRMWARE_VER>=908
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 文本数值
*  \param  sign 0-无符号，1-有符号
*  \param  fill_zero 数字位数，不足时左侧补零
*  \param  uart_id UART ID
*/
void SetTextInt32(uint16 screen_id, uint16 control_id, uint32 value, uint8 sign, uint8 fill_zero, uint8 uart_id);

/*!
*  \brief  设置文本单精度浮点值，要求FIRMWARE_VER>=908
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 文本数值
*  \param  precision 小数位数
*  \param  show_zeros 为1时，显示末尾0
*  \param  uart_id UART ID
*/
void SetTextFloat(uint16 screen_id, uint16 control_id, float value, uint8 precision, uint8 show_zeros, uint8 uart_id);
#endif

/*!
*  \brief  设置进度值
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 数值
*  \param  uart_id UART ID
*/
void SetProgressValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  设置仪表值
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 数值
*  \param  uart_id UART ID
*/
void SetMeterValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  设置仪表值
*  \param  screen_id 画面ID
*  \param  control_id 图片控件ID
*  \param  value 数值
*  \param  uart_id UART ID
*/
void Set_picMeterValue(uint16 screen_id, uint16 control_id, uint16 value, uint8 uart_id);

/*!
*  \brief  设置滑动条
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 数值
*  \param  uart_id UART ID
*/
void SetSliderValue(uint16 screen_id, uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  设置选择控件
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  item 当前选项
*  \param  uart_id UART ID
*/
void SetSelectorValue(uint16 screen_id, uint16 control_id, uint8 item, uint8 uart_id);

/*!
*  \brief  开始播放动画
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void AnimationStart(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  停止播放动画
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void AnimationStop(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  暂停播放动画
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void AnimationPause(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  播放指定帧
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  frame_id 帧ID
*  \param  uart_id UART ID
*/
void AnimationPlayFrame(uint16 screen_id, uint16 control_id, uint8 frame_id, uint8 uart_id);

/*!
*  \brief  播放上一帧
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void AnimationPlayPrev(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  播放下一帧
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void AnimationPlayNext(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  曲线控件-添加通道
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  channel 通道号
*  \param  color 颜色
*  \param  uart_id UART ID
*/
void GraphChannelAdd(uint16 screen_id, uint16 control_id, uint8 channel, uint16 color, uint8 uart_id);

/*!
*  \brief  曲线控件-删除通道
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  channel 通道号
*  \param  uart_id UART ID
*/
void GraphChannelDel(uint16 screen_id, uint16 control_id, uint8 channel, uint8 uart_id);

/*!
*  \brief  曲线控件-添加数据
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  channel 通道号
*  \param  pData 曲线数据
*  \param  nDataLen 数据个数
*  \param  uart_id UART ID
*/
void GraphChannelDataAdd(uint16 screen_id, uint16 control_id, uint8 channel, uint8 *pData, uint16 nDataLen, uint8 uart_id);

/*!
*  \brief  曲线控件-清除数据
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  channel 通道号
*  \param  uart_id UART ID
*/
void GraphChannelDataClear(uint16 screen_id, uint16 control_id, uint8 channel, uint8 uart_id);

/*!
*  \brief  曲线控件-设置视图窗口
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  x_offset 水平偏移
*  \param  x_mul 水平缩放系数
*  \param  y_offset 垂直偏移
*  \param  y_mul 垂直缩放系数
*  \param  uart_id UART ID
*/
void GraphSetViewport(uint16 screen_id, uint16 control_id, int16 x_offset, uint16 x_mul, int16 y_offset, uint16 y_mul, uint8 uart_id);

/*!
*  \brief  开始批量更新
*  \param  screen_id 画面ID
*  \param  uart_id UART ID
*/
void BatchBegin(uint16 screen_id, uint8 uart_id);

/*!
*  \brief  批量更新按钮控件
*  \param  control_id 控件ID
*  \param  state 数值
*  \param  uart_id UART ID
*/
void BatchSetButtonValue(uint16 control_id, uint8 state, uint8 uart_id);

/*!
*  \brief  批量更新进度条控件
*  \param  control_id 控件ID
*  \param  value 数值
*  \param  uart_id UART ID
*/
void BatchSetProgressValue(uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  批量更新滑动条控件
*  \param  control_id 控件ID
*  \param  value 数值
*  \param  uart_id UART ID
*/
void BatchSetSliderValue(uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  批量更新仪表控件
*  \param  control_id 控件ID
*  \param  value 数值
*  \param  uart_id UART ID
*/
void BatchSetMeterValue(uint16 control_id, uint32 value, uint8 uart_id);

/*!
*  \brief  计算字符串长度
*  \param  str 字符串
*  \return 字符串长度
*/
uint32 GetStringLen(uchar *str);

/*!
*  \brief  批量更新文本控件
*  \param  control_id 控件ID
*  \param  strings 字符串
*  \param  uart_id UART ID
*/
void BatchSetText(uint16 control_id, uchar *strings, uint8 uart_id);

/*!
*  \brief  批量更新动画/图标控件
*  \param  control_id 控件ID
*  \param  frame_id 帧ID
*  \param  uart_id UART ID
*/
void BatchSetFrame(uint16 control_id, uint16 frame_id, uint8 uart_id);

#if FIRMWARE_VER>=908
/*!
*  \brief  批量设置控件可见
*  \param  control_id 控件ID
*  \param  visible 是否可见
*  \param  uart_id UART ID
*/
void BatchSetVisible(uint16 control_id, uint8 visible, uint8 uart_id);

/*!
*  \brief  批量设置控件使能
*  \param  control_id 控件ID
*  \param  enable 是否使能
*  \param  uart_id UART ID
*/
void BatchSetEnable(uint16 control_id, uint8 enable, uint8 uart_id);
#endif

/*!
*  \brief  结束批量更新
*  \param  uart_id UART ID
*/
void BatchEnd(uint8 uart_id);

/*!
*  \brief  设置倒计时控件
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  timeout 倒计时(秒)
*  \param  uart_id UART ID
*/
void SetTimer(uint16 screen_id, uint16 control_id, uint32 timeout, uint8 uart_id);

/*!
*  \brief  开启倒计时控件
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void StartTimer(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  停止倒计时控件
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void StopTimer(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  暂停倒计时控件
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void PauseTimer(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  设置控件背景色
*  \details  支持控件：进度条、文本
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  color 背景色
*  \param  uart_id UART ID
*/
void SetControlBackColor(uint16 screen_id, uint16 control_id, uint16 color, uint8 uart_id);

/*!
*  \brief  设置控件前景色
*  \details  支持控件：进度条
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  color 前景色
*  \param  uart_id UART ID
*/
void SetControlForeColor(uint16 screen_id, uint16 control_id, uint16 color, uint8 uart_id);

/*!
*  \brief  显示/隐藏弹出菜单控件
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  show 是否显示，为0时focus_control_id无效
*  \param  focus_control_id 关联的文本控件(菜单控件的内容输出到文本控件)
*  \param  uart_id UART ID
*/
void ShowPopupMenu(uint16 screen_id, uint16 control_id, uint8 show, uint16 focus_control_id, uint8 uart_id);

/*!
*  \brief  显示/隐藏系统键盘
*  \param  show 0隐藏，1显示
*  \param  x 键盘显示位置X坐标
*  \param  y 键盘显示位置Y坐标
*  \param  type 0小键盘，1全键盘
*  \param  option 0正常字符，1密码，2时间设置
*  \param  max_len 键盘录入字符长度限制
*  \param  uart_id UART ID
*/
void ShowKeyboard(uint8 show, uint16 x, uint16 y, uint8 type, uint8 option, uint8 max_len, uint8 uart_id);

#if FIRMWARE_VER>=921
/*!
*  \brief  多语言设置
*  \param  ui_lang 用户界面语言0~9
*  \param  sys_lang 系统键盘语言-0中文，1英文
*  \param  uart_id UART ID
*/
void SetLanguage(uint8 ui_lang, uint8 sys_lang, uint8 uart_id);
#endif

#if FIRMWARE_VER>=921
/*!
*  \brief  开始保存控件数值到FLASH
*  \param  version 数据版本号，可任意指定，高16位为主版本号，低16位为次版本号
*  \param  address 数据在用户存储区的存放地址，注意防止地址重叠、冲突
*  \param  uart_id UART ID
*/
void FlashBeginSaveControl(uint32 version, uint32 address, uint8 uart_id);

/*!
*  \brief  保存某个控件的数值到FLASH
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void FlashSaveControl(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  结束保存控件数值到FLASH
*  \param  uart_id UART ID
*/
void FlashEndSaveControl(uint8 uart_id);

/*!
*  \brief  从FLASH中恢复控件数据
*  \param  version 数据版本号，主版本号必须与存储时一致，否则会加载失败
*  \param  address 数据在用户存储区的存放地址
*  \param  uart_id UART ID
*/
void FlashRestoreControl(uint32 version, uint32 address, uint8 uart_id);
#endif

#if FIRMWARE_VER>=921
/*!
*  \brief  设置历史曲线采样数据值(单字节，uint8或int8)
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 采样点数据
*  \param  channel 通道数
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt8(uint16 screen_id, uint16 control_id, uint8 *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  设置历史曲线采样数据值(双字节，uint16或int16)
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 采样点数据
*  \param  channel 通道数
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt16(uint16 screen_id, uint16 control_id, uint16 *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  设置历史曲线采样数据值(四字节，uint32或int32)
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 采样点数据
*  \param  channel 通道数
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueInt32(uint16 screen_id, uint16 control_id, uint32 *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  设置历史曲线采样数据值(单精度浮点数)
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 采样点数据
*  \param  channel 通道数
*  \param  uart_id UART ID
*/
void HistoryGraph_SetValueFloat(uint16 screen_id, uint16 control_id, float *value, uint8 channel, uint8 uart_id);

/*!
*  \brief  允许或禁止历史曲线采样
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  enable 0-禁止，1-允许
*  \param  uart_id UART ID
*/
void HistoryGraph_EnableSampling(uint16 screen_id, uint16 control_id, uint8 enable, uint8 uart_id);

/*!
*  \brief  显示或隐藏历史曲线通道
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  channel 通道编号
*  \param  show 0-隐藏，1-显示
*  \param  uart_id UART ID
*/
void HistoryGraph_ShowChannel(uint16 screen_id, uint16 control_id, uint8 channel, uint8 show, uint8 uart_id);

/*!
*  \brief  设置历史曲线时间长度(即采样点数)
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  sample_count 一屏显示的采样点数
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeLength(uint16 screen_id, uint16 control_id, uint16 sample_count, uint8 uart_id);

/*!
*  \brief  历史曲线缩放到全屏
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeFullScreen(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  设置历史曲线缩放比例系数
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  zoom 缩放百分比(zoom>100%时水平方向缩小，反正放大)
*  \param  max_zoom 缩放限制，一屏最多显示采样点数
*  \param  min_zoom 缩放限制，一屏最少显示采样点数
*  \param  uart_id UART ID
*/
void HistoryGraph_SetTimeZoom(uint16 screen_id, uint16 control_id, uint16 zoom, uint16 max_zoom, uint16 min_zoom, uint8 uart_id);
#endif

#if SD_FILE_EN
/*!
*  \brief  检测SD卡是否插入
*  \param  uart_id UART ID
*/
void SD_IsInsert(uint8 uart_id);

/*!
*  \brief  打开或创建文件
*  \param  filename 文件名称(仅ASCII编码)
*  \param  mode 模式，可选组合模式如上FA_XXXX
*  \param  uart_id UART ID
*/
void SD_CreateFile(uint8 *filename, uint8 mode, uint8 uart_id);

/*!
*  \brief  以当前时间创建文件，例如:20161015083000.txt
*  \param  ext 文件后缀，例如 txt
*  \param  uart_id UART ID
*/
void SD_CreateFileByTime(uint8 *ext, uint8 uart_id);

/*!
*  \brief  在当前文件末尾写入数据
*  \param  buffer 数据
*  \param  dlc 数据长度
*  \param  uart_id UART ID
*/
void SD_WriteFile(uint8 *buffer, uint16 dlc, uint8 uart_id);

/*!
*  \brief  读取当前文件
*  \param  offset 文件位置偏移
*  \param  dlc 数据长度
*  \param  uart_id UART ID
*/
void SD_ReadFile(uint32 offset, uint16 dlc, uint8 uart_id);

/*!
*  \brief  获取当前文件长度
*  \param  uart_id UART ID
*/
void SD_GetFileSize(uint8 uart_id);

/*!
*  \brief  关闭当前文件
*  \param  uart_id UART ID
*/
void SD_CloseFile(uint8 uart_id);
#endif

/*!
*  \brief  记录控件-触发警告
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 告警值
*  \param  time 告警产生的时间，为0时使用屏幕内部时间
*  \param  uart_id UART ID
*/
void Record_SetEvent(uint16 screen_id, uint16 control_id, uint16 value, uint8 *time, uint8 uart_id);

/*!
*  \brief  记录控件-解除警告
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  value 告警值
*  \param  time 告警解除的时间，为0时使用屏幕内部时间
*  \param  uart_id UART ID
*/
void Record_ResetEvent(uint16 screen_id, uint16 control_id, uint16 value, uint8 *time, uint8 uart_id);

/*!
*  \brief  记录控件- 添加常规记录
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  record 一条记录(字符串)，子项通过分号隔开，例如：第一项;第二项;第三项;
*  \param  uart_id UART ID
*/
void Record_Add(uint16 screen_id, uint16 control_id, uint8 *record, uint8 uart_id);

/*!
*  \brief  记录控件-清除记录数据
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void Record_Clear(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  记录控件-设置记录显示偏移
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  offset 显示偏移，滚动条位置
*  \param  uart_id UART ID
*/
void Record_SetOffset(uint16 screen_id, uint16 control_id, uint16 offset, uint8 uart_id);

/*!
*  \brief  记录控件-获取当前记录数目
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  uart_id UART ID
*/
void Record_GetCount(uint16 screen_id, uint16 control_id, uint8 uart_id);

/*!
*  \brief  读取屏幕RTC时间
*  \param  uart_id UART ID
*/
void ReadRTC(uint8 uart_id);

/*!
*  \brief  播放音乐
*  \param  buffer 十六进制的音乐路径及名字
*  \param  uart_id UART ID
*/
void PlayMusic(uint8 *buffer, uint8 uart_id);

/*!
*  \brief  记录控件-读取指定位置的记录
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  index 索引
*  \param  uart_id UART ID
*/
void Record_Read(uint16 screen_id, uint16 control_id, uint16 index, uint8 uart_id);

/*!
*  \brief  记录控件-修改指定位置的记录
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  index 索引
*  \param  record 一条记录(字符串)，子项通过分号隔开，例如：第一项;第二项;第三项;
*  \param  uart_id UART ID
*/
void Record_Modify(uint16 screen_id, uint16 control_id, uint16 index, uint8 *record, uint8 uart_id);

/*!
*  \brief  记录控件-删除指定位置的记录
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  index 索引
*  \param  uart_id UART ID
*/
void Record_Delete(uint16 screen_id, uint16 control_id, uint16 index, uint8 uart_id);

/*!
*  \brief  记录控件-在指定位置插入记录
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  index 索引
*  \param  record 一条记录(字符串)，子项通过分号隔开，例如：第一项;第二项;第三项;
*  \param  uart_id UART ID
*/
void Record_Insert(uint16 screen_id, uint16 control_id, uint16 index, uint8 *record, uint8 uart_id);

/*!
*  \brief  记录控件-选中指定记录索引
*  \param  screen_id 画面ID
*  \param  control_id 控件ID
*  \param  index 索引
*  \param  uart_id UART ID
*/
void Record_Select(uint16 screen_id, uint16 control_id, uint16 index, uint8 uart_id);




#endif
