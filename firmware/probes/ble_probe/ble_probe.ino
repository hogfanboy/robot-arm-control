/**
 * ble_probe.ino - First test: an ESP32-C3 talks Bluetooth to the LeArm robot arm.
 *
 * What it does:
 *   1. Looks for the arm's Bluetooth name "Hiwonder" and connects to it.
 *   2. Lets you type commands in the Serial Monitor (115200 baud) to move servos
 *      and read the arm's battery voltage.
 *
 * The arm accepts only ONE Bluetooth connection: disconnect the phone app first.
 *
 * Servo map (verified): 1 claw, 2 wrist (spins), 3 wrist tilt, 4 elbow,
 *                       5 shoulder, 6 base rotate (to be checked).
 * Positions are in microseconds: 500 to 2500, 1500 = center (arm straight up).
 *
 * Commands (type one, press Enter):
 *   help             show this list
 *   b                read the arm's battery voltage
 *   m ID POS [MS]    move servo ID to POS over MS milliseconds (default: see "t")
 *                    example: m 6 1200 2000
 *   j ID STEP        jog servo ID by STEP from where we last sent it (example: j 1 -100)
 *   c                center all six servos slowly (arm stands straight up)
 *   t MS             set the default move time (300-5000 ms)
 *   p                show the last position we sent to each servo
 *   raw 55 55 02 0F  send raw bytes (hex) to the arm
 *
 * Board: ESP32-C3 SuperMini (Arduino board "MakerGO ESP32 C3 SuperMini", USB CDC On Boot enabled).
 * Uses only the BLE library that comes with the ESP32 Arduino core (3.3.x).
 */
#include <BLEDevice.h>

// ---- Arm Bluetooth details (verified with nRF Connect) ----
static const char *ARM_NAME = "Hiwonder";
static BLEUUID SERVICE_UUID((uint16_t)0xFFF0);
static BLEUUID CHAR_UUID((uint16_t)0xFFE1);

// ---- Hiwonder command numbers ----
const uint8_t CMD_SERVO_MOVE  = 3;
const uint8_t CMD_GET_BATTERY = 15;

// ---- Safety limits ----
const int POS_MIN = 500, POS_MAX = 2500;
const int TIME_MIN = 300, TIME_MAX = 5000;
const int NUM_SERVOS = 6;

// ---- State ----
BLEClient *client = nullptr;
BLERemoteCharacteristic *armChar = nullptr;
volatile bool connected = false;
unsigned long lastConnectTry = 0;
int defaultMoveMs = 1000;
int lastPos[NUM_SERVOS + 1] = {0};  // 0 = we don't know yet (the arm can't tell us)

// ---------------------------------------------------------------------------
// Sending commands
// ---------------------------------------------------------------------------
void printHex(const uint8_t *data, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (data[i] < 0x10) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(' ');
  }
}

bool sendBytes(uint8_t *data, size_t len) {
  if (!connected || armChar == nullptr) {
    Serial.println("Not connected to the arm yet.");
    return false;
  }
  Serial.print("  sent: ");
  printHex(data, len);
  Serial.println();
  armChar->writeValue(data, len, false);
  return true;
}

// Move several servos at once. ids[] and positions[] have 'count' entries.
bool moveServos(const uint8_t *ids, const int *positions, int count, int timeMs) {
  uint8_t buf[5 + 3 * NUM_SERVOS + 2];
  timeMs = constrain(timeMs, TIME_MIN, TIME_MAX);
  int n = 0;
  buf[n++] = 0x55;
  buf[n++] = 0x55;
  buf[n++] = count * 3 + 5;        // length = bytes after the two 0x55
  buf[n++] = CMD_SERVO_MOVE;
  buf[n++] = count;
  buf[n++] = timeMs & 0xFF;
  buf[n++] = timeMs >> 8;
  for (int i = 0; i < count; i++) {
    int p = constrain(positions[i], POS_MIN, POS_MAX);
    buf[n++] = ids[i];
    buf[n++] = p & 0xFF;
    buf[n++] = p >> 8;
  }
  if (!sendBytes(buf, n)) return false;
  for (int i = 0; i < count; i++) lastPos[ids[i]] = constrain(positions[i], POS_MIN, POS_MAX);
  return true;
}

bool moveOne(int id, int pos, int timeMs) {
  uint8_t ids[1] = {(uint8_t)id};
  int positions[1] = {pos};
  return moveServos(ids, positions, 1, timeMs);
}

void askBattery() {
  uint8_t buf[4] = {0x55, 0x55, 0x02, CMD_GET_BATTERY};
  sendBytes(buf, 4);
}

// ---------------------------------------------------------------------------
// Replies from the arm (Bluetooth notifications)
// ---------------------------------------------------------------------------
void onArmReply(BLERemoteCharacteristic *ch, uint8_t *data, size_t len, bool isNotify) {
  Serial.print("  arm says: ");
  printHex(data, len);
  if (len >= 6 && data[0] == 0x55 && data[1] == 0x55 && data[3] == CMD_GET_BATTERY) {
    int mv = data[4] | (data[5] << 8);
    Serial.printf(" -> battery %d mV (%.2f V)", mv, mv / 1000.0);
  }
  Serial.println();
}

// ---------------------------------------------------------------------------
// Finding and connecting to the arm
// ---------------------------------------------------------------------------
class ClientEvents : public BLEClientCallbacks {
  void onConnect(BLEClient *c) override {}
  void onDisconnect(BLEClient *c) override {
    connected = false;
    armChar = nullptr;
    Serial.println("Arm disconnected. Will look for it again.");
  }
};

bool findAndConnect() {
  Serial.printf("Looking for \"%s\" (3 s scan)...\n", ARM_NAME);
  BLEScan *scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults *results = scan->start(3, false);

  BLEAdvertisedDevice *arm = nullptr;
  for (int i = 0; i < results->getCount(); i++) {
    BLEAdvertisedDevice dev = results->getDevice(i);
    if (dev.haveName() && dev.getName() == ARM_NAME) {
      arm = new BLEAdvertisedDevice(dev);
      break;
    }
  }
  scan->clearResults();

  if (arm == nullptr) {
    Serial.println("  Not found. Is the arm switched on, and the phone app disconnected?");
    return false;
  }

  Serial.printf("  Found it (signal %d dBm). Connecting...\n", arm->getRSSI());
  if (client == nullptr) {
    client = BLEDevice::createClient();
    client->setClientCallbacks(new ClientEvents());
  }
  bool ok = client->connect(arm);
  delete arm;
  if (!ok) {
    Serial.println("  Connect failed.");
    return false;
  }

  BLERemoteService *service = client->getService(SERVICE_UUID);
  if (service == nullptr) {
    Serial.println("  Connected, but service FFF0 is missing. Disconnecting.");
    client->disconnect();
    return false;
  }
  armChar = service->getCharacteristic(CHAR_UUID);
  if (armChar == nullptr) {
    Serial.println("  Connected, but characteristic FFE1 is missing. Disconnecting.");
    client->disconnect();
    return false;
  }
  if (armChar->canNotify()) armChar->registerForNotify(onArmReply);

  connected = true;
  Serial.println("  CONNECTED to the arm. Type 'help' for commands.");
  askBattery();
  return true;
}

// ---------------------------------------------------------------------------
// Serial Monitor commands
// ---------------------------------------------------------------------------
void printHelp() {
  Serial.println();
  Serial.println("Commands:");
  Serial.println("  b                battery voltage");
  Serial.println("  m ID POS [MS]    move one servo, e.g. m 6 1200 2000  (POS 500-2500, 1500 = center)");
  Serial.println("  j ID STEP        jog from last position, e.g. j 1 -100");
  Serial.println("  c                center all six slowly (arm straight up)");
  Serial.println("  t MS             default move time (now " + String(defaultMoveMs) + " ms)");
  Serial.println("  p                last positions sent");
  Serial.println("  raw 55 55 02 0F  send raw hex bytes");
  Serial.println("Servos: 1 claw, 2 wrist spin, 3 wrist tilt, 4 elbow, 5 shoulder, 6 base");
  Serial.println();
}

bool validId(int id) {
  if (id >= 1 && id <= NUM_SERVOS) return true;
  Serial.println("Servo ID must be 1 to 6.");
  return false;
}

void handleCommand(String line) {
  line.trim();
  if (line.length() == 0) return;
  Serial.println("> " + line);

  char cmd[8] = {0};
  int a = 0, b = 0, c = 0;
  int got = sscanf(line.c_str(), "%7s %d %d %d", cmd, &a, &b, &c);
  String name = String(cmd);
  name.toLowerCase();

  if (name == "help" || name == "h" || name == "?") {
    printHelp();
  } else if (name == "b") {
    askBattery();
  } else if (name == "m") {
    if (got < 3) { Serial.println("Use: m ID POS [MS]"); return; }
    if (!validId(a)) return;
    moveOne(a, b, got >= 4 ? c : defaultMoveMs);
  } else if (name == "j") {
    if (got < 3) { Serial.println("Use: j ID STEP"); return; }
    if (!validId(a)) return;
    if (lastPos[a] == 0) { Serial.println("Don't know where that servo is yet: use 'm' or 'c' first."); return; }
    int t = constrain(abs(b) * 3, TIME_MIN, TIME_MAX);  // bigger step = slower move
    moveOne(a, lastPos[a] + b, t);
  } else if (name == "c") {
    uint8_t ids[NUM_SERVOS] = {1, 2, 3, 4, 5, 6};
    int positions[NUM_SERVOS] = {1500, 1500, 1500, 1500, 1500, 1500};
    moveServos(ids, positions, NUM_SERVOS, 2000);
  } else if (name == "t") {
    if (got < 2) { Serial.println("Use: t MS"); return; }
    defaultMoveMs = constrain(a, TIME_MIN, TIME_MAX);
    Serial.printf("Default move time is now %d ms\n", defaultMoveMs);
  } else if (name == "p") {
    for (int i = 1; i <= NUM_SERVOS; i++) {
      Serial.printf("  servo %d: ", i);
      if (lastPos[i] == 0) Serial.println("unknown"); else Serial.println(lastPos[i]);
    }
  } else if (name == "raw") {
    uint8_t buf[32];
    int n = 0;
    const char *s = line.c_str() + 3;
    while (*s && n < 32) {
      while (*s == ' ') s++;
      if (!*s) break;
      buf[n++] = (uint8_t)strtol(s, (char **)&s, 16);
    }
    if (n > 0) sendBytes(buf, n);
  } else {
    Serial.println("Unknown command. Type 'help'.");
  }
}

// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  unsigned long start = millis();
  while (!Serial && millis() - start < 3000) delay(10);  // wait for the Serial Monitor (USB)
  Serial.println();
  Serial.println("=== LeArm BLE probe ===");
  BLEDevice::init("ArmProbe");
  printHelp();
}

void loop() {
  if (!connected && millis() - lastConnectTry > 2000) {
    findAndConnect();
    lastConnectTry = millis();
  }

  static String line;
  while (Serial.available()) {
    char ch = Serial.read();
    if (ch == '\n' || ch == '\r') {
      handleCommand(line);
      line = "";
    } else {
      line += ch;
    }
  }
  delay(5);
}
