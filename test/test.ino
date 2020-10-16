#include <SPI.h>
#include <Servo.h>
#include <MFRC522.h>
#include <LittleFS.h>

#define RST_PIN 5 // 配置针脚
#define SS_PIN 4
#define SERVO_PIN 15
#define LED_PIN 16 // RST按键上方LED
//LED_BUILTIN 4 天线下方LED
#define BT1_PIN 2 // 录入模式
#define BT2_PIN 0 // 重置

MFRC522 rfid(SS_PIN, RST_PIN); //实例化类

String file_name = "/tmp/uid.txt";

int times = 0;
String uid[15][1];

Servo myservo;

bool lastState = HIGH;
byte click = 0;
bool toggle = HIGH;

void setup()
{
  Serial.begin(9600);
  SPI.begin();     // 初始化SPI总线
  rfid.PCD_Init(); // 初始化 MFRC522

  initLoad();

  // 初始化舵机
  myservo.attach(SERVO_PIN);
  myservo.write(0);

  // 初始化LED和按钮
  pinMode(BT1_PIN, INPUT_PULLUP);
  pinMode(BT2_PIN, INPUT_PULLUP);

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
  bool flag = judgeBt();

  reset();

  // 找卡
  if (!rfid.PICC_IsNewCardPresent())
    return;

  // 验证NUID是否可读
  if (!rfid.PICC_ReadCardSerial())
    return;

  // MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  Serial.print("十六进制UID：");
  String id = dumpHexId(rfid.uid.uidByte, rfid.uid.size);
  Serial.println(id);

  // 录入模式
  if (judgeId(id, false) && flag)
  {
    // 写入数据至LittleFS文件系统
    writeFile(id);

    // 初始化uid数组
    times = 0;
    initLoad();
  }

  // 卡片验证通过
  if (!judgeId(id, true))
  {
    Serial.println("欢迎回来");
    servoWork();
  }
  else
  {
    Serial.println("验证失败");
  }

  // 使放置在读卡区的IC卡进入休眠状态，不再重复读卡
  rfid.PICC_HaltA();

  // 停止读卡模块编码
  rfid.PCD_StopCrypto1();

  Serial.print("times:");
  Serial.println(times);
}

String dumpHexId(byte *buffer, byte bufferSize)
{
  String tmp = "";
  for (byte i = 0; i < bufferSize; i++)
  {
    tmp += buffer[i] < 0x10 ? "0" : "";
    tmp += String(buffer[i], DEC);
  }
  return tmp;
}

void initLoad()
{
  if (LittleFS.begin()) // 启动闪存文件系统
    Serial.println("LittleFS Started.");
  else
    Serial.println("LittleFS Failed to Start.");

  //确认闪存中是否有file_name文件
  if (LittleFS.exists(file_name))
  {
    Serial.print(file_name + " FOUND.\n");

    //建立File对象用于从LittleFS中读取文件
    File dataFile = LittleFS.open(file_name, "r");

    //读取文件内容并且通过串口监视器输出文件信息
    String tmp = "";
    for (int i = 0; i < dataFile.size(); i++)
    {
      char id = (char)dataFile.read();
      if (id == '\n')
      {
        Serial.print("file:");
        Serial.print(tmp);

        uid[times][0] = tmp;
        times++;
        Serial.println();
        tmp = "";
      }
      else
      {
        tmp += id;
      }
    }
    //完成文件读取后关闭文件
    dataFile.close();
  }
  else
  {
    Serial.print(file_name + " NOT FOUND.\n");
    File dataFile = LittleFS.open(file_name, "a"); // 建立File对象用于向LittleFS中的file对象（即/notes.txt）写入信息
    dataFile.print("");                            // 向dataFile添加字符串信息
    dataFile.close();                              // 完成文件操作后关闭文件
    Serial.println("Finished init uid data to LittleFS");
  }
}

void writeFile(String str)
{
  Serial.print("str:");
  Serial.println(str);
  File dataFile = LittleFS.open(file_name, "a"); // 建立File对象用于向LittleFS中的file对象（即/notes.txt）写入信息
  dataFile.print(str + "\n");                    // 向dataFile添加字符串信息
  dataFile.close();                              // 完成文件操作后关闭文件
  delay(500);
}

bool judgeId(String id, bool tag)
{
  for (int i = 0; i < times; i++)
  {
    if (tag)
    {
      Serial.println("id:" + id + "uid:" + uid[i][0]);
      Serial.println((id.equals(uid[i][0])));
    }
    if (id.equals(uid[i][0]))
      return false;
  }
  return true;
}

// 舵机运作
void servoWork()
{
  delay(500);
  myservo.write(172);
  delay(3500);
  myservo.write(0);
  delay(500);
}

bool judgeBt()
{
  bool b1 = digitalRead(BT1_PIN); //读取目前的开关状态

  if (b1 != lastState) //如果与以前的开关至不同
  {
    delay(20);                      //去震动
    bool b2 = digitalRead(BT1_PIN); //再读取一次开关状态

    if (b2 == b1) //确认两次开关状态是否一致
    {
      lastState = b1; //存储开关状态
      click++;
    }
  }
  if (click == 2) //如果开关状态改变2次
  {
    click = 0;        //归零
    toggle = !toggle; //取相反值
    digitalWrite(LED_PIN, toggle);
    if (toggle == LOW)
    {
      Serial.println("已进入录入模式！");
    }
    else
    {
      Serial.println("退出录入模式！");
    }
  }
  if (toggle == LOW)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void reset()
{
  bool temp = digitalRead(BT2_PIN);
  if (temp == LOW)
  {
    digitalWrite(LED_BUILTIN, LOW);

    Serial.println("LittleFS format start");
    LittleFS.format(); // 格式化LittleFS
    Serial.println("LittleFS format finish");

    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);

    // 初始化uid数组
    times = 0;
    initLoad();
  }
}