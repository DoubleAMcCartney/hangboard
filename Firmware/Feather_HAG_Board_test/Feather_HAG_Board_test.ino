#include <bluefruit.h>
#include <Stepper.h>
#include <HX711.h>

// definitions for weight sensor
#define calibration_factor -7050.0 //This value is obtained using the SparkFun_HX711_Calibration sketch
#define DOUT  4
#define CLK  5

BLEService        hags = BLEService(0x53e74e5ad19247a08f0635ec90c73a3a);        //hag service
BLECharacteristic hagd = BLECharacteristic(0x2d0319d8de1511e89f32f2801f1b9fd1); //desired char
BLECharacteristic hagc = BLECharacteristic(0x4e0f43c2de1511e89f32f2801f1b9fd2); //current char
BLECharacteristic hagm = BLECharacteristic(0x2d0317b2de1511e89f32f2801f1b9fd3); //move status char

BLEDis  bledis;
BLEUart bleuart;
BLEBas  blebas;

// TODO: map homeButton and toFarButton to limit switch pins
bool homeButton = false;
bool toFarButton = false;
 
int const in1Pin = A0;
int const in2Pin = A1;
int const in3Pin = A2;
int const in4Pin = A3;
int const motorSpeed = 100;
int const mmPerRot = 4;
int const stepsPerRot = 200;

uint8_t angle = 0;
uint8_t depth = 0;
uint16_t weight = 0;
uint8_t weightArray[2];
uint8_t desiredAngle = 0;
uint8_t desiredDepth = 0;

void connect_callback(uint16_t conn_handle);
void disconnect_callback(uint16_t conn_handle, uint8_t reason);
void write_callback(BLECharacteristic& chr, uint8_t* data, uint16_t len, uint16_t offset);
void updateWeight();

Stepper motor(stepsPerRot, in1Pin, in2Pin, in3Pin, in4Pin);
HX711 scale(DOUT, CLK);

// Runs once at power up
void setup()
{
  Serial.begin(115200);

  Serial.println("H.A.G. Board");
  Serial.println("------------\n");

  // Initialise the Bluefruit module
  Serial.println("Initialise the Bluefruit nRF52 module");
  Bluefruit.begin();

  // Set the advertised device name
  Serial.println("Setting Device Name to 'H.A.G. Board'");
  Bluefruit.setName("H.A.G. Board");

  // Set the callback handlers
  Bluefruit.setConnectCallback(connect_callback);
  Bluefruit.setDisconnectCallback(disconnect_callback);
  hagd.setWriteCallback(write_callback);

  // Setup the H.A.G. Board service using
  // BLEService and BLECharacteristic classes
  Serial.println("Configuring the H.A.G. Board Service");
  setupHAG();

  // Set up Advertising Packet
  Serial.println("Setting up the advertising payload(s)");
  startAdv();

  Serial.println("\nAdvertising");

  // Set up for scale
  scale.set_scale(calibration_factor); //This value is obtained by using the SparkFun_HX711_Calibration sketch
  scale.tare(); //Assuming there is no weight on the scale at start up, reset the scale to 0

  // Set up for stepper motor
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(in3Pin, OUTPUT);
  pinMode(in4Pin, OUTPUT);
  
  motor.setSpeed(motorSpeed);

//  homeStepper();
}

void startAdv(void)
{
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include H.A.G. board 128-bit uuid
  Bluefruit.Advertising.addService(hags);

  // There is no room for Name in Advertising packet
  // Use Scan response for Name
  Bluefruit.ScanResponse.addName();

  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void setupHAG() {
  
  // Configure and Start HAG Service
  hags.begin();

  // Start desired Service
  hagd.setProperties(CHR_PROPS_WRITE); // client (app) will write to this charactoristic
  hagd.setPermission(SECMODE_OPEN, SECMODE_OPEN); // open reading access, open writing access
  hagd.setFixedLen(2); // two byte length
  hagd.begin();

  // Start current Service
  hagc.setProperties(CHR_PROPS_NOTIFY); // client (app) will be notified when this charactoristic changes
  hagc.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS); // open reading access, no write access
  hagc.setFixedLen(4); // four byte length
  hagc.begin();
  weightArray[0]=weight & 0xff;
  weightArray[1]=(weight >> 8);
  uint8_t hagCurrentData [4] = {angle, depth, weightArray[0], weightArray[1]};
  hagd.notify(hagCurrentData, 4);
  
  // Start move status Service
  hagm.setProperties(CHR_PROPS_NOTIFY); // client (app) will be notified when this charactoristic changes
  hagm.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS); // open reading access, no write access
  hagm.setFixedLen(1); // one byte length
  hagm.begin();
  uint8_t hagMoveData [1] = {0x00000000};
  hagm.notify(hagMoveData, 1);
}

void homeStepper() {
  while (!homeButton) {
    motor.step(-1);
  }
  depth=0;
}

void updateWeight() {
  //Serial.print("Updating wieght");
  //weight = (int)scale.get_units();
  //Serial.print("Wieght: "); Serial.println(weight);
}

// main loop
void loop()
{
  if ( Bluefruit.connected() ) {
    updateWeight();
    weightArray[0]=weight & 0xff;
    weightArray[1]=(weight >> 8);
    uint8_t hagCurrentData [4] = {angle, depth, weightArray[0], weightArray[1]};
    if ( hagc.notify(hagCurrentData, 4) ){
      //Serial.print("Wieght: "); Serial.println(weight);
      //Serial.print("Deptht: "); Serial.println(depth);
      //Serial.print("Angle: "); Serial.println(angle);
    }else{
      Serial.println("ERROR: Notify not set in the CCCD or not connected!");
    }
  }

  //update depth
  if (depth != desiredDepth) {
    // calculate steps to be moved
    long steps = (depth - desiredDepth)*stepsPerRot/mmPerRot;

    while (steps != 0) {
      Serial.println("Motor Moving:");
      Serial.println(steps);
      if (steps > 0) {
        motor.step(1);
        steps--; // move stepper
      }
      else {
        motor.step(-1);
        steps++; // move stepper
      }
//      // Check limit switches
//      if (homeButton) {
//        steps = 0;
//      }
//      else if (toFarButton){
//        steps = 0;
//      }
    }
    delay(10);

    // Check limit switches
    // Update current depth
    if (homeButton) {
      Serial.println("Home Button Hit!!!");
      depth = 0;
    }
    else if (toFarButton){
      Serial.println("Limit Button Hit!!!");
      depth = 100;  // TODO: change to max depth when we know what that is
    }
    else {
      depth = desiredDepth;
    }
    
  }
  delay(1000);

}

void connect_callback(uint16_t conn_handle)
{
  char central_name[32] = { 0 };
  Bluefruit.Gap.getPeerName(conn_handle, central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.println();
  Serial.println("Disconnected");
  Serial.println("Bluefruit will start advertising again");
}

void write_callback(BLECharacteristic& chr, uint8_t* data, uint16_t len, uint16_t offset)
{
  desiredAngle = data[0];
  desiredDepth = data[1];
  if ((desiredDepth < 0) || (desiredDepth > 100)) {
    Serial.print("Error. Desired depth is out of bounds!!!");
    desiredDepth = 0;
  }
  Serial.print("Desired depth updated to: "); Serial.println(desiredDepth);
  Serial.print("Desired angle updated to: "); Serial.println(desiredAngle);
}


/**
 * RTOS Idle callback is automatically invoked by FreeRTOS
 * when there are no active threads. E.g when loop() calls delay() and
 * there is no bluetooth or hw event. This is the ideal place to handle
 * background data.
 * 
 * NOTE: FreeRTOS is configured as tickless idle mode. After this callback
 * is executed, if there is time, freeRTOS kernel will go into low power mode.
 * Therefore waitForEvent() should not be called in this callback.
 * http://www.freertos.org/low-power-tickless-rtos.html
 * 
 * WARNING: This function MUST NOT call any blocking FreeRTOS API 
 * such as delay(), xSemaphoreTake() etc ... for more information
 * http://www.freertos.org/a00016.html
 */
void rtos_idle_callback(void)
{
  // Don't call any other FreeRTOS blocking API()
  // Perform background task(s) here
}
