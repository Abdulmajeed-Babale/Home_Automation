#include<ESP8266WiFi.h>
#include<LiquidCrystal_I2C.h>
#include <DHT.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);


//Constants
#define DHTPIN 13     // what pin we're connected to
#define DHTTYPE DHT11   // DHT 11  (AM2302)
// Initialize DHT sensor for normal 16mhz Arduino
DHT dht(DHTPIN, DHTTYPE);

//Variables
int chk;
float hum;  //Stores humidity value
float temp; //Stores temperature value

const int Sensor_Pin = A0;
unsigned int Sensitivity = 185;   // 185mV/A for 5A, 100 mV/A for 20A and 66mV/A for 30A Module
float Vpp = 0; // peak-peak voltage
float Vrms = 0; // rms voltage
float Irms = 0; // rms current
float Supply_Voltage = 200.0;           // reading from DMM
float Vcc = 3.33;         // ADC reference voltage // voltage at 5V pin
float power = 0;         // power in watt
float Wh = 0 ;            // Energy in kWh
unsigned long last_time = 0;
unsigned long current_time = 0;
unsigned long interval = 100;
unsigned int calibration = 100;  // V2 slider calibrates this
unsigned int pF = 70;           // Power Factor default 95
//float bill_amount = 0;   // 30 day cost as present energy usage incl approx PF
//unsigned int energyTariff = 8.0; // Energy cost per unit (kWh)

void setup() {
  Serial.begin(115200);
  dht.begin();
  lcd.begin();
  lcd.backlight();
  pinMode(Sensor_Pin, INPUT);
}

void loop() {
  getACS712();
  read_dht();
  lcd_disp();
}

void getACS712() {  // for AC
  Vpp = getVPP();
  Vrms = (Vpp / 2.0) * 0.707;
  Vrms = Vrms - (calibration / 10000.0);     // calibtrate to zero with slider
  Irms = (Vrms * 1000) / Sensitivity ;
  if (Irms >= -0.01 && Irms <= -0.04) { // remove low end chatter
    Irms = 0.0;
  }
  //power= (Supply_Voltage * Irms) * (pF / 100.0);
  power = avg_power();
  if (power < 0) {
    power = 0;
  }
  last_time = current_time;
  current_time = millis();
  Wh = Wh +  power * (( current_time - last_time) / 3600000.0) ; // calculating energy in Watt-Hour
//  bill_amount = Wh * (energyTariff / 1000);
  Serial.print("Irms:  ");
  Serial.print(String(Irms, 2));
  Serial.println(" A");
  Serial.print("Power: ");
  Serial.print(String(power, 2));
  Serial.println(" W");
}
float getVPP()
{
  float result;
  int readValue;
  int maxValue = 0;
  int minValue = 1024;
  uint32_t start_time = millis();
  while ((millis() - start_time) < 950) //read every 0.95 Sec
  {
    readValue = analogRead(Sensor_Pin);
    if (readValue > maxValue)
    {
      maxValue = readValue;
    }
    if (readValue < minValue)
    {
      minValue = readValue;
    }
  }
  result = ((maxValue - minValue) * Vcc) / 1024.0;
  return result;
}

float avg_power() {
  int acc = 0;
  for (int i = 0; i <= 1000; i++) {
    power = (Supply_Voltage * Irms) * (pF / 100.0);
    acc += power;
  }
  float average = acc / 1000.0;
  return average;

}

void lcd_disp() {
  lcd.clear();
  lcd.print("P:");
  lcd.print(power);
  lcd.print("W ");
  lcd.print("T:");
  lcd.print(int(temp));
  lcd.print("*C");
  lcd.setCursor(0, 1);
  lcd.print("E:");
  lcd.print((Wh / 1000));
  lcd.print("KWh ");
  lcd.print("H:");
  lcd.print(int(hum));
  lcd.print("gm");
}

void read_dht()
{
  //Read data and store it to variables hum and temp
  hum = dht.readHumidity();
  temp = dht.readTemperature();
  //Print temp and humidity values to serial monitor
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %, Temp: ");
  Serial.print(temp);
  Serial.println(" Celsius");
  //delay(2000); //Delay 2 sec.
}
