使用[Arduino](https://www.arduino.cc/en/Main/Software)编译上传，[自行配置ESP8266](https://www.jianshu.com/p/cb0274d612b5)的环境，并添加依赖库[MFRC522](https://github.com/miguelbalboa/rfid)

RC522读取卡片卡号，若该卡号与录入卡号所匹配，则驱动舵机完成开门。

建议使用***手机NFC、手环NFC、校园卡***作为开门钥匙。

- **Reset按钮**

  按下后清除目前录入的所有卡号，即清除固件LFS下的文件。

- **录入模式按钮**

  按一下后LED灯会常亮，表示进入录入模式，再按一下则LED灯熄灭，表示退出录入模式。在录入模式下，读取的卡片的卡号会写入固件LFS下的文件，即录入卡号，且断电后所录入的卡号不会消失。

> 针脚定义修改

```C
#define RST_PIN 5 // 配置针脚
#define SS_PIN 4
#define SERVO_PIN 15
#define LED_PIN 16 // RST按键上方LED
//LED_BUILTIN 4 天线下方LED
#define BT1_PIN 2 // 录入模式
#define BT2_PIN 0 // 重置

String uid[15][1]; //最多保存15张卡片，有需要可以修改
```

> 接线图解

![pic](https://cdn.jsdelivr.net/gh/Henvy-Mango/nfc_door@latest/layout.png)

> 按钮接线图解

![pic](https://cdn.jsdelivr.net/gh/Henvy-Mango/nfc_door@latest/layout-button.png)

