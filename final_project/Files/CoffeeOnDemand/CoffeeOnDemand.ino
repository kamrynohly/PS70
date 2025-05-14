#include <WiFi.h>             
#include <Firebase_ESP_Client.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>

// Hardware connections
const int servoPin = D3;

// WiFi credentials
const char* ssid = "Kamryn iPhone";  
const char* password = "DELETEDFORPRIVACY";     

// Firebase credentials
#define FIREBASE_HOST "https://coffee-clip-c83aa-default-rtdb.firebaseio.com"  // Firebase database URL
#define FIREBASE_AUTH "DELETEDFORPRIVACY"                // Firebase Auth Token

// Firebase objects
FirebaseData db;
FirebaseAuth auth;
FirebaseConfig config;

struct Request {
  String id;
  String type;
  String scheduled_time;
  String status;
  String timestamp;
};

class CoffeeDatabaseManager {

  JsonDocument doc;

  public:
    CoffeeDatabaseManager() {
    }

    /*
      Setup the necessary Firebase components.
    */
    void setup() {
      // Initialize configuration properly
      config.api_key = FIREBASE_AUTH;
      config.database_url = FIREBASE_HOST;
      
      // Initialize Firebase
      Firebase.begin(&config, &auth);
      Firebase.reconnectWiFi(true);
    }

    /*
      Currently, authentication is not necessary.
      Thus, we can set default empty strings for these values.
    */
    void authenticate(const char* username = "", const char* password = "") {
      if (Firebase.signUp(&config, &auth, username, password)){
        Serial.println("Authentication succeeded.");
      }
      else{
        Serial.println("An error occurred while trying to authenticate...");
      }
    }

    /*
      Fetch any data from the Firebase database.
    */
    Request monitorDB() {
      // Read data from the "request_queue" location
      if (Firebase.RTDB.get(&db, "/request_queue")) {
        if (db.dataType() == "json") {
          Serial.println("Data received from Firebase:");
          String data = db.payload();
          Serial.println(data);

          // Format JSON into `Request` struct instances
          DeserializationError error = deserializeJson(doc, data);

          if (error) {
            Serial.print("Failed to parse JSON: ");
            Serial.println(error.f_str());
            Request emptyRequest = {"None", "None", "None", "None", "None"};
            return emptyRequest;
          }

          // Iterate over the keys in the JSON object
          for (JsonPair p : doc.as<JsonObject>()) {
            const char* uniqueKey = p.key().c_str();  // Get the unique key
            JsonObject request = p.value().as<JsonObject>();  // Get the value (request) associated with the key

            if (!request.isNull()) {
              const char* schedule_time = request["schedule_time"];
              const char* status = request["status"];
              const char* timestamp = request["timestamp"];
              const char* type = request["type"];

              // As soon as we get a new request, handle it!
              Request new_request = {uniqueKey, type, schedule_time, timestamp, status};
              return new_request;
            }
          }
        } else {
          Serial.println("Error: Unexpected data type.");
        }
      } else {
        Serial.println("Error reading data from Firebase: " + db.errorReason());
        Request emptyRequest = {"None", "None", "None", "None", "None"};
        return emptyRequest;
      }
    }

    void remove_request(Request request) {
      String path = "/request_queue/" + request.id;
      if (Firebase.RTDB.deleteNode(&db, path)) {
        Serial.println("Key and value removed successfully");
      } else {
        Serial.println("Failed to remove key and value");
        Serial.println(db.errorReason());
      }
    }
};

class CoffeeServo {
  Servo servo;
  int pos;            // Current location of servo
  int startPos;       // Where the servo should start
  int endPos;         // Where the servo should end
  int increment;      // How much to move it at a given moment
  int updateInterval; // Speed of servo
  unsigned long lastUpdate;
  public:
    CoffeeServo(int interval, int start, int target) {
      updateInterval = interval;
      startPos = start;
      endPos = target;
      increment = -10;
    }
    void Attach(int pin) {
      servo.attach(pin);
      servo.write(80); // Reset to start position
    }

    void pressButton() {
      servo.write(80);
      delay(2000);
      servo.write(145);
      delay(500);
      servo.write(80);
      delay(2000);
    }

    void Return() {
      if ((millis() - lastUpdate) > updateInterval && pos != startPos) {
        lastUpdate = millis();
        pos -= increment;
        servo.write(pos);
      }
    }

    void reset() {
      servo.write(80);
    }

    bool isMoving() {
      return !(pos == endPos || pos == startPos);
    }
};

CoffeeServo myServo(50, 0, 0);

CoffeeDatabaseManager db_manager;

void setup() {
  // Start Serial Monitor
  Serial.begin(115200);
  myServo.Attach(servoPin);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  db_manager.setup();
  db_manager.authenticate();
}

void loop() {
  Request request = db_manager.monitorDB();

  if (request.scheduled_time.equals("None")) {
    myServo.reset();
    // Do nothing
  } else {
    // We want to brew a coffee!
    myServo.pressButton();
    db_manager.remove_request(request);
  }
  delay(2000);
}