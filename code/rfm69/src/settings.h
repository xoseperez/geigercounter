// -----------------------------------------------------------------------------
// Settings
// -----------------------------------------------------------------------------

// This node ID, should be unique in the same network
#define NODEID              13

// Gateway ID that will receive the messages from this node
#define GATEWAYID           1

// Network ID, only nodes from the same network see each other
#define NETWORKID           164

// Frequency of the radio, should match your radio module and
// comply with your country legislation
#define FREQUENCY           RF69_868MHZ

// Encription key, shared between the node and the gateway
#define ENCRYPTKEY          "fibonacci0123456"

// If you are using a RFM69HW or RFM69HCW module set this to 1 to
// transmitt to extra power (and extra battery drainage)
#define IS_RFM69HW          0

// Set the target RSSI for Auto Transmission Control feature.
// The module will modify it's transmitting power to match this
// value (relies on ACK packets to check the RSSI the gateway has seen).
#define ATC_RSSI            -75

// Comment to stop sending debug messages to serial console
#define DEBUG

// Define serial baudrate
#define SERIAL_BAUD         115200

// Various PIN definitions
#define LED_PIN             9
#define COUNTER_PIN         3

// Update counts every 6 seconds
#define UPDATE_INTERVAL     6000

// Ring size is 10 positions,
// when it overflows a message is sent via radio,
// that means every 60 seconds
#define RING_SIZE           10

// After an interrupt, wait this amount of milliseconds for
// signal to stabilize (avoid bouncing).
#define DEBOUNCE_INTERVAL   5

// Flash LED for this amount of milliseconds after every message sent
#define NOTIFICATION_TIME   5

// Conversion factor from CPM to uSv/h based on data from Libellium for the SBM-20 tube
// http://www.cooking-hacks.com/index.php/documentation/tutorials/geiger-counter-arduino-radiation-sensor-board
#define CPM_TO_USVH         0.0057
